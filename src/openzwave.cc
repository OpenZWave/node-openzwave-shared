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
#include <iostream>
#include <list>
#include <queue>

#include <node.h>
#include <v8.h>

#include "Manager.h"
#include "Node.h"
#include "Notification.h"
#include "Options.h"
#include "Value.h"

#ifdef WIN32
class mutex
{
public:
	mutex()              { InitializeCriticalSection(&_criticalSection); }
	~mutex()             { DeleteCriticalSection(&_criticalSection); }
	inline void lock()   { EnterCriticalSection(&_criticalSection); }
	inline void unlock() { LeaveCriticalSection(&_criticalSection); }

	class scoped_lock
	{
	public:
		inline explicit scoped_lock(mutex & sp) : _sl(sp) { _sl.lock(); }
		inline ~scoped_lock()                             { _sl.unlock(); }
	private:
		scoped_lock(scoped_lock const &);
		scoped_lock & operator=(scoped_lock const &);
		mutex& _sl;
	};

private:
	CRITICAL_SECTION _criticalSection;
};
#endif

#ifdef linux
#include <unistd.h>
#include <pthread.h>

class mutex
{
public:
	mutex()             { pthread_mutex_init(&_mutex, NULL); }
	~mutex()            { pthread_mutex_destroy(&_mutex); }
	inline void lock()  { pthread_mutex_lock(&_mutex); }
	inline void unlock(){ pthread_mutex_unlock(&_mutex); }

	class scoped_lock
	{
	public:
		inline explicit scoped_lock(mutex & sp) : _sl(sp)  { _sl.lock(); }
		inline ~scoped_lock()                              { _sl.unlock(); }
	private:
		scoped_lock(scoped_lock const &);
		scoped_lock & operator=(scoped_lock const &);
		mutex&  _sl;
	};

private:
	pthread_mutex_t _mutex;
};
#endif


using namespace v8;
using namespace node;

namespace {

	struct OZW : ObjectWrap {
		static Handle<Value> New(const Arguments& args);
		static Handle<Value> Connect(const Arguments& args);
		static Handle<Value> Disconnect(const Arguments& args);
		static Handle<Value> SetValue(const Arguments& args);
		static Handle<Value> SetLocation(const Arguments& args);
		static Handle<Value> SetName(const Arguments& args);
		static Handle<Value> EnablePoll(const Arguments& args);
		static Handle<Value> DisablePoll(const Arguments& args);
		static Handle<Value> HardReset(const Arguments& args);
		static Handle<Value> SoftReset(const Arguments& args);
		static Handle<Value> SetNodeOn(const Arguments& args);
		static Handle<Value> SetNodeOff(const Arguments& args);
		static Handle<Value> SwitchAllOn(const Arguments& args);
		static Handle<Value> SwitchAllOff(const Arguments& args);
		static Handle<Value> CreateScene(const Arguments& args);
		static Handle<Value> RemoveScene(const Arguments& args);
		static Handle<Value> GetScenes(const Arguments& args);
		static Handle<Value> AddSceneValue(const Arguments& args);
		static Handle<Value> RemoveSceneValue(const Arguments& args);
		static Handle<Value> SceneGetValues(const Arguments& args);
		static Handle<Value> ActivateScene(const Arguments& args);
		static Handle<Value> HealNetworkNode(const Arguments& args);
		static Handle<Value> HealNetwork(const Arguments& args);
		static Handle<Value> GetNodeNeighbors(const Arguments& args);
		static Handle<Value> SetConfigParam(const Arguments& args);
	};

	Persistent<Object> context_obj;

	uv_async_t async;

	typedef struct {
		uint32_t type;
		uint32_t homeid;
		uint8_t nodeid;
		uint8_t groupidx;
		uint8_t event;
		uint8_t buttonid;
		uint8_t sceneid;
		uint8_t notification;
		std::list<OpenZWave::ValueID> values;
	} NotifInfo;

	typedef struct {
		uint32_t homeid;
		uint8_t nodeid;
		bool polled;
		std::list<OpenZWave::ValueID> values;
	} NodeInfo;

	typedef struct {
		uint32_t sceneid;
		std::string label;
		std::list<OpenZWave::ValueID> values;
	} SceneInfo;

	/*
	* Message passing queue between OpenZWave callback and v8 async handler.
	*/
	static mutex zqueue_mutex;
	static std::queue<NotifInfo *> zqueue;

	/*
	* Node state.
	*/
	static mutex znodes_mutex;
	static std::list<NodeInfo *> znodes;

	static mutex zscenes_mutex;

	static std::list<SceneInfo *> zscenes;

	static uint32_t homeid;

	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value);

