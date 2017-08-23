/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015-2017 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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
using namespace OpenZWave;

namespace OZW {

//
uv_async_t async;

//
Nan::Callback *emit_cb;
Nan::CopyablePersistentTraits<v8::Object>::CopyablePersistent ctx_obj;

// Message passing queue between OpenZWave callback and v8 async handler.
mutex zqueue_mutex;
std::queue<NotifInfo *> zqueue;

// Node state.
mutex znodes_mutex;
std::map<uint8_t, NodeInfo *> znodes;

mutex zscenes_mutex;
std::list<SceneInfo *> zscenes;
/*
* OpenZWave callback, registered in Driver::AddWatcher.
* Just push onto queue and trigger the handler in v8 land.
*/
// ===================================================================
void ozw_watcher_callback(OpenZWave::Notification const *cb, void *ctx)
// ===================================================================
{
  NotifInfo *notif = new NotifInfo();

  notif->type = cb->GetType();
  notif->homeid = cb->GetHomeId();
  notif->nodeid = cb->GetNodeId();
  // only valueId-related callbacks carry an actual OZW ValueID
  if (notif->type <= OpenZWave::Notification::Type_ValueRefreshed) {
    notif->values.push_front(cb->GetValueID());
  }
  notif->help = getNotifHelpMsg(cb);
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
#if OPENZWAVE_SECURITY == 1
  case OpenZWave::Notification::Type_ControllerCommand:
    notif->event = cb->GetEvent();
    notif->notification = cb->GetNotification();
    break;
#endif
  }

  {
    mutex::scoped_lock sl(zqueue_mutex);
    zqueue.push(notif);
  }
  uv_async_send(&async);
}

// ##### LEGACY MODE ###### //
#if OPENZWAVE_SECURITY != 1 //
                            // ######################## //
/*
* OpenZWave callback, registered in Manager::BeginControllerCommand.
* Just push onto queue and trigger the handler in v8 land.
*/
// ===================================================================
void ozw_ctrlcmd_callback(Driver::ControllerState _state,
                          Driver::ControllerError _err, void *context)
// ===================================================================
{

  NotifInfo *notif = new NotifInfo();
  notif->event = _err;
  notif->notification = _state;
  notif->homeid = 0; // use as guard value for legacy mode
  notif->help = std::string("Controller State: ")
                    .append(getControllerStateAsStr(_state))
                    .append(", Error: ")
                    .append(getControllerErrorAsStr(_err));
  {
    mutex::scoped_lock sl(zqueue_mutex);
    zqueue.push(notif);
  }
  uv_async_send(&async);
}

// ===================================================================
void handleControllerCommand(NotifInfo *notif)
// ===================================================================
{
  Nan::HandleScope scope;

  Local<v8::Value> info[16];
  info[0] = Nan::New<String>("controller command").ToLocalChecked();
  info[1] = Nan::New<Integer>(notif->nodeid);
  info[2] = Nan::New<Integer>(notif->event);        // Driver::ControllerCommand
  info[3] = Nan::New<Integer>(notif->notification); // Driver::ControllerCommand
  info[4] = Nan::New<String>(notif->help.c_str()).ToLocalChecked();
  emit_cb->Call(Nan::New(ctx_obj), 5, info);
}
// ##### END OF LEGACY MODE ###### //
#endif

/*
 * handle normal OpenZWave notifications
 */
