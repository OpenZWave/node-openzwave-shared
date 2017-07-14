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

#include <algorithm>

using namespace v8;
using namespace node;

namespace OZW {

	uint32      homeid;
	CommandMap* ctrlCmdNames;

	std::string ozw_userpath;
	const std::string ozw_config_path  = stringify( OPENZWAVE_ETC );

	// ===================================================================
	extern "C" void init(Handle<Object> target, Handle<Object> module) {

		Nan::HandleScope scope;

    // get the user config directory
		std::string modulefilename = std::string(*v8::String::Utf8Value(
			Nan::Get(module,
          Nan::New("filename").ToLocalChecked()
		  ).ToLocalChecked()
		));
		std::size_t found = modulefilename.find_last_of("/\\");
		if (found > 0) {
			//std::cout << " path: " << modulefilename.substr(0,found) << '\n';
			//std::cout << " file: " << modulefilename.substr(found+1) << '\n';
			ozw_userpath.assign(modulefilename.substr(0,found));
    } else {
      ozw_userpath.assign(modulefilename);
    }
		ozw_userpath.append("/../../");

		Local < FunctionTemplate > t = Nan::New<FunctionTemplate>(OZW::New);
		t->SetClassName(Nan::New("OZW").ToLocalChecked());
		t->InstanceTemplate()->SetInternalFieldCount(1);

		// openzwave-config.cc
		Nan::SetPrototypeMethod(t, "setConfigParam", OZW::SetConfigParam);
		Nan::SetPrototypeMethod(t, "requestConfigParam", OZW::RequestConfigParam);
		Nan::SetPrototypeMethod(t, "requestAllConfigParams", OZW::RequestAllConfigParams);
		// openzwave-controller.cc
		Nan::SetPrototypeMethod(t, "hardReset", OZW::HardReset);
		Nan::SetPrototypeMethod(t, "softReset", OZW::SoftReset);
		Nan::SetPrototypeMethod(t, "getControllerNodeId", OZW::GetControllerNodeId); // ** new
		Nan::SetPrototypeMethod(t, "getSUCNodeId", OZW::GetSUCNodeId); // ** new
		Nan::SetPrototypeMethod(t, "isPrimaryController", OZW::IsPrimaryController); // ** new
		Nan::SetPrototypeMethod(t, "isStaticUpdateController", OZW::IsStaticUpdateController); // ** new
		Nan::SetPrototypeMethod(t, "isBridgeController", OZW::IsBridgeController); // ** new
		Nan::SetPrototypeMethod(t, "getLibraryVersion", OZW::GetLibraryVersion); // ** new
		Nan::SetPrototypeMethod(t, "getLibraryTypeName", OZW::GetLibraryTypeName); // ** new
		Nan::SetPrototypeMethod(t, "getSendQueueCount", OZW::GetSendQueueCount);	// ** new
		// openzwave-driver.cc
		Nan::SetPrototypeMethod(t, "connect", OZW::Connect);
		Nan::SetPrototypeMethod(t, "disconnect", OZW::Disconnect);
		// openzwave-groups.cc
		Nan::SetPrototypeMethod(t, "getNumGroups", OZW::GetNumGroups);
		Nan::SetPrototypeMethod(t, "getAssociations", OZW::GetAssociations);
		Nan::SetPrototypeMethod(t, "getMaxAssociations", OZW::GetMaxAssociations);
		Nan::SetPrototypeMethod(t, "getGroupLabel", OZW::GetGroupLabel);
		Nan::SetPrototypeMethod(t, "addAssociation", OZW::AddAssociation);
		Nan::SetPrototypeMethod(t, "removeAssociation", OZW::RemoveAssociation);
		// openzwave-management.cc
#if OPENZWAVE_SECURITY == 1
		Nan::SetPrototypeMethod(t, "addNode", OZW::AddNode);
	  Nan::SetPrototypeMethod(t, "removeNode", OZW::RemoveNode);
	  Nan::SetPrototypeMethod(t, "removeFailedNode", OZW::RemoveFailedNode);
	  Nan::SetPrototypeMethod(t, "hasNodeFailed", OZW::HasNodeFailed);
	  Nan::SetPrototypeMethod(t, "requestNodeNeighborUpdate", OZW::RequestNodeNeighborUpdate);
	  Nan::SetPrototypeMethod(t, "assignReturnRoute", OZW::AssignReturnRoute);
	  Nan::SetPrototypeMethod(t, "deleteAllReturnRoutes", OZW::DeleteAllReturnRoutes);
	  Nan::SetPrototypeMethod(t, "sendNodeInformation", OZW::SendNodeInformation);
	  Nan::SetPrototypeMethod(t, "createNewPrimary", OZW::CreateNewPrimary);
	  Nan::SetPrototypeMethod(t, "receiveConfiguration", OZW::ReceiveConfiguration);
	  Nan::SetPrototypeMethod(t, "replaceFailedNode", OZW::ReplaceFailedNode);
	  Nan::SetPrototypeMethod(t, "transferPrimaryRole", OZW::TransferPrimaryRole);
	  Nan::SetPrototypeMethod(t, "requestNetworkUpdate", OZW::RequestNetworkUpdate);
	  Nan::SetPrototypeMethod(t, "replicationSend", OZW::ReplicationSend);
	  Nan::SetPrototypeMethod(t, "createButton", OZW::CreateButton);
	  Nan::SetPrototypeMethod(t, "deleteButton", OZW::DeleteButton);
#else
		Nan::SetPrototypeMethod(t, "beginControllerCommand", OZW::BeginControllerCommand);
#endif
		Nan::SetPrototypeMethod(t, "cancelControllerCommand", OZW::CancelControllerCommand);
		Nan::SetPrototypeMethod(t, "writeConfig", OZW::WriteConfig);
		Nan::SetPrototypeMethod(t, "getDriverStatistics", OZW::GetDriverStatistics);
		Nan::SetPrototypeMethod(t, "getNodeStatistics", OZW::GetNodeStatistics);
		// openzwave-network.cc
		Nan::SetPrototypeMethod(t, "testNetworkNode", OZW::TestNetworkNode);
		Nan::SetPrototypeMethod(t, "testNetwork", OZW::TestNetwork);
		Nan::SetPrototypeMethod(t, "healNetworkNode", OZW::HealNetworkNode);
		Nan::SetPrototypeMethod(t, "healNetwork", OZW::HealNetwork);
		// openzwave-nodes.cc
		Nan::SetPrototypeMethod(t, "setNodeOn", OZW::SetNodeOn);
		Nan::SetPrototypeMethod(t, "setNodeOff", OZW::SetNodeOff);
		Nan::SetPrototypeMethod(t, "setNodeLevel", OZW::SetNodeLevel);
		Nan::SetPrototypeMethod(t, "switchAllOn", OZW::SwitchAllOn);
		Nan::SetPrototypeMethod(t, "switchAllOff", OZW::SwitchAllOff);
		Nan::SetPrototypeMethod(t, "pressButton", OZW::PressButton);
		Nan::SetPrototypeMethod(t, "releaseButton", OZW::ReleaseButton);
		//
		Nan::SetPrototypeMethod(t, "refreshNodeInfo", OZW::RefreshNodeInfo); // ** new
		Nan::SetPrototypeMethod(t, "requestNodeState", OZW::RequestNodeState); // ** new
		Nan::SetPrototypeMethod(t, "requestNodeDynamic", OZW::RequestNodeDynamic); // ** new
		// getter-setter pairs
		Nan::SetPrototypeMethod(t, "getNodeLocation", OZW::GetNodeLocation); // ** new
		Nan::SetPrototypeMethod(t, "setNodeLocation", OZW::SetNodeLocation);
		Nan::SetPrototypeMethod(t, "getNodeName", OZW::GetNodeName);
		Nan::SetPrototypeMethod(t, "setNodeName", OZW::SetNodeName);
		Nan::SetPrototypeMethod(t, "getNodeManufacturerName", OZW::GetNodeManufacturerName); // ** new
		Nan::SetPrototypeMethod(t, "setNodeManufacturerName", OZW::SetNodeManufacturerName); // ** new
		Nan::SetPrototypeMethod(t, "getNodeProductName", OZW::GetNodeProductName); // ** new
		Nan::SetPrototypeMethod(t, "setNodeProductName", OZW::SetNodeProductName); // ** new
		// getters
		Nan::SetPrototypeMethod(t, "getNodeMaxBaudRate", OZW::GetNodeMaxBaudRate); // ** new
		Nan::SetPrototypeMethod(t, "getNodeVersion", OZW::GetNodeVersion); // ** new
		Nan::SetPrototypeMethod(t, "getNodeBasic", OZW::GetNodeBasic); // ** new
		Nan::SetPrototypeMethod(t, "getNodeGeneric", OZW::GetNodeGeneric); // ** new
		Nan::SetPrototypeMethod(t, "getNodeManufacturerId", OZW::GetNodeManufacturerId); // ** new
		Nan::SetPrototypeMethod(t, "getNodeNeighbors", OZW::GetNodeNeighbors);
		Nan::SetPrototypeMethod(t, "getNodeProductId", OZW::GetNodeProductId); // ** new
		Nan::SetPrototypeMethod(t, "getNodeProductType", OZW::GetNodeProductType); // ** new
		Nan::SetPrototypeMethod(t, "getNodeSecurity", OZW::GetNodeSecurity); // ** new
		Nan::SetPrototypeMethod(t, "getNodeSpecific", OZW::GetNodeSpecific); // ** new
		Nan::SetPrototypeMethod(t, "getNodeType", OZW::GetNodeType); // ** new
		Nan::SetPrototypeMethod(t, "isNodeListeningDevice", OZW::IsNodeListeningDevice); // ** new
		Nan::SetPrototypeMethod(t, "isNodeFrequentListeningDevice", OZW::IsNodeFrequentListeningDevice); // ** new
		Nan::SetPrototypeMethod(t, "isNodeBeamingDevice", OZW::IsNodeBeamingDevice); // ** new
		Nan::SetPrototypeMethod(t, "isNodeRoutingDevice", OZW::IsNodeRoutingDevice); // ** new
		Nan::SetPrototypeMethod(t, "isNodeSecurityDevice", OZW::IsNodeSecurityDevice); // ** new
		// openzwave-values.cc
		Nan::SetPrototypeMethod(t, "setValue", OZW::SetValue);
		Nan::SetPrototypeMethod(t, "refreshValue", OZW::RefreshValue);
		Nan::SetPrototypeMethod(t, "setChangeVerified", OZW::SetChangeVerified);
		Nan::SetPrototypeMethod(t, "getNumSwitchPoints", OZW::GetNumSwitchPoints);
		Nan::SetPrototypeMethod(t, "clearSwitchPoints", OZW::ClearSwitchPoints);
		Nan::SetPrototypeMethod(t, "getSwitchPoint", OZW::GetSwitchPoint);
		Nan::SetPrototypeMethod(t, "setSwitchPoint", OZW::SetSwitchPoint);
		Nan::SetPrototypeMethod(t, "removeSwitchPoint", OZW::RemoveSwitchPoint);
		// openzwave-polling.cc
		Nan::SetPrototypeMethod(t, "enablePoll", OZW::EnablePoll);
		Nan::SetPrototypeMethod(t, "disablePoll", OZW::DisablePoll);
		Nan::SetPrototypeMethod(t, "isPolled",  OZW::IsPolled); // ** new
		Nan::SetPrototypeMethod(t, "getPollInterval",  OZW::GetPollInterval); // ** new
		Nan::SetPrototypeMethod(t, "setPollInterval",  OZW::SetPollInterval); // ** new
		Nan::SetPrototypeMethod(t, "getPollIntensity",  OZW::GetPollIntensity); // ** new
		Nan::SetPrototypeMethod(t, "setPollIntensity",  OZW::SetPollIntensity); // ** new
		// openzwave-scenes.cc
		Nan::SetPrototypeMethod(t, "createScene", OZW::CreateScene);
		Nan::SetPrototypeMethod(t, "removeScene", OZW::RemoveScene);
		Nan::SetPrototypeMethod(t, "getScenes", OZW::GetScenes);
		Nan::SetPrototypeMethod(t, "addSceneValue", OZW::AddSceneValue);
		Nan::SetPrototypeMethod(t, "removeSceneValue", OZW::RemoveSceneValue);
		Nan::SetPrototypeMethod(t, "sceneGetValues", OZW::SceneGetValues);
		Nan::SetPrototypeMethod(t, "activateScene", OZW::ActivateScene);
		//
		Nan::Set(target, Nan::New<String>("Emitter").ToLocalChecked(), t->GetFunction());
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

  // ===================================================================
	NAN_METHOD(OZW::New)
	// ===================================================================
	{
		Nan::HandleScope scope;

		assert(info.IsConstructCall());
		OZW* self = new OZW();
		self->Wrap(info.This());
		std::string option_overrides;
		// Options are global for all drivers and can only be set once.
		if (info.Length() > 0) {
			Local < Object > opts = Nan::To<Object>(info[0]).ToLocalChecked();
			Local < Array > props = Nan::GetOwnPropertyNames(opts).ToLocalChecked();
			for (unsigned int i = 0; i < props->Length(); ++i) {
				Local<Value> key       = props->Get(i);
				std::string  keyname   = *v8::String::Utf8Value(key);
				Local<Value> argval    = Nan::Get(opts, key).ToLocalChecked();
				std::string  argvalstr = *v8::String::Utf8Value(argval);
				// UserPath is directly passed to Manager->Connect()
				// scan for OpenZWave options.xml in the nodeJS module's '/config' subdirectory
				if (keyname == "UserPath") {
					ozw_userpath.assign(argvalstr);
				} else {
					option_overrides += " --" + keyname + " " + argvalstr;
				}
			}
		}

		std::ostringstream versionstream;
		versionstream << ozw_vers_major << "." << ozw_vers_minor << "." << ozw_vers_revision;
		std::cout << "Initialising OpenZWave " << versionstream.str() << " binary addon for Node.JS.\n";

#if OPENZWAVE_SECURITY == 1
		std::cout << "\tOpenZWave Security API is ENABLED\n";
#else
		std::cout << "\tSecurity API not found, using legacy BeginControllerCommand() instead\n";
#endif

		std::cout << "\tZWave device db    : " << ozw_config_path << "\n";
		std::cout << "\tUser settings path : " << ozw_userpath << "\n";
		if (option_overrides.length() > 0) {
			std::cout << "\tOption Overrides :" << option_overrides << "\n";
		}

		// Store configuration data for connect.
		self->config_path = ozw_config_path;
		self->userpath = ozw_userpath;
		self->option_overrides = option_overrides;

		ctx_obj = Nan::Persistent<Object>(info.This());

		//
		info.GetReturnValue().Set(info.This());
	}
}

NODE_MODULE(openzwave_shared, OZW::init)