#include "utils.cc"

	/*
	* OpenZWave callback, just push onto queue and trigger the handler
	* in v8 land.
	*/
	void cb(OpenZWave::Notification const *cb, void *ctx) {
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

		{
			mutex::scoped_lock sl(zqueue_mutex);
			zqueue.push(notif);
		}
		uv_async_send(&async);
	}

	/*
	* Async handler, triggered by the OpenZWave callback.
	*/
	void async_cb_handler(uv_async_t *handle, int status) {
		NodeInfo *node;
		NotifInfo *notif;
		Local < Value > args[16];


		mutex::scoped_lock sl(zqueue_mutex);

		while (!zqueue.empty()) {
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
				{
					mutex::scoped_lock sl(znodes_mutex);
					znodes.push_back(node);
				}
				args[0] = String::New("node added");
				args[1] = Integer::New(notif->nodeid);
				MakeCallback(context_obj, "emit", 2, args);
				break;
				/*
				* Ignore intermediate notifications about a node status, we
				* wait until the node is ready before retrieving information.
				*/
			case OpenZWave::Notification::Type_NodeProtocolInfo:
				break;
			case OpenZWave::Notification::Type_NodeNaming: {
				Local < Object > info = Object::New();
				info->Set(String::NewSymbol("manufacturer"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("manufacturerid"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("product"), String::New(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("producttype"), String::New(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("productid"), String::New(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("type"), String::New(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("name"), String::New(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("loc"), String::New(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
				args[0] = String::New("node naming");
				args[1] = Integer::New(notif->nodeid);
				args[2] = info;
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
														   // XXX: these should be supported correctly.
			case OpenZWave::Notification::Type_PollingEnabled:
			case OpenZWave::Notification::Type_PollingDisabled:
				break;
				/*
				* Node values.
				*/
			case OpenZWave::Notification::Type_ValueAdded: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				if ((node = get_node_info(notif->nodeid))) {
					mutex::scoped_lock sl(znodes_mutex);
					//pthread_mutex_lock(&znodes_mutex);
					node->values.push_back(value);
					//pthread_mutex_unlock(&znodes_mutex);
				}

				args[0] = String::New("value added");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueChanged: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				args[0] = String::New("value changed");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueRefreshed: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				args[0] = String::New("value refreshed");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueRemoved: {
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
				args[3] = Integer::New(value.GetInstance());
				args[4] = Integer::New(value.GetIndex());
				MakeCallback(context_obj, "emit", 5, args);
				break;
			}
			
			/*
			 *Now node can accept commands.
			 */
			case OpenZWave::Notification::Type_EssentialNodeQueriesComplete: {
				Local < Object > info = Object::New();
				info->Set(String::NewSymbol("manufacturer"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("manufacturerid"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("product"), String::New(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("producttype"), String::New(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("productid"), String::New(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("type"), String::New(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("name"), String::New(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("loc"), String::New(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
				args[0] = String::New("node available");
				args[1] = Integer::New(notif->nodeid);
				args[2] = info;
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
				/*
				* The node is now fully ready for operation.
				*/
			case OpenZWave::Notification::Type_NodeQueriesComplete: {
				Local < Object > info = Object::New();
				info->Set(String::NewSymbol("manufacturer"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("manufacturerid"), String::New(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("product"), String::New(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("producttype"), String::New(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("productid"), String::New(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("type"), String::New(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("name"), String::New(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(String::NewSymbol("loc"), String::New(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
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
			case OpenZWave::Notification::Type_NodeEvent: {
				args[0] = String::New("node event");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(notif->event);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			case OpenZWave::Notification::Type_SceneEvent:{
				args[0] = String::New("scene event");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(notif->sceneid);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
														  /*
														  * A general notification.
														  */
			case OpenZWave::Notification::Type_Notification:
				args[0] = String::New("notification");
				args[1] = Integer::New(notif->nodeid);
				args[2] = Integer::New(notif->notification);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			case OpenZWave::Notification::Type_Group:
				// Leave it for now
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
	}

	Handle<Value> OZW::New(const Arguments& args) {
		HandleScope scope;

		assert(args.IsConstructCall());
		OZW* self = new OZW();
		self->Wrap(args.This());

		Local < Object > opts = args[0]->ToObject();
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

#include "openzwave-config.cc"
#include "openzwave-connection.cc"
#include "openzwave-network.cc"
#include "openzwave-nodes.cc"
#include "openzwave-polling.cc"
#include "openzwave-scenes.cc"
#include "openzwave-values.cc"

	extern "C" void init(Handle<Object> target) {
		HandleScope scope;

		Local < FunctionTemplate > t = FunctionTemplate::New(OZW::New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		t->SetClassName(String::New("OZW"));

		NODE_SET_PROTOTYPE_METHOD(t, "connect", OZW::Connect);
		NODE_SET_PROTOTYPE_METHOD(t, "disconnect", OZW::Disconnect);
		NODE_SET_PROTOTYPE_METHOD(t, "setValue", OZW::SetValue);
		NODE_SET_PROTOTYPE_METHOD(t, "setLocation", OZW::SetLocation);
		NODE_SET_PROTOTYPE_METHOD(t, "setName", OZW::SetName);
		NODE_SET_PROTOTYPE_METHOD(t, "enablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "disablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "hardReset", OZW::HardReset);
		NODE_SET_PROTOTYPE_METHOD(t, "softReset", OZW::SoftReset);
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeOn", OZW::SetNodeOn);
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeOff", OZW::SetNodeOff);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOn", OZW::SwitchAllOn);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOff", OZW::SwitchAllOff);
		NODE_SET_PROTOTYPE_METHOD(t, "createScene", OZW::CreateScene);
		NODE_SET_PROTOTYPE_METHOD(t, "removeScene", OZW::RemoveScene);
		NODE_SET_PROTOTYPE_METHOD(t, "getScenes", OZW::GetScenes);
		NODE_SET_PROTOTYPE_METHOD(t, "addSceneValue", OZW::AddSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "removeSceneValue", OZW::RemoveSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "sceneGetValues", OZW::SceneGetValues);
		NODE_SET_PROTOTYPE_METHOD(t, "activateScene", OZW::ActivateScene);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetworkNode", OZW::HealNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetwork", OZW::HealNetwork);
		NODE_SET_PROTOTYPE_METHOD(t, "getNeighbors", OZW::GetNodeNeighbors);
		NODE_SET_PROTOTYPE_METHOD(t, "setConfigParam", OZW::SetConfigParam);

		target->Set(String::NewSymbol("Emitter"), t->GetFunction());
	}

}

NODE_MODULE(openzwave, init)