// ===================================================================
void handleNotification(NotifInfo *notif)
// ===================================================================
{
  Nan::HandleScope scope;
  Local<v8::Value> emitinfo[16];
  Local<Object> cbinfo = Nan::New<Object>();
	//
  NodeInfo *node;
  //
  switch (notif->type) {
	  //                            #################
	  case OpenZWave::Notification::Type_DriverReady: {
		    //                            #################
		    // the driver is ready, create an OZWDriver instance
			cout << "driver ready\n";
			Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(OZWDriver::New);
			t->SetClassName(Nan::New("OZWDriver").ToLocalChecked());
			t->InstanceTemplate()->SetInternalFieldCount(1);
			// openzwave-config.cc
			Nan::SetPrototypeMethod(t, "setConfigParam", OZWDriver::SetConfigParam);
			Nan::SetPrototypeMethod(t, "requestConfigParam", OZWDriver::RequestConfigParam);
			Nan::SetPrototypeMethod(t, "requestAllConfigParams", OZWDriver::RequestAllConfigParams);
			// openzwave-controller.cc
			Nan::SetPrototypeMethod(t, "hardReset", OZWDriver::HardReset);
			Nan::SetPrototypeMethod(t, "softReset", OZWDriver::SoftReset);
			Nan::SetPrototypeMethod(t, "getControllerNodeId", OZWDriver::GetControllerNodeId); // ** new
			Nan::SetPrototypeMethod(t, "getSUCNodeId", OZWDriver::GetSUCNodeId); // ** new
			Nan::SetPrototypeMethod(t, "isPrimaryController", OZWDriver::IsPrimaryController); // ** new
			Nan::SetPrototypeMethod(t, "isStaticUpdateController", OZWDriver::IsStaticUpdateController); // ** new
			Nan::SetPrototypeMethod(t, "isBridgeController", OZWDriver::IsBridgeController); // ** new
			Nan::SetPrototypeMethod(t, "getLibraryVersion", OZWDriver::GetLibraryVersion); // ** new
			Nan::SetPrototypeMethod(t, "getLibraryTypeName", OZWDriver::GetLibraryTypeName); // ** new
			Nan::SetPrototypeMethod(t, "getSendQueueCount", OZWDriver::GetSendQueueCount);	// ** new

			// openzwave-groups.cc
			Nan::SetPrototypeMethod(t, "getNumGroups", OZWDriver::GetNumGroups);
			Nan::SetPrototypeMethod(t, "getAssociations", OZWDriver::GetAssociations);
			Nan::SetPrototypeMethod(t, "getMaxAssociations", OZWDriver::GetMaxAssociations);
			Nan::SetPrototypeMethod(t, "getGroupLabel", OZWDriver::GetGroupLabel);
			Nan::SetPrototypeMethod(t, "addAssociation", OZWDriver::AddAssociation);
			Nan::SetPrototypeMethod(t, "removeAssociation", OZWDriver::RemoveAssociation);
			// openzwave-management.cc
	#if OPENZWAVE_SECURITY == 1
			Nan::SetPrototypeMethod(t, "addNode", OZWDriver::AddNode);
		  Nan::SetPrototypeMethod(t, "removeNode", OZWDriver::RemoveNode);
		  Nan::SetPrototypeMethod(t, "removeFailedNode", OZWDriver::RemoveFailedNode);
		  Nan::SetPrototypeMethod(t, "hasNodeFailed", OZWDriver::HasNodeFailed);
		  Nan::SetPrototypeMethod(t, "requestNodeNeighborUpdate", OZWDriver::RequestNodeNeighborUpdate);
		  Nan::SetPrototypeMethod(t, "assignReturnRoute", OZWDriver::AssignReturnRoute);
		  Nan::SetPrototypeMethod(t, "deleteAllReturnRoutes", OZWDriver::DeleteAllReturnRoutes);
		  Nan::SetPrototypeMethod(t, "sendNodeInformation", OZWDriver::SendNodeInformation);
		  Nan::SetPrototypeMethod(t, "createNewPrimary", OZWDriver::CreateNewPrimary);
		  Nan::SetPrototypeMethod(t, "receiveConfiguration", OZWDriver::ReceiveConfiguration);
		  Nan::SetPrototypeMethod(t, "replaceFailedNode", OZWDriver::ReplaceFailedNode);
		  Nan::SetPrototypeMethod(t, "transferPrimaryRole", OZWDriver::TransferPrimaryRole);
		  Nan::SetPrototypeMethod(t, "requestNetworkUpdate", OZWDriver::RequestNetworkUpdate);
		  Nan::SetPrototypeMethod(t, "replicationSend", OZWDriver::ReplicationSend);
		  Nan::SetPrototypeMethod(t, "createButton", OZWDriver::CreateButton);
		  Nan::SetPrototypeMethod(t, "deleteButton", OZWDriver::DeleteButton);
	#else
			Nan::SetPrototypeMethod(t, "beginControllerCommand", OZWDriver::BeginControllerCommand);
	#endif
			Nan::SetPrototypeMethod(t, "cancelControllerCommand", OZWDriver::CancelControllerCommand);
			Nan::SetPrototypeMethod(t, "writeConfig", OZWDriver::WriteConfig);
			Nan::SetPrototypeMethod(t, "getDriverStatistics", OZWDriver::GetDriverStatistics);
			Nan::SetPrototypeMethod(t, "getNodeStatistics", OZWDriver::GetNodeStatistics);
			// openzwave-network.cc
			Nan::SetPrototypeMethod(t, "testNetworkNode", OZWDriver::TestNetworkNode);
			Nan::SetPrototypeMethod(t, "testNetwork", OZWDriver::TestNetwork);
			Nan::SetPrototypeMethod(t, "healNetworkNode", OZWDriver::HealNetworkNode);
			Nan::SetPrototypeMethod(t, "healNetwork", OZWDriver::HealNetwork);
			// openzwave-nodes.cc
			Nan::SetPrototypeMethod(t, "setNodeOn", OZWDriver::SetNodeOn);
			Nan::SetPrototypeMethod(t, "setNodeOff", OZWDriver::SetNodeOff);
			Nan::SetPrototypeMethod(t, "setNodeLevel", OZWDriver::SetNodeLevel);
			Nan::SetPrototypeMethod(t, "switchAllOn", OZWDriver::SwitchAllOn);
			Nan::SetPrototypeMethod(t, "switchAllOff", OZWDriver::SwitchAllOff);
			Nan::SetPrototypeMethod(t, "pressButton", OZWDriver::PressButton);
			Nan::SetPrototypeMethod(t, "releaseButton", OZWDriver::ReleaseButton);
			//
			Nan::SetPrototypeMethod(t, "refreshNodeInfo", OZWDriver::RefreshNodeInfo); // ** new
			Nan::SetPrototypeMethod(t, "requestNodeState", OZWDriver::RequestNodeState); // ** new
			Nan::SetPrototypeMethod(t, "requestNodeDynamic", OZWDriver::RequestNodeDynamic); // ** new
			// getter-setter pairs
			Nan::SetPrototypeMethod(t, "getNodeLocation", OZWDriver::GetNodeLocation); // ** new
			Nan::SetPrototypeMethod(t, "setNodeLocation", OZWDriver::SetNodeLocation);
			Nan::SetPrototypeMethod(t, "getNodeName", OZWDriver::GetNodeName);
			Nan::SetPrototypeMethod(t, "setNodeName", OZWDriver::SetNodeName);
			Nan::SetPrototypeMethod(t, "getNodeManufacturerName", OZWDriver::GetNodeManufacturerName); // ** new
			Nan::SetPrototypeMethod(t, "setNodeManufacturerName", OZWDriver::SetNodeManufacturerName); // ** new
			Nan::SetPrototypeMethod(t, "getNodeProductName", OZWDriver::GetNodeProductName); // ** new
			Nan::SetPrototypeMethod(t, "setNodeProductName", OZWDriver::SetNodeProductName); // ** new
			// getters
			Nan::SetPrototypeMethod(t, "getNodeMaxBaudRate", OZWDriver::GetNodeMaxBaudRate); // ** new
			Nan::SetPrototypeMethod(t, "getNodeVersion", OZWDriver::GetNodeVersion); // ** new
			Nan::SetPrototypeMethod(t, "getNodeBasic", OZWDriver::GetNodeBasic); // ** new
			Nan::SetPrototypeMethod(t, "getNodeGeneric", OZWDriver::GetNodeGeneric); // ** new
			Nan::SetPrototypeMethod(t, "getNodeManufacturerId", OZWDriver::GetNodeManufacturerId); // ** new
			Nan::SetPrototypeMethod(t, "getNodeNeighbors", OZWDriver::GetNodeNeighbors);
			Nan::SetPrototypeMethod(t, "getNodeProductId", OZWDriver::GetNodeProductId); // ** new
			Nan::SetPrototypeMethod(t, "getNodeProductType", OZWDriver::GetNodeProductType); // ** new
			Nan::SetPrototypeMethod(t, "getNodeSecurity", OZWDriver::GetNodeSecurity); // ** new
			Nan::SetPrototypeMethod(t, "getNodeSpecific", OZWDriver::GetNodeSpecific); // ** new
			Nan::SetPrototypeMethod(t, "getNodeType", OZWDriver::GetNodeType); // ** new
			Nan::SetPrototypeMethod(t, "isNodeListeningDevice", OZWDriver::IsNodeListeningDevice); // ** new
			Nan::SetPrototypeMethod(t, "isNodeFrequentListeningDevice", OZWDriver::IsNodeFrequentListeningDevice); // ** new
			Nan::SetPrototypeMethod(t, "isNodeBeamingDevice", OZWDriver::IsNodeBeamingDevice); // ** new
			Nan::SetPrototypeMethod(t, "isNodeRoutingDevice", OZWDriver::IsNodeRoutingDevice); // ** new
			Nan::SetPrototypeMethod(t, "isNodeSecurityDevice", OZWDriver::IsNodeSecurityDevice); // ** new
			// openzwave-values.cc
			Nan::SetPrototypeMethod(t, "setValue", OZWDriver::SetValue);
			Nan::SetPrototypeMethod(t, "refreshValue", OZWDriver::RefreshValue);
			Nan::SetPrototypeMethod(t, "setChangeVerified", OZWDriver::SetChangeVerified);
			Nan::SetPrototypeMethod(t, "getNumSwitchPoints", OZWDriver::GetNumSwitchPoints);
			Nan::SetPrototypeMethod(t, "clearSwitchPoints", OZWDriver::ClearSwitchPoints);
			Nan::SetPrototypeMethod(t, "getSwitchPoint", OZWDriver::GetSwitchPoint);
			Nan::SetPrototypeMethod(t, "setSwitchPoint", OZWDriver::SetSwitchPoint);
			Nan::SetPrototypeMethod(t, "removeSwitchPoint", OZWDriver::RemoveSwitchPoint);
			// openzwave-polling.cc
			Nan::SetPrototypeMethod(t, "enablePoll", OZWDriver::EnablePoll);
			Nan::SetPrototypeMethod(t, "disablePoll", OZWDriver::DisablePoll);
			Nan::SetPrototypeMethod(t, "isPolled", OZWDriver::IsPolled); // ** new
			Nan::SetPrototypeMethod(t, "getPollInterval", OZWDriver::GetPollInterval); // ** new
			Nan::SetPrototypeMethod(t, "setPollInterval", OZWDriver::SetPollInterval); // ** new
			Nan::SetPrototypeMethod(t, "getPollIntensity", OZWDriver::GetPollIntensity); // ** new
			Nan::SetPrototypeMethod(t, "setPollIntensity", OZWDriver::SetPollIntensity); // ** new
			// openzwave-scenes.cc
			Nan::SetPrototypeMethod(t, "createScene", OZWDriver::CreateScene);
			Nan::SetPrototypeMethod(t, "removeScene", OZWDriver::RemoveScene);
			Nan::SetPrototypeMethod(t, "getScenes", OZWDriver::GetScenes);
			Nan::SetPrototypeMethod(t, "addSceneValue", OZWDriver::AddSceneValue);
			Nan::SetPrototypeMethod(t, "removeSceneValue", OZWDriver::RemoveSceneValue);
			Nan::SetPrototypeMethod(t, "sceneGetValues", OZWDriver::SceneGetValues);
			Nan::SetPrototypeMethod(t, "activateScene", OZWDriver::ActivateScene);
			//
			Nan::SetPrototypeMethod(t, "toString", OZWDriver::ToString);
			v8::Local<v8::Value> argv[1] = { Nan::New<Integer>(notif->homeid) };
			v8::Local<v8::Function> f = t->GetFunction();
			v8::MaybeLocal<v8::Object> instance = f->NewInstance(Nan::GetCurrentContext(), 1, argv);
			//
		    emitinfo[0] = Nan::New<String>("driver ready").ToLocalChecked();
		    emitinfo[1] = Nan::New<Integer>(notif->homeid);
			emitinfo[2] = instance.ToLocalChecked();
		    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
		    break;
		  }
		  //                            ##################
		  case OpenZWave::Notification::Type_DriverFailed: {
		    //                            ##################
		    emitinfo[0] = Nan::New<String>("driver failed").ToLocalChecked();
		    emit_cb->Call(Nan::New(ctx_obj), 1, emitinfo);
		    break;
		  }
  //                            ################
  case OpenZWave::Notification::Type_ValueAdded: {
    //                            ################
    OpenZWave::ValueID value = notif->values.front();
    Local<Object> valobj = zwaveValue2v8Value(value);

    if ((node = get_node_info(notif->nodeid))) {
      mutex::scoped_lock sl(znodes_mutex);
      node->values.push_back(value);
    }

    emitinfo[0] = Nan::New<String>("value added").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(value.GetCommandClassId());
    emitinfo[3] = valobj;
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
  }
  //                            ##################
  case OpenZWave::Notification::Type_ValueRemoved: {
    //                            ##################
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
    emitinfo[0] = Nan::New<String>("value removed").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(value.GetCommandClassId());
    emitinfo[3] = Nan::New<Integer>(value.GetInstance());
    emitinfo[4] = Nan::New<Integer>(value.GetIndex());
    emit_cb->Call(Nan::New(ctx_obj), 5, emitinfo);
    break;
  }
  //                            ##################
  case OpenZWave::Notification::Type_ValueChanged: {
    //                            ##################
    OpenZWave::ValueID value = notif->values.front();
    Local<Object> valobj = zwaveValue2v8Value(value);
    emitinfo[0] = Nan::New<String>("value changed").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(value.GetCommandClassId());
    emitinfo[3] = valobj;
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
  }
  //                            ####################
  case OpenZWave::Notification::Type_ValueRefreshed: {
    //                            ####################
    OpenZWave::ValueID value = notif->values.front();
    Local<Object> valobj = zwaveValue2v8Value(value);
    emitinfo[0] = Nan::New<String>("value refreshed").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(value.GetCommandClassId());
    emitinfo[3] = valobj;
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
  }
  //                            #############
  case OpenZWave::Notification::Type_NodeNew:
    //                            #############
    /* NodeNew is triggered when a node is discovered which is not
    * found in the OpenZWave XML file.  As we do not use that file
    * simply ignore those notifications for now.
    *
    * NodeAdded is when we actually have a new node to set up.
    */
    break;
  //                            ###############
  case OpenZWave::Notification::Type_NodeAdded: {
    //                            ###############
    node = new NodeInfo();
    node->homeid = notif->homeid;
    node->nodeid = notif->nodeid;
    node->polled = false;
    {
      mutex::scoped_lock sl(znodes_mutex);
      znodes[notif->nodeid] = node;
    }
    emitinfo[0] = Nan::New<String>("node added").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emit_cb->Call(Nan::New(ctx_obj), 2, emitinfo);
    break;
  }
  //                            #################
  case OpenZWave::Notification::Type_NodeRemoved: {
    //                            #################
    {
      mutex::scoped_lock sl(znodes_mutex);
      znodes.erase(notif->nodeid);
    }
    emitinfo[0] = Nan::New<String>("node removed").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emit_cb->Call(Nan::New(ctx_obj), 2, emitinfo);
    break;
  }
  //                            ######################
  case OpenZWave::Notification::Type_NodeProtocolInfo: {
    //                            ######################
    /*
    * Ignore intermediate notifications about a node status, we
    * wait until the node is ready before retrieving information.
    */
    break;
  }
  //                            ################
  case OpenZWave::Notification::Type_NodeNaming: {
    //                            ################
    populateNode(cbinfo, notif->homeid, notif->nodeid);
    emitinfo[0] = Nan::New<String>("node naming").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = cbinfo;
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ###############
  case OpenZWave::Notification::Type_NodeEvent: {
    //                            ###############
    emitinfo[0] = Nan::New<String>("node event").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->event);
    emitinfo[3] = Nan::New<String>(notif->help.c_str()).ToLocalChecked();
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
  }
  //                            #####################
  case OpenZWave::Notification::Type_PollingDisabled: {
    //                            #####################
    if ((node = get_node_info(notif->nodeid))) {
      node->polled = false;
      emitinfo[0] = Nan::New<String>("polling disabled").ToLocalChecked();
      emitinfo[1] = Nan::New<Integer>(notif->nodeid);
      emit_cb->Call(Nan::New(ctx_obj), 2, emitinfo);
    }
    break;
  }
  //                            ####################
  case OpenZWave::Notification::Type_PollingEnabled: {
    //                            ####################
    if ((node = get_node_info(notif->nodeid))) {
      node->polled = true;
      emitinfo[0] = Nan::New<String>("polling enabled").ToLocalChecked();
      emitinfo[1] = Nan::New<Integer>(notif->nodeid);
      emit_cb->Call(Nan::New(ctx_obj), 2, emitinfo);
    }
    break;
  }
  //                            ################
  case OpenZWave::Notification::Type_SceneEvent: {
    //                            ################
    emitinfo[0] = Nan::New<String>("scene event").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->sceneid);
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ##################
  case OpenZWave::Notification::Type_CreateButton: {
    //                            ##################
    emitinfo[0] = Nan::New<String>("create button").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->buttonid);
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ##################
  case OpenZWave::Notification::Type_DeleteButton: {
    //                            ##################
    emitinfo[0] = Nan::New<String>("delete button").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->buttonid);
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ##############
  case OpenZWave::Notification::Type_ButtonOn: {
    //                            ##############
    emitinfo[0] = Nan::New<String>("button on").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->buttonid);
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ###############
  case OpenZWave::Notification::Type_ButtonOff: {
    //                            ###############
    emitinfo[0] = Nan::New<String>("button off").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->buttonid);
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  //                            ##################################
  case OpenZWave::Notification::Type_EssentialNodeQueriesComplete: {
    //                            ##################################
    populateNode(cbinfo, notif->homeid, notif->nodeid);
    emitinfo[0] = Nan::New<String>("node available").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = cbinfo;
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  /*
  * The node is now fully ready for operation.
  */
  //                            #########################
  case OpenZWave::Notification::Type_NodeQueriesComplete: {
    //                            #########################
    populateNode(cbinfo, notif->homeid, notif->nodeid);
    emitinfo[0] = Nan::New<String>("node ready").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = cbinfo;
    emit_cb->Call(Nan::New(ctx_obj), 3, emitinfo);
    break;
  }
  /*
  * The network scan has been completed.  Currently we do not
  * care about dead nodes - is there anything we can do anyway?
  */
  //                            #########################
  case OpenZWave::Notification::Type_AwakeNodesQueried:
  case OpenZWave::Notification::Type_AllNodesQueried:
  case OpenZWave::Notification::Type_AllNodesQueriedSomeDead: {
    //                            #############################
    emitinfo[0] = Nan::New<String>("scan complete").ToLocalChecked();
    emit_cb->Call(Nan::New(ctx_obj), 1, emitinfo);
    break;
  }
  //                            ##################
  case OpenZWave::Notification::Type_Notification: {
    //                            #########################
    emitinfo[0] = Nan::New<String>("notification").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->notification);
    emitinfo[3] = Nan::New<String>(notif->help.c_str()).ToLocalChecked();
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
  }
  case OpenZWave::Notification::Type_DriverRemoved:
  case OpenZWave::Notification::Type_Group:
    /* The associations for the node have changed. The
     * application should rebuild any group information it
     * holds about the node.
     */
    // todo
    break;
#if OPENZWAVE_SECURITY == 1
  case OpenZWave::Notification::Type_ControllerCommand:
    emitinfo[0] = Nan::New<String>("controller command").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->event); // Driver::ControllerCommand
    emitinfo[3] =
        Nan::New<Integer>(notif->notification); // Driver::ControllerState
    emitinfo[4] = Nan::New<String>(notif->help.c_str()).ToLocalChecked();
    emit_cb->Call(Nan::New(ctx_obj), 5, emitinfo);
    break;
  case OpenZWave::Notification::Type_NodeReset:
    emitinfo[0] = Nan::New<String>("node reset").ToLocalChecked();
    emitinfo[1] = Nan::New<Integer>(notif->nodeid);
    emitinfo[2] = Nan::New<Integer>(notif->event); // Driver::ControllerCommand
    emitinfo[3] =
        Nan::New<Integer>(notif->notification); // Driver::ControllerState
    emit_cb->Call(Nan::New(ctx_obj), 4, emitinfo);
    break;
#endif
  default:
    fprintf(stderr, "Unhandled OpenZWave notification: %d\n", notif->type);
    break;
  } // end switch
}

/*
* Async handler, triggered by the OpenZWave callback.
*/
// ===================================================================
void async_cb_handler(uv_async_t *handle)
// ===================================================================
{
  NotifInfo *notif;

  mutex::scoped_lock sl(zqueue_mutex);

  while (!zqueue.empty()) {
    notif = zqueue.front();
		zqueue.pop();
		zqueue_mutex.unlock();
#if OPENZWAVE_SECURITY != 1
    if (notif->homeid == 0) {
      handleControllerCommand(notif);
    } else {
      handleNotification(notif);
    }
#else
    handleNotification(notif);
#endif
    delete notif;
  }
}

void async_cb_handler(uv_async_t *handle, int status) {
  async_cb_handler(handle);
}
}
