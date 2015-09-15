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

	//
	uv_async_t 		async;

	// 
	NanCallback *emit_cb;
	
	// Message passing queue between OpenZWave callback and v8 async handler.
	mutex 		zqueue_mutex;
	std::queue<NotifInfo *> zqueue;

	// Node state.
	mutex 		znodes_mutex;
	std::list<NodeInfo *> znodes;

	mutex zscenes_mutex;
	std::list<SceneInfo *> zscenes;

	uint32_t homeid;
	CommandMap* ctrlCmdNames;

	// ===================================================================
	NAN_METHOD(OZW::OZW::New)
	// ===================================================================
	{
		NanScope();

		assert(args.IsConstructCall());
		OZW* self = new OZW();
		self->Wrap(args.This());

		Local < Object > opts = args[0]->ToObject();
		std::string confpath = (*String::Utf8Value(opts->Get(NanNew<String>("modpath")->ToString())));
		confpath += "/../deps/open-zwave/config";

		/*
		* Options are global for all drivers and can only be set once.
		*/
		OpenZWave::Options::Create(confpath.c_str(), "", "");
		OpenZWave::Options::Get()->AddOptionBool("ConsoleOutput",     opts->Get(NanNew<String>("consoleoutput"))->BooleanValue());
		OpenZWave::Options::Get()->AddOptionBool("Logging",           opts->Get(NanNew<String>("logging"))->BooleanValue());
		OpenZWave::Options::Get()->AddOptionBool("SaveConfiguration", opts->Get(NanNew<String>("saveconfig"))->BooleanValue());
		OpenZWave::Options::Get()->AddOptionInt("DriverMaxAttempts",  opts->Get(NanNew<String>("driverattempts"))->IntegerValue());
		OpenZWave::Options::Get()->AddOptionInt("PollInterval",       opts->Get(NanNew<String>("pollinterval"))->IntegerValue());
		OpenZWave::Options::Get()->AddOptionBool("IntervalBetweenPolls", true);
		OpenZWave::Options::Get()->AddOptionBool("SuppressValueRefresh", opts->Get(NanNew<String>("suppressrefresh"))->BooleanValue());
		OpenZWave::Options::Get()->Lock();

		NanReturnValue(args.This());
	}

	// ===================================================================
	extern "C" void init(Handle<Object> target) {
  
		NanScope();
		Local < FunctionTemplate > t = NanNew<FunctionTemplate>(OZW::New);
		t->SetClassName(NanNew("OZW"));	
		t->InstanceTemplate()->SetInternalFieldCount(1);
		
		// openzwave-config.cc
		NODE_SET_PROTOTYPE_METHOD(t, "setConfigParam", OZW::SetConfigParam);
		NODE_SET_PROTOTYPE_METHOD(t, "requestConfigParam", OZW::RequestConfigParam);
		NODE_SET_PROTOTYPE_METHOD(t, "requestAllConfigParams", OZW::RequestAllConfigParams);
		// openzwave-controller.cc
		NODE_SET_PROTOTYPE_METHOD(t, "hardReset", OZW::HardReset);
		NODE_SET_PROTOTYPE_METHOD(t, "softReset", OZW::SoftReset);
		NODE_SET_PROTOTYPE_METHOD(t, "beginControllerCommand", OZW::BeginControllerCommand); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "cancelControllerCommand", OZW::CancelControllerCommand); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getControllerNodeId", OZW::GetControllerNodeId); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getSUCNodeId", OZW::GetSUCNodeId); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isPrimaryController", OZW::IsPrimaryController); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isStaticUpdateController", OZW::IsStaticUpdateController); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isBridgeController", OZW::IsBridgeController); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getLibraryVersion", OZW::GetLibraryVersion); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getLibraryTypeName", OZW::GetLibraryTypeName); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getSendQueueCount", OZW::GetSendQueueCount);	// ** new
		// openzwave-driver.cc
		NODE_SET_PROTOTYPE_METHOD(t, "connect", OZW::Connect);
		NODE_SET_PROTOTYPE_METHOD(t, "disconnect", OZW::Disconnect);
		// openzwave-groups.cc
		NODE_SET_PROTOTYPE_METHOD(t, "getNumGroups", OZW::GetNumGroups);
		NODE_SET_PROTOTYPE_METHOD(t, "getAssociations", OZW::GetAssociations);
		NODE_SET_PROTOTYPE_METHOD(t, "getMaxAssociations", OZW::GetMaxAssociations);
		NODE_SET_PROTOTYPE_METHOD(t, "getGroupLabel", OZW::GetGroupLabel);
		NODE_SET_PROTOTYPE_METHOD(t, "addAssociation", OZW::AddAssociation);
		NODE_SET_PROTOTYPE_METHOD(t, "removeAssociation", OZW::RemoveAssociation);
		// openzwave-network.cc
		NODE_SET_PROTOTYPE_METHOD(t, "testNetworkNode", OZW::TestNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "testNetwork", OZW::TestNetwork);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetworkNode", OZW::HealNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetwork", OZW::HealNetwork);
		// openzwave-nodes.cc
		NODE_SET_PROTOTYPE_METHOD(t, "setLocation", OZW::SetLocation);
		NODE_SET_PROTOTYPE_METHOD(t, "setName", OZW::SetName);
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeOn", OZW::SetNodeOn);
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeOff", OZW::SetNodeOff);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOn", OZW::SwitchAllOn);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOff", OZW::SwitchAllOff);
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeNeighbors", OZW::GetNodeNeighbors);
		NODE_SET_PROTOTYPE_METHOD(t, "refreshNodeInfo", OZW::RefreshNodeInfo);
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeManufacturerName", OZW::GetNodeManufacturerName); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "requestNodeState", OZW::RequestNodeState); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "requestNodeDynamic", OZW::RequestNodeDynamic); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isNodeListeningDevice", OZW::IsNodeListeningDevice); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isNodeFrequentListeningDevice", OZW::IsNodeFrequentListeningDevice); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isNodeBeamingDevice", OZW::IsNodeBeamingDevice); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isNodeRoutingDevice", OZW::IsNodeRoutingDevice); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "isNodeSecurityDevice", OZW::IsNodeSecurityDevice); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeMaxBaudRate", OZW::GetNodeMaxBaudRate); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeVersion", OZW::GetNodeVersion); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeSecurity", OZW::GetNodeSecurity); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeBasic", OZW::GetNodeBasic); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeGeneric", OZW::GetNodeGeneric); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeSpecific", OZW::GetNodeSpecific); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeType", OZW::GetNodeType); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeProductName", OZW::GetNodeProductName); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeName", OZW::GetNodeName); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeLocation", OZW::GetNodeLocation); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeManufacturerId", OZW::GetNodeManufacturerId); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeProductType", OZW::GetNodeProductType); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeProductId", OZW::GetNodeProductId); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeManufacturerName", OZW::SetNodeManufacturerName); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "setNodeProductName", OZW::SetNodeProductName); // ** new
		// openzwave-values.cc
		NODE_SET_PROTOTYPE_METHOD(t, "setValue", OZW::SetValue);
		// openzwave-polling.cc
		NODE_SET_PROTOTYPE_METHOD(t, "enablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "disablePoll", OZW::DisablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "isPolled",  OZW::IsPolled); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getPollInterval",  OZW::GetPollInterval); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "setPollInterval",  OZW::SetPollInterval); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "getPollIntensity",  OZW::GetPollIntensity); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "setPollIntensity",  OZW::SetPollIntensity); // ** new
		// openzwave-scenes.cc
		NODE_SET_PROTOTYPE_METHOD(t, "createScene", OZW::CreateScene);
		NODE_SET_PROTOTYPE_METHOD(t, "removeScene", OZW::RemoveScene);
		NODE_SET_PROTOTYPE_METHOD(t, "getScenes", OZW::GetScenes);
		NODE_SET_PROTOTYPE_METHOD(t, "addSceneValue", OZW::AddSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "removeSceneValue", OZW::RemoveSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "sceneGetValues", OZW::SceneGetValues);
		NODE_SET_PROTOTYPE_METHOD(t, "activateScene", OZW::ActivateScene);
		//
		target->Set(NanNew<String>("Emitter"), t->GetFunction());
		
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
		(*ctrlCmdNames)["HasNodeFailed"]			= OpenZWave::Driver::ControllerCommand_HasNodeFailed;
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

NODE_MODULE(openzwave_shared, OZW::init)
