/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

#include "openzwave.hpp"

using namespace v8;
using namespace node;

namespace OZW {

	uv_async_t async;
	Persistent<Object> context_obj;

	/*
	* Message passing queue between OpenZWave callback and v8 async handler.
	*/
	mutex 		zqueue_mutex;
	std::queue<NotifInfo *> zqueue;

	/*
	* Node state.
	*/
	mutex 		znodes_mutex;
	std::list<NodeInfo *> znodes;

	mutex zscenes_mutex;
	std::list<SceneInfo *> zscenes;

	uint32_t homeid;
	
	
	/*
	* OpenZWave callback, just push onto queue and trigger the handler
	* in v8 land.
	*/
	// ===================================================================
	void cb(OpenZWave::Notification const *cb, void *ctx)
	// ===================================================================
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

		{
			mutex::scoped_lock sl(zqueue_mutex);
			zqueue.push(notif);
		}
		uv_async_send(&async);
	}

	/*
	* Async handler, triggered by the OpenZWave callback.
	*/
	// ===================================================================
	void async_cb_handler(uv_async_t *handle, int status)
	// ===================================================================
	{
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
			// TODO: these should be supported correctly.
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

	// ===================================================================
	Handle<v8::Value> OZW::New(const Arguments& args)
	// ===================================================================
	{
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

	// ===================================================================
	Handle<v8::Value> OZW::Connect(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		uv_async_init(uv_default_loop(), &async, async_cb_handler);

		context_obj = Persistent < Object > ::New(args.This());

		OpenZWave::Manager::Create();
		OpenZWave::Manager::Get()->AddWatcher(cb, NULL);
		OpenZWave::Manager::Get()->AddDriver(path);

		Handle<v8::Value> argv[1] = { String::New("connected") };
		MakeCallback(context_obj, "emit", 1, argv);

		return Undefined();
	}

	// ===================================================================
	Handle<v8::Value> OZW::Disconnect(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		OpenZWave::Manager::Get()->RemoveDriver(path);
		OpenZWave::Manager::Get()->RemoveWatcher(cb, NULL);
		OpenZWave::Manager::Destroy();
		OpenZWave::Options::Destroy();

		return scope.Close(Undefined());
	}

	// ===================================================================
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
		//
		NODE_SET_PROTOTYPE_METHOD(t, "enablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "disablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "setPollInterval",  OZW::SetPollInterval);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "hardReset", OZW::HardReset);
		NODE_SET_PROTOTYPE_METHOD(t, "softReset", OZW::SoftReset);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOn", OZW::SwitchAllOn);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOff", OZW::SwitchAllOff);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "createScene", OZW::CreateScene);
		NODE_SET_PROTOTYPE_METHOD(t, "removeScene", OZW::RemoveScene);
		NODE_SET_PROTOTYPE_METHOD(t, "getScenes", OZW::GetScenes);
		NODE_SET_PROTOTYPE_METHOD(t, "addSceneValue", OZW::AddSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "removeSceneValue", OZW::RemoveSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "sceneGetValues", OZW::SceneGetValues);
		NODE_SET_PROTOTYPE_METHOD(t, "activateScene", OZW::ActivateScene);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "healNetworkNode", OZW::HealNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetwork", OZW::HealNetwork);
		NODE_SET_PROTOTYPE_METHOD(t, "getNeighbors", OZW::GetNodeNeighbors);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "setConfigParam", OZW::SetConfigParam);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "beginControllerCommand", OZW::BeginControllerCommand);
		NODE_SET_PROTOTYPE_METHOD(t, "cancelControllerCommand", OZW::CancelControllerCommand);
		
		target->Set(String::NewSymbol("Emitter"), t->GetFunction());
		
		/* for BeginControllerCommand
		 * http://openzwave.com/dev/classOpenZWave_1_1Manager.html#aa11faf40f19f0cda202d2353a60dbf7b
		 */ 
		ctrlCmdNames = new CommandMap();
		// (*ctrlCmdNames)["None"] 					= OpenZWave::Driver::ControllerCommand_None;
		(*ctrlCmdNames)["AddDevice"]				= OpenZWave::Driver::ControllerCommand_AddDevice;
		(*ctrlCmdNames)["CreateNewPrimary"] 		= OpenZWave::Driver::ControllerCommand_CreateNewPrimary;
		(*ctrlCmdNames)["ReceiveConfiguration"] 	= OpenZWave::Driver::ControllerCommand_ReceiveConfiguration;
		(*ctrlCmdNames)["RemoveDevice"]  			= OpenZWave::Driver::ControllerCommand_RemoveDevice;
		(*ctrlCmdNames)["RemoveFailedNode"]			= OpenZWave::Driver::ControllerCommand_RemoveFailedNode;
		(*ctrlCmdNames)["HasNodeFailed "]			= OpenZWave::Driver::ControllerCommand_HasNodeFailed;
		(*ctrlCmdNames)["ReplaceFailedNode"]		= OpenZWave::Driver::ControllerCommand_ReplaceFailedNode;
		(*ctrlCmdNames)["TransferPrimaryRole"]		= OpenZWave::Driver::ControllerCommand_TransferPrimaryRole;
		(*ctrlCmdNames)["RequestNetworkUpdate"]		= OpenZWave::Driver::ControllerCommand_RequestNetworkUpdate;
		(*ctrlCmdNames)["RequestNodeNeighborUpdate"]= OpenZWave::Driver::ControllerCommand_RequestNodeNeighborUpdate;
		(*ctrlCmdNames)["AssignReturnRoute"] 		= OpenZWave::Driver::ControllerCommand_AssignReturnRoute;
		(*ctrlCmdNames)["DeleteAllReturnRoutes"]	= OpenZWave::Driver::ControllerCommand_DeleteAllReturnRoutes;
		(*ctrlCmdNames)["SendNodeInformation"] 		= OpenZWave::Driver::ControllerCommand_SendNodeInformation;
		(*ctrlCmdNames)["ReplicationSend"] 			= OpenZWave::Driver::ControllerCommand_ReplicationSend;
		(*ctrlCmdNames)["CreateButton"]				= OpenZWave::Driver::ControllerCommand_CreateButton;
		(*ctrlCmdNames)["DeleteButton"]				= OpenZWave::Driver::ControllerCommand_DeleteButton;
	}
}

NODE_MODULE(openzwave, OZW::init)
