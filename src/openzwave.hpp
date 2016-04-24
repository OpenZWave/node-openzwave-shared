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
//=================================
#ifndef __NODE_OPENZWAVE_HPP_INCLUDED__
#define __NODE_OPENZWAVE_HPP_INCLUDED__

#include <iostream>
#include <sstream>
#include <list>
#include <queue>

#include <v8.h>
#include "nan.h"

#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Notification.h"
#include "Options.h"
#include "Value.h"

#if defined(_WIN32) || defined( __APPLE__)
    #include <unordered_map>
    typedef ::std::unordered_map <std::string, OpenZWave::Driver::ControllerCommand> CommandMap;
#else
    #include <tr1/unordered_map>
    typedef ::std::tr1::unordered_map <std::string, OpenZWave::Driver::ControllerCommand> CommandMap;
#endif

#include "utils.hpp"

using namespace v8;
using namespace node;

namespace OZW {

	struct OZW : public ObjectWrap {
		static NAN_METHOD(New);
		// openzwave-config.cc
		static NAN_METHOD(SetConfigParam);
		static NAN_METHOD(RequestConfigParam);
		static NAN_METHOD(RequestAllConfigParams);
		// openzwave-controller.cc
		static NAN_METHOD(HardReset);
		static NAN_METHOD(SoftReset);
		static NAN_METHOD(GetControllerNodeId);
		static NAN_METHOD(GetSUCNodeId);
		static NAN_METHOD(IsPrimaryController);
		static NAN_METHOD(IsStaticUpdateController);
		static NAN_METHOD(IsBridgeController);
		static NAN_METHOD(GetLibraryVersion);
		static NAN_METHOD(GetLibraryTypeName);
		static NAN_METHOD(GetSendQueueCount);
		// openzwave-driver.cc
		static NAN_METHOD(Connect);
		static NAN_METHOD(Disconnect);
		// openzwave-groups.cc
		static NAN_METHOD(GetNumGroups);
		static NAN_METHOD(GetAssociations);
		static NAN_METHOD(GetMaxAssociations);
		static NAN_METHOD(GetGroupLabel);
		static NAN_METHOD(AddAssociation);
		static NAN_METHOD(RemoveAssociation);
#if OPENZWAVE_SECURITY == 1
		static NAN_METHOD(AddNode);
		static NAN_METHOD(RemoveNode);
		static NAN_METHOD(RemoveFailedNode);
		static NAN_METHOD(HasNodeFailed);
		static NAN_METHOD(AssignReturnRoute);
		static NAN_METHOD(RequestNodeNeighborUpdate);
		static NAN_METHOD(DeleteAllReturnRoutes);
		static NAN_METHOD(SendNodeInformation);
		static NAN_METHOD(CreateNewPrimary);
		static NAN_METHOD(ReceiveConfiguration);
		static NAN_METHOD(ReplaceFailedNode);
		static NAN_METHOD(TransferPrimaryRole);
		static NAN_METHOD(RequestNetworkUpdate);
		static NAN_METHOD(ReplicationSend);
		static NAN_METHOD(CreateButton);
		static NAN_METHOD(DeleteButton);
#else
		static NAN_METHOD(BeginControllerCommand);
#endif
		static NAN_METHOD(CancelControllerCommand);
		// openzwave-network.cc
		static NAN_METHOD(TestNetworkNode);
		static NAN_METHOD(TestNetwork);
		static NAN_METHOD(HealNetworkNode);
		static NAN_METHOD(HealNetwork);
		// openzwave-nodes.cc
		static NAN_METHOD(SetNodeOn);
		static NAN_METHOD(SetNodeOff);
		static NAN_METHOD(SetNodeLevel);
		static NAN_METHOD(SwitchAllOn);
		static NAN_METHOD(SwitchAllOff);
    static NAN_METHOD(PressButton);
		//
		static NAN_METHOD(RefreshNodeInfo);
		static NAN_METHOD(RequestNodeState);
		static NAN_METHOD(RequestNodeDynamic);
		static NAN_METHOD(IsNodeListeningDevice);
		static NAN_METHOD(IsNodeFrequentListeningDevice);
		static NAN_METHOD(IsNodeBeamingDevice);
		static NAN_METHOD(IsNodeRoutingDevice);
		static NAN_METHOD(IsNodeSecurityDevice);
		// getter+setter pairs
		static NAN_METHOD(GetNodeLocation);
		static NAN_METHOD(SetNodeLocation);
		static NAN_METHOD(GetNodeName);
		static NAN_METHOD(SetNodeName);
		static NAN_METHOD(GetNodeManufacturerName);
		static NAN_METHOD(SetNodeManufacturerName);
		static NAN_METHOD(GetNodeProductName);
		static NAN_METHOD(SetNodeProductName);
		// plain getters
		static NAN_METHOD(GetNodeMaxBaudRate);
		static NAN_METHOD(GetNodeVersion);
		static NAN_METHOD(GetNodeSecurity);
		static NAN_METHOD(GetNodeBasic);
		static NAN_METHOD(GetNodeGeneric);
		static NAN_METHOD(GetNodeSpecific);
		static NAN_METHOD(GetNodeType);
		static NAN_METHOD(GetNodeManufacturerId);
		static NAN_METHOD(GetNodeProductType);
		static NAN_METHOD(GetNodeProductId);
		static NAN_METHOD(GetNodeNeighbors);
		// openzwave-values.cc
		static NAN_METHOD(SetValue);
		static NAN_METHOD(RefreshValue);
		static NAN_METHOD(SetChangeVerified);
    static NAN_METHOD(GetNumSwitchPoints);
    static NAN_METHOD(GetSwitchPoint);
    static NAN_METHOD(ClearSwitchPoints);
    static NAN_METHOD(SetSwitchPoint);
    static NAN_METHOD(RemoveSwitchPoint);
		// openzwave-polling.cc
		static NAN_METHOD(GetPollInterval);
		static NAN_METHOD(SetPollInterval);
		static NAN_METHOD(EnablePoll);
		static NAN_METHOD(DisablePoll);
		static NAN_METHOD(IsPolled);
		static NAN_METHOD(SetPollIntensity);
		static NAN_METHOD(GetPollIntensity);
		// openzwave-scenes.cc
		static NAN_METHOD(CreateScene);
		static NAN_METHOD(RemoveScene);
		static NAN_METHOD(GetScenes);
		static NAN_METHOD(AddSceneValue);
		static NAN_METHOD(RemoveSceneValue);
		static NAN_METHOD(SceneGetValues);
		static NAN_METHOD(ActivateScene);

    // Passing configuration around
    std::string userpath;
    std::string option_overrides;
    std::string config_path;
	};

	// our ZWave Home ID
	extern uint32 homeid;

	// map of controller command names to enum values
	extern CommandMap* ctrlCmdNames;

}

// OpenZWave version constituents
extern uint16_t ozw_vers_major;
extern uint16_t ozw_vers_minor;
extern uint16_t ozw_vers_revision;

#endif // __NODE_OPENZWAVE_HPP_INCLUDED__
