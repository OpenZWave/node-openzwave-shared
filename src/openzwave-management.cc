/*
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
#include <algorithm>

using namespace v8;
using namespace node;

namespace OZW {

	#if OPENZWAVE_SECURITY == 1
	/* -------------------------------------------
	// OpenZWave >= 1.3: Enable Security Functions
	// and deprecate BeginControllerCommand
	----------------------------------------------*/

	/* bool 	AddNode (uint32 const _homeId, bool _doSecurity=true)
	* Start the Inclusion Process to add a Node to the Network.
	* The Status of the Node Inclusion is communicated via Notifications.
	* Specifically, you should monitor ControllerCommand Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZW::AddNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		bool doSecurity = info.Length() > 0 && info[0]->ToBoolean()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->AddNode(homeid, doSecurity)
		));
	}

	/*bool 	RemoveNode (uint32 const _homeId)
	* Remove a Device from the Z-Wave Network
	* The Status of the Node Removal is communicated via Notifications.
	* Specifically, you should monitor ControllerCommand Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZW::RemoveNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RemoveNode(homeid)
		));
	}

	/*bool 	RemoveFailedNode (uint32 const _homeId, uint8 const _nodeId)
	Remove a Failed Device from the Z-Wave Network
	This Command will remove a failed node from the network. The Node should be on
	the Controllers Failed Node List, otherwise this command will fail. You can
	use the HasNodeFailed function below to test if the Controller believes the
	Node has Failed. The Status of the Node Removal is communicated via
	Notifications. Specifically, you should monitor ControllerCommand Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZW::RemoveFailedNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RemoveFailedNode(homeid, nodeid)
		));
	}

	/*bool 	HasNodeFailed (uint32 const _homeId, uint8 const _nodeId)
	Check if the Controller Believes a Node has Failed.
	This is different from thevIsNodeFailed call in that we test the Controllers
	Failed Node List, whereasvthe IsNodeFailed is testing our list of Failed Nodes,
	which might be different. The Results will be communicated via Notifications.
	Specifically, you should monitor the ControllerCommand notifications.
	*/
	// =================================================================
	NAN_METHOD(OZW::HasNodeFailed)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->HasNodeFailed(homeid, nodeid)
		));
	}

	/* bool 	RequestNodeNeighborUpdate (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to update its Neighbor Tables
	This command will ask a Node to update its Neighbor Tables.
	*/
	// =================================================================
	NAN_METHOD(OZW::RequestNodeNeighborUpdate)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RequestNodeNeighborUpdate(homeid, nodeid)
		));
	}

	/*
	bool 	AssignReturnRoute (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to update its update its Return Route to the Controller
	This command will ask a Node to update its Return Route to the Controller.
	*/
	// =================================================================
	NAN_METHOD(OZW::AssignReturnRoute)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->AssignReturnRoute(homeid, nodeid)
		));
	}

	/* bool 	DeleteAllReturnRoutes (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to delete all Return Route.
	This command will ask a Node to delete all its return routes, and will
	rediscover when needed.
	*/
	// =================================================================
	NAN_METHOD(OZW::DeleteAllReturnRoutes)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->DeleteAllReturnRoutes(homeid, nodeid)
		));
	}

	/*bool 	SendNodeInformation (uint32 const _homeId, uint8 const _nodeId)
	Send a NIF frame from the Controller to a Node.
	This command send a NIF frame from the Controller to a Node.
	*/
	// =================================================================
	NAN_METHOD(OZW::SendNodeInformation)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->SendNodeInformation(homeid, nodeid)
		));
	}

	/*bool 	CreateNewPrimary (uint32 const _homeId)
	Create a new primary controller when old primary fails. Requires SUC.
	This command Creates a new Primary Controller when the Old Primary has Failed.
	Requires a SUC on the network to function.
	*/
	// =================================================================
	NAN_METHOD(OZW::CreateNewPrimary)
	// =================================================================
	{
		Nan::HandleScope scope;
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->CreateNewPrimary(homeid)
		));
	}

	/*bool 	ReceiveConfiguration (uint32 const _homeId)
	Receive network configuration information from primary controller. Requires secondary.
	This command prepares the controller to recieve Network Configuration from a Secondary Controller.
	*/
	// =================================================================
	NAN_METHOD(OZW::ReceiveConfiguration)
	// =================================================================
	{
		Nan::HandleScope scope;
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReceiveConfiguration(homeid)
		));
	}

	/*bool 	ReplaceFailedNode (uint32 const _homeId, uint8 const _nodeId)
	Replace a failed device with another.
	If the node is not in the controller's failed nodes list, or the node responds,
	this command will fail. You can check if a Node is in the Controllers Failed
	node list by using the HasNodeFailed method.
	*/
	// =================================================================
	NAN_METHOD(OZW::ReplaceFailedNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReplaceFailedNode(homeid, nodeid)
		));
	}

	/*bool 	TransferPrimaryRole (uint32 const _homeId)
	Add a new controller to the network and make it the primary.
	The existing primary will become a secondary controller.
	*/
	// =================================================================
	NAN_METHOD(OZW::TransferPrimaryRole)
	// =================================================================
	{
		Nan::HandleScope scope;
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->TransferPrimaryRole(homeid)
		));
	}

	/*bool 	RequestNetworkUpdate (uint32 const _homeId, uint8 const _nodeId)
	Update the controller with network information from the SUC/SIS.
	*/
	// =================================================================
	NAN_METHOD(OZW::RequestNetworkUpdate)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RequestNetworkUpdate(homeid, nodeid)
		));
	}

	/* bool 	ReplicationSend (uint32 const _homeId, uint8 const _nodeId)
	Send information from primary to secondary.
	*/
	// =================================================================
	NAN_METHOD(OZW::ReplicationSend)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReplicationSend(homeid, nodeid)
		));
	}

	/* bool 	CreateButton (uint32 const _homeId, uint8 const _nodeId, uint8 const _buttonid)
	Create a handheld button id.
	*/
	// =================================================================
	NAN_METHOD(OZW::CreateButton)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		uint8 btnid = info[1]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->CreateButton(homeid, nodeid, btnid)
		));
	}

	/* bool 	DeleteButton (uint32 const _homeId, uint8 const _nodeId, uint8 const _buttonid)
	Delete a handheld button id.
	*/
	// =================================================================
	NAN_METHOD(OZW::DeleteButton)
	// =================================================================
	{
		Nan::HandleScope scope;
		uint8 nodeid = info[0]->ToNumber()->Value();
		uint8 btnid = info[1]->ToNumber()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->DeleteButton(homeid, nodeid, btnid)
		));
	}

	#else

	/* ------------------------------------
	// LEGACY MODE (using BeginControllerCommand)
	---------------------------------------*/
	// ===================================================================
	NAN_METHOD(OZW::BeginControllerCommand)
	// ===================================================================
	{
		Nan::HandleScope scope;

		std::string ctrcmd = (*String::Utf8Value(info[0]->ToString()));
		uint8    nodeid1 = 0xff;
		uint8    nodeid2 = 0;
		bool highpower = false;
		if (info.Length() > 1) {
			highpower = info[1]->ToBoolean()->Value();
			if (info.Length() > 2) {
				nodeid1 = info[2]->ToNumber()->Value();
				if (info.Length() > 3) {
					nodeid2 = info[3]->ToNumber()->Value();
				}
			}
		}
		//
		CommandMap::const_iterator search = (*ctrlCmdNames).find(ctrcmd);
		if(search != (*ctrlCmdNames).end()) {
			/*
			* BeginControllerCommand
			* http://openzwave.com/dev/classOpenZWave_1_1Manager.html#aa11faf40f19f0cda202d2353a60dbf7b
			*
			_homeId		The Home ID of the Z-Wave controller.
			_command	The command to be sent to the controller.
			_callback	pointer to a function that will be called at various stages during the command process to notify the user of progress or to request actions on the user's part. Defaults to NULL.
			_context	pointer to user defined data that will be passed into to the callback function. Defaults to NULL.
			_highPower	used only with the AddDevice, AddController, RemoveDevice and RemoveController commands. Usually when adding or removing devices, the controller operates at low power so that the controller must be physically close to the device for security reasons. If _highPower is true, the controller will operate at normal power levels instead. Defaults to false.
			_nodeId	is the node ID used by the command if necessary.
			_arg	is an optional argument, usually another node ID, that is used by the command.
			* */
			OpenZWave::Manager::Get()->BeginControllerCommand (
				homeid,
				search->second, // _command
				ozw_ctrlcmd_callback, // _callback
				NULL, 	// void * 	_context = NULL,
				highpower,	// bool 	_highPower = false,
				nodeid1,// uint8 	_nodeId = 0xff,
				nodeid2	// uint8 	_arg = 0
			);
		}
	}

	#endif


	// ===================================================================
	NAN_METHOD(OZW::CancelControllerCommand)
	// ===================================================================
	{
		Nan::HandleScope scope;

		OpenZWave::Manager::Get()->CancelControllerCommand (homeid);
	}

}
