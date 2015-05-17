/*
 * Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <unistd.h>
#include <pthread.h>
#include <list>
#include <queue>

#include <node.h>
#include <v8.h>

#include "Manager.h"
#include "Node.h"
#include "Notification.h"
#include "Options.h"
#include "Value.h"

using namespace v8;
using namespace node;

namespace {

struct OZW: ObjectWrap {
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Connect(const Arguments& args);
	static Handle<Value> Disconnect(const Arguments& args);
	static Handle<Value> SetValue(const Arguments& args);
	static Handle<Value> SetLevel(const Arguments& args);
	static Handle<Value> SetLocation(const Arguments& args);
	static Handle<Value> SetName(const Arguments& args);
	static Handle<Value> SwitchOn(const Arguments& args);
	static Handle<Value> SwitchOff(const Arguments& args);
	static Handle<Value> EnablePoll(const Arguments& args);
	static Handle<Value> DisablePoll(const Arguments& args);
	static Handle<Value> HardReset(const Arguments& args);
	static Handle<Value> SoftReset(const Arguments& args);
};

Persistent<Object> context_obj;

uv_async_t async;

typedef struct {
	uint32_t			type;
	uint32_t			homeid;
	uint8_t				nodeid;
	uint8_t				groupidx;
	uint8_t				event;
	uint8_t				buttonid;
	uint8_t				sceneid;
	uint8_t				notification;
	std::list<OpenZWave::ValueID>	values;
} NotifInfo;

typedef struct {
	uint32_t			homeid;
	uint8_t				nodeid;
	bool				polled;
	std::list<OpenZWave::ValueID>	values;
} NodeInfo;

/*
 * Message passing queue between OpenZWave callback and v8 async handler.
 */
static pthread_mutex_t zqueue_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::queue<NotifInfo *> zqueue;

/*
 * Node state.
 */
static pthread_mutex_t znodes_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::list<NodeInfo *> znodes;

static uint32_t homeid;

/*
 * Return the node for this request.
 */
NodeInfo *get_node_info(uint8_t nodeid)
{
	std::list<NodeInfo *>::iterator it;
	NodeInfo *node;

	for (it = znodes.begin(); it != znodes.end(); ++it) {
		node = *it;
		if (node->nodeid == nodeid)
			return node;
	}

	return NULL;
}

/*
 * OpenZWave callback, just push onto queue and trigger the handler
 * in v8 land.
 */
void cb(OpenZWave::Notification const *cb, void *ctx)
{
	NotifInfo *notif = new NotifInfo();

	notif->type = cb->GetType();
	notif->homeid = cb->GetHomeId();
	notif->nodeid = cb->GetNodeId();
	notif->values.push_front(cb->GetValueID());

	/*
	 * Some values are only set on particular notifications, and
	 * assertions in openzwave prevent us from trying to fetch them
	 * unconditionally.
	 */
	switch (notif->type) {
	case OpenZWave::Notification::Type_Group:
		notif->groupidx = cb->GetGroupIdx();
		break;
	case OpenZWave::Notification::Type_NodeEvent:
		notif->event = cb->GetEvent();
		break;
	case OpenZWave::Notification::Type_CreateButton:
	case OpenZWave::Notification::Type_DeleteButton:
	case OpenZWave::Notification::Type_ButtonOn:
	case OpenZWave::Notification::Type_ButtonOff:
		notif->buttonid = cb->GetButtonId();
		break;
	case OpenZWave::Notification::Type_SceneEvent:
		notif->sceneid = cb->GetSceneId();
		break;
	case OpenZWave::Notification::Type_Notification:
		notif->notification = cb->GetNotification();
		break;
	}

	pthread_mutex_lock(&zqueue_mutex);
	zqueue.push(notif);
	pthread_mutex_unlock(&zqueue_mutex);

	uv_async_send(&async);
}

/*
 * Async handler, triggered by the OpenZWave callback.
 */
void async_cb_handler(uv_async_t *handle, int status)
{
	NodeInfo *node;
	NotifInfo *notif;
	Local<Value> args[16];

	pthread_mutex_lock(&zqueue_mutex);

	while (!zqueue.empty())
	{
		notif = zqueue.front();

		switch (notif->type) {
		case OpenZWave::Notification::Type_DriverReady:
			homeid = notif->homeid;
			args[0] = String::New("driver ready");
			args[1] = Integer::New(homeid);
			MakeCallback(context_obj, "emit", 2, args);
			break;
		case OpenZWave::Notification::Type_DriverFailed:
			args[0] = String::New("driver failed");
			MakeCallback(context_obj, "emit", 1, args);
			break;
		/*
		 * NodeNew is triggered when a node is discovered which is not
		 * found in the OpenZWave XML file.  As we do not use that file
		 * simply ignore those notifications for now.
		 *
		 * NodeAdded is when we actually have a new node to set up.
		 */
		case OpenZWave::Notification::Type_NodeNew:
			break;
		case OpenZWave::Notification::Type_NodeAdded:
			node = new NodeInfo();
			node->homeid = notif->homeid;
			node->nodeid = notif->nodeid;
			node->polled = false;
			pthread_mutex_lock(&znodes_mutex);
			znodes.push_back(node);
			pthread_mutex_unlock(&znodes_mutex);
			args[0] = String::New("node added");
			args[1] = Integer::New(notif->nodeid);
			MakeCallback(context_obj, "emit", 2, args);
			break;
		/*
		 * Ignore intermediate notifications about a node status, we
		 * wait until the node is ready before retrieving information.
		 */
		case OpenZWave::Notification::Type_NodeProtocolInfo:
		case OpenZWave::Notification::Type_NodeNaming:
		// XXX: these should be supported correctly.
		case OpenZWave::Notification::Type_PollingEnabled:
		case OpenZWave::Notification::Type_PollingDisabled:
			break;
		/*
		 * Node values.
		 */
		case OpenZWave::Notification::Type_ValueAdded:
		case OpenZWave::Notification::Type_ValueChanged:
		{
			OpenZWave::ValueID value = notif->values.front();
			Local<Object> valobj = Object::New();
			const char *evname = (notif->type == OpenZWave::Notification::Type_ValueAdded)
			    ? "value added" : "value changed";

			if (notif->type == OpenZWave::Notification::Type_ValueAdded) {
				if ((node = get_node_info(notif->nodeid))) {
					pthread_mutex_lock(&znodes_mutex);
					node->values.push_back(value);
					pthread_mutex_unlock(&znodes_mutex);
				}
				OpenZWave::Manager::Get()->SetChangeVerified(value, true);
			}

			/*
			 * Common value types.
			 */
			valobj->Set(String::NewSymbol("type"),
				    String::New(OpenZWave::Value::GetTypeNameFromEnum(value.GetType())));
			valobj->Set(String::NewSymbol("genre"),
				    String::New(OpenZWave::Value::GetGenreNameFromEnum(value.GetGenre())));
			valobj->Set(String::NewSymbol("instance"),
				    Integer::New(value.GetInstance()));
			valobj->Set(String::NewSymbol("index"),
				    Integer::New(value.GetIndex()));
			valobj->Set(String::NewSymbol("label"),
				    String::New(OpenZWave::Manager::Get()->GetValueLabel(value).c_str()));
			valobj->Set(String::NewSymbol("units"),
				    String::New(OpenZWave::Manager::Get()->GetValueUnits(value).c_str()));
			valobj->Set(String::NewSymbol("read_only"),
				    Boolean::New(OpenZWave::Manager::Get()->IsValueReadOnly(value))->ToBoolean());
			valobj->Set(String::NewSymbol("write_only"),
				    Boolean::New(OpenZWave::Manager::Get()->IsValueWriteOnly(value))->ToBoolean());
			// XXX: verify_changes=
			// XXX: poll_intensity=
			valobj->Set(String::NewSymbol("min"),
				    Integer::New(OpenZWave::Manager::Get()->GetValueMin(value)));
			valobj->Set(String::NewSymbol("max"),
				    Integer::New(OpenZWave::Manager::Get()->GetValueMax(value)));

			/*
			 * The value itself is type-specific.
			 */
			switch (value.GetType()) {
			case OpenZWave::ValueID::ValueType_Bool:
			{
				bool val;
				OpenZWave::Manager::Get()->GetValueAsBool(value, &val);
				valobj->Set(String::NewSymbol("value"), Boolean::New(val)->ToBoolean());
				break;
			}
			case OpenZWave::ValueID::ValueType_Byte:
			{
				uint8_t val;
				OpenZWave::Manager::Get()->GetValueAsByte(value, &val);
				valobj->Set(String::NewSymbol("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_Decimal:
			{
				float val;
				OpenZWave::Manager::Get()->GetValueAsFloat(value, &val);
				valobj->Set(String::NewSymbol("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_Int:
			{
				int32_t val;
				OpenZWave::Manager::Get()->GetValueAsInt(value, &val);
				valobj->Set(String::NewSymbol("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_List:
			{
				Local<Array> items;
			}
			case OpenZWave::ValueID::ValueType_Short:
			{
				int16_t val;
				OpenZWave::Manager::Get()->GetValueAsShort(value, &val);
				valobj->Set(String::NewSymbol("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_String:
			{
				std::string val;
				OpenZWave::Manager::Get()->GetValueAsString(value, &val);
				valobj->Set(String::NewSymbol("value"), String::New(val.c_str()));
				break;
			}
			/*
			 * Buttons do not have a value.
			 */
			case OpenZWave::ValueID::ValueType_Button:
			{
				break;
			}
			default:
				fprintf(stderr, "unsupported value type: 0x%x\n", value.GetType());
				break;
			}

			args[0] = String::New(evname);
			args[1] = Integer::New(notif->nodeid);
			args[2] = Integer::New(value.GetCommandClassId());
			args[3] = valobj;
			MakeCallback(context_obj, "emit", 4, args);

			break;
		}
		/*
		 * A value update was sent but nothing changed, likely due to
		 * the value just being polled.  Ignore, as we handle actual
		 * changes above.
		 */
		case OpenZWave::Notification::Type_ValueRefreshed:
			break;
		case OpenZWave::Notification::Type_ValueRemoved:
		{
			OpenZWave::ValueID value = notif->values.front();
			std::list<OpenZWave::ValueID>::iterator vit;
			if ((node = get_node_info(notif->nodeid))) {
				for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
					if ((*vit) == notif->values.front()) {
						node->values.erase(vit);
						break;
					}
				}
			}
			args[0] = String::New("value removed");
			args[1] = Integer::New(notif->nodeid);
			args[2] = Integer::New(value.GetCommandClassId());
			args[3] = Integer::New(value.GetIndex());
			args[4] = Integer::New(value.GetInstance());
			MakeCallback(context_obj, "emit", 5, args);
			break;
		}
		/*
		 * I believe this means that the node is now ready to accept
		 * commands, however for now we will wait until all queries are
		 * complete before notifying upstack, just in case.
		 */
		case OpenZWave::Notification::Type_EssentialNodeQueriesComplete:
			break;
		/*
		 * The node is now fully ready for operation.
		 */
		case OpenZWave::Notification::Type_NodeQueriesComplete:
		{
			Local<Object> info = Object::New();
			info->Set(String::NewSymbol("manufacturer"),
			    String::New(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("manufacturerid"),
			    String::New(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("product"),
			    String::New(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("producttype"),
			    String::New(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("productid"),
			    String::New(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("type"),
			    String::New(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("name"),
			    String::New(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
			info->Set(String::NewSymbol("loc"),
			    String::New(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
			args[0] = String::New("node ready");
			args[1] = Integer::New(notif->nodeid);
			args[2] = info;
			MakeCallback(context_obj, "emit", 3, args);
			break;
		}
		/*
		 * The network scan has been completed.  Currently we do not
		 * care about dead nodes - is there anything we can do anyway?
		 */
		case OpenZWave::Notification::Type_AwakeNodesQueried:
		case OpenZWave::Notification::Type_AllNodesQueried:
		case OpenZWave::Notification::Type_AllNodesQueriedSomeDead:
			args[0] = String::New("scan complete");
			MakeCallback(context_obj, "emit", 1, args);
			break;
		/*
		 * A general notification.
		 */
		case OpenZWave::Notification::Type_Notification:
			args[0] = String::New("notification");
			args[1] = Integer::New(notif->nodeid);
			args[2] = Integer::New(notif->notification);
			MakeCallback(context_obj, "emit", 3, args);
			break;
		/*
		 * Send unhandled events to stderr so we can monitor them if
		 * necessary.
		 */
		default:
			fprintf(stderr, "Unhandled notification: %d\n", notif->type);
			break;
		}

		zqueue.pop();
	}

	pthread_mutex_unlock(&zqueue_mutex);
}

Handle<Value> OZW::New(const Arguments& args)
{
	HandleScope scope;

	assert(args.IsConstructCall());
	OZW* self = new OZW();
	self->Wrap(args.This());

	Local<Object> opts = args[0]->ToObject();
	std::string confpath = (*String::Utf8Value(opts->Get(String::New("modpath")->ToString())));
	confpath += "/../deps/open-zwave/config";

	/*
	 * Options are global for all drivers and can only be set once.
	 */
	OpenZWave::Options::Create(confpath.c_str(), "", "");
	OpenZWave::Options::Get()->AddOptionBool("ConsoleOutput", opts->Get(String::New("consoleoutput"))->BooleanValue());
	OpenZWave::Options::Get()->AddOptionBool("Logging", opts->Get(String::New("logging"))->BooleanValue());
	OpenZWave::Options::Get()->AddOptionBool("SaveConfiguration", opts->Get(String::New("saveconfig"))->BooleanValue());
	OpenZWave::Options::Get()->AddOptionInt("DriverMaxAttempts", opts->Get(String::New("driverattempts"))->IntegerValue());
	OpenZWave::Options::Get()->AddOptionInt("PollInterval", opts->Get(String::New("pollinterval"))->IntegerValue());
	OpenZWave::Options::Get()->AddOptionBool("IntervalBetweenPolls", true);
	OpenZWave::Options::Get()->AddOptionBool("SuppressValueRefresh", opts->Get(String::New("suppressrefresh"))->BooleanValue());
	OpenZWave::Options::Get()->Lock();

	return scope.Close(args.This());
}

Handle<Value> OZW::Connect(const Arguments& args)
{
	HandleScope scope;

	std::string path = (*String::Utf8Value(args[0]->ToString()));

	uv_async_init(uv_default_loop(), &async, async_cb_handler);

	context_obj = Persistent<Object>::New(args.This());

	OpenZWave::Manager::Create();
	OpenZWave::Manager::Get()->AddWatcher(cb, NULL);
	OpenZWave::Manager::Get()->AddDriver(path);

	Handle<Value> argv[1] = { String::New("connected") };
	MakeCallback(context_obj, "emit", 1, argv);

	return Undefined();
}

Handle<Value> OZW::Disconnect(const Arguments& args)
{
	HandleScope scope;

	std::string path = (*String::Utf8Value(args[0]->ToString()));

	OpenZWave::Manager::Get()->RemoveDriver(path);
	OpenZWave::Manager::Get()->RemoveWatcher(cb, NULL);
	OpenZWave::Manager::Destroy();
	OpenZWave::Options::Destroy();

	return scope.Close(Undefined());
}

/*
 * Generic value set.
 */
Handle<Value> OZW::SetValue(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	uint8_t comclass = args[1]->ToNumber()->Value();
	uint8_t index = args[2]->ToNumber()->Value();
	uint8_t instance = args[3]->ToNumber()->Value();

	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if (((*vit).GetCommandClassId() == comclass) &&
			    ((*vit).GetIndex() == index) &&
			    ((*vit).GetInstance() == instance)) {

				switch ((*vit).GetType()) {
				case OpenZWave::ValueID::ValueType_Bool:
				{
					bool val = args[4]->ToBoolean()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Byte:
				{
					uint8_t val = args[4]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Decimal:
				{
					float val = args[4]->ToNumber()->NumberValue();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Int:
				{
					int32_t val = args[4]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Short:
				{
					int16_t val = args[4]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_String:
				{
					std::string val = (*String::Utf8Value(args[4]->ToString()));
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_List:
				case OpenZWave::ValueID::ValueType_Schedule:
				case OpenZWave::ValueID::ValueType_Button:
				case OpenZWave::ValueID::ValueType_Raw: {
					break;
				}
				}
			}
		}
	}

	return scope.Close(Undefined());
}

/*
 * Set a COMMAND_CLASS_SWITCH_MULTILEVEL device to a specific value.
 */
Handle<Value> OZW::SetLevel(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	uint8_t value = args[1]->ToNumber()->Value();

	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if ((*vit).GetCommandClassId() == 0x26 && (*vit).GetIndex() == 0 && (*vit).GetInstance() == 1) {
				OpenZWave::Manager::Get()->SetValue(*vit, value);
				break;
			}
		}
	}

	return scope.Close(Undefined());
}

/*
 * Write a new location string to the device, if supported.
 */
Handle<Value> OZW::SetLocation(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	std::string location = (*String::Utf8Value(args[1]->ToString()));

	OpenZWave::Manager::Get()->SetNodeLocation(homeid, nodeid, location);

	return scope.Close(Undefined());
}

/*
 * Write a new name string to the device, if supported.
 */
Handle<Value> OZW::SetName(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	std::string name = (*String::Utf8Value(args[1]->ToString()));

	OpenZWave::Manager::Get()->SetNodeName(homeid, nodeid, name);

	return scope.Close(Undefined());
}

/*
 * Switch a COMMAND_CLASS_SWITCH_BINARY on/off
 */
void set_switch(uint8_t nodeid, bool state)
{
	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if ((*vit).GetCommandClassId() == 0x25 && (*vit).GetIndex() == 0 && (*vit).GetInstance() == 1) {
				OpenZWave::Manager::Get()->SetValue(*vit, state);
				break;
			}
		}
	}
}
Handle<Value> OZW::SwitchOn(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	set_switch(nodeid, true);

	return scope.Close(Undefined());
}
Handle<Value> OZW::SwitchOff(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	set_switch(nodeid, false);

	return scope.Close(Undefined());
}

/*
 * Enable/Disable polling on a COMMAND_CLASS basis.
 */
Handle<Value> OZW::EnablePoll(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	uint8_t comclass = args[1]->ToNumber()->Value();
	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if ((*vit).GetCommandClassId() == comclass) {
				OpenZWave::Manager::Get()->EnablePoll((*vit), 1);
				break;
			}
		}
	}

	return scope.Close(Undefined());
}
Handle<Value> OZW::DisablePoll(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	uint8_t comclass = args[1]->ToNumber()->Value();
	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if ((*vit).GetCommandClassId() == comclass) {
				OpenZWave::Manager::Get()->DisablePoll((*vit));
				break;
			}
		}
	}

	return scope.Close(Undefined());
}

/*
 * Reset the ZWave controller chip.  A hard reset is destructive and wipes
 * out all known configuration, a soft reset just restarts the chip.
 */
Handle<Value> OZW::HardReset(const Arguments& args)
{
	HandleScope scope;

	OpenZWave::Manager::Get()->ResetController(homeid);

	return scope.Close(Undefined());
}
Handle<Value> OZW::SoftReset(const Arguments& args)
{
	HandleScope scope;

	OpenZWave::Manager::Get()->SoftReset(homeid);

	return scope.Close(Undefined());
}

extern "C" void init(Handle<Object> target)
{
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(OZW::New);
	t->InstanceTemplate()->SetInternalFieldCount(1);
	t->SetClassName(String::New("OZW"));

	NODE_SET_PROTOTYPE_METHOD(t, "connect", OZW::Connect);
	NODE_SET_PROTOTYPE_METHOD(t, "disconnect", OZW::Disconnect);
	NODE_SET_PROTOTYPE_METHOD(t, "setValue", OZW::SetValue);
	NODE_SET_PROTOTYPE_METHOD(t, "setLevel", OZW::SetLevel);
	NODE_SET_PROTOTYPE_METHOD(t, "setLocation", OZW::SetLocation);
	NODE_SET_PROTOTYPE_METHOD(t, "setName", OZW::SetName);
	NODE_SET_PROTOTYPE_METHOD(t, "switchOn", OZW::SwitchOn);
	NODE_SET_PROTOTYPE_METHOD(t, "switchOff", OZW::SwitchOff);
	NODE_SET_PROTOTYPE_METHOD(t, "enablePoll", OZW::EnablePoll);
	NODE_SET_PROTOTYPE_METHOD(t, "disablePoll", OZW::EnablePoll);
	NODE_SET_PROTOTYPE_METHOD(t, "hardReset", OZW::HardReset);
	NODE_SET_PROTOTYPE_METHOD(t, "softReset", OZW::SoftReset);

	target->Set(String::NewSymbol("Emitter"), t->GetFunction());
}

}

NODE_MODULE(openzwave, init)
