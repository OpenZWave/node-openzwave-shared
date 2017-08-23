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

namespace OZW {
  // ===================================================================
	NAN_METHOD(OZWDriver::New)
	// ===================================================================
	{
		Nan::HandleScope scope;
		assert(info.IsConstructCall());
		assert(info.Length() > 0);
		uint32 homeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::cout << "Initialising Driver for " << homeid << "\n";
		OZWDriver* self = new OZWDriver();
		std::cout<< *String::Utf8Value(info.This()->GetConstructorName()) << "\n";
		std::cout << "internalFieldCount=" << info.This()->InternalFieldCount() << "\n";
		self->Wrap(info.This());
		self->homeid = homeid;
		info.GetReturnValue().Set(info.This());
	}

	#if OPENZWAVE_SECURITY == 1
	/* -------------------------------------------
	// OpenZWave >= 1.3: Enable Security Functions
	// and deprecate BeginControllerCommand
	----------------------------------------------*/

	/* bool AddNode (uint32 const _homeId, bool _doSecurity=true)
	* Start the Inclusion Process to add a Node to the Network.
	* The Status of the Node Inclusion is communicated via Notifications.
	* Specifically, you should monitor ControllerCommand Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::AddNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		bool doSecurity = info.Length() > 0 && Nan::To<Boolean>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->AddNode(self->homeid, doSecurity)
		));
	}

	/*bool RemoveNode (uint32 const _homeId)
	* Remove a Device from the Z-Wave Network
	* The Status of the Node Removal is communicated via Notifications.
	* Specifically, you should monitor ControllerCommand Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::RemoveNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RemoveNode(self->homeid)
		));
	}

	/*bool RemoveFailedNode (uint32 const _homeId, uint8 const _nodeId)
	Remove a Failed Device from the Z-Wave Network
	This Command will remove a failed node from the network. The Node should be on
	the Controllers Failed Node List, otherwise this command will fail. You can
	use the HasNodeFailed function below to test if the Controller believes the
	Node has Failed. The Status of the Node Removal is communicated via
	Notifications. Specifically, you should monitor ControllerCommand
	Notifications.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::RemoveFailedNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RemoveFailedNode(self->homeid, nodeid)
		));
	}

	/* bool HasNodeFailed (uint32 const _homeId, uint8 const _nodeId)
	Check if the Controller Believes a Node has Failed.
	This is different from thevIsNodeFailed call in that we test the Controllers
	Failed Node List, whereasvthe IsNodeFailed is testing our list of Failed Nodes,
	which might be different. The Results will be communicated via Notifications.
	Specifically, you should monitor the ControllerCommand notifications.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::HasNodeFailed)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->HasNodeFailed(self->homeid, nodeid)
		));
	}

	/* bool RequestNodeNeighborUpdate (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to update its Neighbor Tables
	This command will ask a Node to update its Neighbor Tables.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::RequestNodeNeighborUpdate)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RequestNodeNeighborUpdate(self->homeid, nodeid)
		));
	}

	/* bool AssignReturnRoute (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to update its update its Return Route to the Controller
	This command will ask a Node to update its Return Route to the Controller.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::AssignReturnRoute)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->AssignReturnRoute(self->homeid, nodeid)
		));
	}

	/* bool DeleteAllReturnRoutes (uint32 const _homeId, uint8 const _nodeId)
	Ask a Node to delete all Return Route.
	This command will ask a Node to delete all its return routes, and will
	rediscover when needed.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::DeleteAllReturnRoutes)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->DeleteAllReturnRoutes(self->homeid, nodeid)
		));
	}

	/* bool SendNodeInformation (uint32 const _homeId, uint8 const _nodeId)
	Send a NIF frame from the Controller to a Node.
	This command send a NIF frame from the Controller to a Node.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::SendNodeInformation)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->SendNodeInformation(self->homeid, nodeid)
		));
	}

	/*bool CreateNewPrimary (uint32 const _homeId)
	Create a new primary controller when old primary fails. Requires SUC.
	This command Creates a new Primary Controller when the Old Primary has Failed.
	Requires a SUC on the network to function.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::CreateNewPrimary)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->CreateNewPrimary(self->homeid)
		));
	}

	/* bool ReceiveConfiguration (uint32 const _homeId)
	Receive network configuration information from primary controller. Requires
	secondary. This command prepares the controller to recieve Network
	Configuration from a Secondary Controller.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::ReceiveConfiguration)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReceiveConfiguration(self->homeid)
		));
	}

	/*bool ReplaceFailedNode (uint32 const _homeId, uint8 const _nodeId)
	Replace a failed device with another.
	If the node is not in the controller's failed nodes list, or the node responds,
	this command will fail. You can check if a Node is in the Controllers Failed
	node list by using the HasNodeFailed method.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::ReplaceFailedNode)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReplaceFailedNode(self->homeid, nodeid)
		));
	}

	/* bool TransferPrimaryRole (uint32 const _homeId)
	Add a new controller to the network and make it the primary.
	The existing primary will become a secondary controller.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::TransferPrimaryRole)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->TransferPrimaryRole(self->homeid)
		));
	}

	/* bool 	RequestNetworkUpdate (uint32 const _homeId, uint8 const _nodeId)
	Update the controller with network information from the SUC/SIS.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::RequestNetworkUpdate)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->RequestNetworkUpdate(self->homeid, nodeid)
		));
	}

	/* bool 	ReplicationSend (uint32 const _homeId, uint8 const _nodeId)
	Send information from primary to secondary.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::ReplicationSend)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->ReplicationSend(self->homeid, nodeid)
		));
	}

	/* bool CreateButton (uint32 const _homeId, uint8 const _nodeId, uint8 const _buttonid)
	Create a handheld button id.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::CreateButton)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, buttonid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 btnid = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->CreateButton(self->homeid, nodeid, btnid)
		));
	}

	/* bool DeleteButton (uint32 const _homeId, uint8 const _nodeId, uint8 const _buttonid)
	Delete a handheld button id.
	*/
	// =================================================================
	NAN_METHOD(OZWDriver::DeleteButton)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, buttonid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 btnid = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		info.GetReturnValue().Set(Nan::New<Boolean>(
			OpenZWave::Manager::Get()->DeleteButton(self->homeid, nodeid, btnid)
		));
	}

	#else

	/* ------------------------------------
	// LEGACY MODE (using BeginControllerCommand)
	---------------------------------------*/
	// ===================================================================
	NAN_METHOD(OZWDriver::BeginControllerCommand)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "command");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		std::string ctrcmd = (*String::Utf8Value(Nan::To<String>(info[0]).ToLocalChecked()));
		uint8 nodeid1 = 0xff;
		uint8 nodeid2 = 0;
		bool highpower = false;
		if (info.Length() > 1) {
			highpower = Nan::To<Boolean>(info[1]).ToLocalChecked()->Value();
			if (info.Length() > 2) {
				nodeid1 = Nan::To<Number>(info[2]).ToLocalChecked()->Value();
				if (info.Length() > 3) {
					nodeid2 = Nan::To<Number>(info[3]).ToLocalChecked()->Value();
				}
			}
		}
		//
		CommandMap::const_iterator search = (*ctrlCmdNames).find(ctrcmd);
		if (search != (*ctrlCmdNames).end()) {
			/*
			* BeginControllerCommand
			* http://openzwave.com/dev/classOpenZWave_1_1Manager.html#aa11faf40f19f0cda202d2353a60dbf7b
			*
			_homeId		The Home ID of the Z-Wave controller.
			_command	The command to be sent to the controller.
			_callback	pointer to a function that will be called at various stages
			during the command process to notify the user of progress or to request
			actions on the user's part. Defaults to NULL.
			_context	pointer to user defined data that will be passed into to the
			callback function. Defaults to NULL.
			_highPower	used only with the AddDevice, AddController, RemoveDevice and
			RemoveController commands. Usually when adding or removing devices, the
			controller operates at low power so that the controller must be physically
			close to the device for security reasons. If _highPower is true, the
			controller will operate at normal power levels instead. Defaults to false.
			_nodeId	is the node ID used by the command if necessary.
			_arg	is an optional argument, usually another node ID, that is used
			by the command.
			* */
			OpenZWave::Manager::Get()->BeginControllerCommand(
				self->homeid,
				search->second,       // _command
				ozw_ctrlcmd_callback, // _callback
				NULL,                 // void * 	_context = NULL,
				highpower,            // bool 	_highPower = false,
				nodeid1,              // uint8 	_nodeId = 0xff,
				nodeid2               // uint8 	_arg = 0
				);
		}
	}

	#endif

	// ===================================================================
	NAN_METHOD(OZWDriver::CancelControllerCommand)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->CancelControllerCommand(self->homeid);
	}

	// =================================================================
	NAN_METHOD(OZWDriver::WriteConfig)
	// =================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->WriteConfig(self->homeid);
	}

//

	// =================================================================
	NAN_METHOD(OZWDriver::GetDriverStatistics)
	// =================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::Driver::DriverData data;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->GetDriverStatistics(self->homeid, &data);
		Local <Object> stats = Nan::New<Object>();
		AddIntegerProp(stats, SOFCnt, data.m_SOFCnt);
		AddIntegerProp(stats, ACKWaiting, data.m_ACKWaiting);
		AddIntegerProp(stats, readAborts, data.m_readAborts);
		AddIntegerProp(stats, badChecksum, data.m_badChecksum);
		AddIntegerProp(stats, readCnt, data.m_readCnt);
		AddIntegerProp(stats, writeCnt, data.m_writeCnt);
		AddIntegerProp(stats, CANCnt, data.m_CANCnt);
		AddIntegerProp(stats, NAKCnt, data.m_NAKCnt);
		AddIntegerProp(stats, ACKCnt, data.m_ACKCnt);
		AddIntegerProp(stats, OOFCnt, data.m_OOFCnt);
		AddIntegerProp(stats, dropped, data.m_dropped);
		AddIntegerProp(stats, retries, data.m_retries);
		AddIntegerProp(stats, callbacks, data.m_callbacks);
		AddIntegerProp(stats, badroutes, data.m_badroutes);
		//
		info.GetReturnValue().Set(stats);
	}

	// =================================================================
	NAN_METHOD(OZWDriver::GetNodeStatistics)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Node::NodeData data;
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();

		OpenZWave::Manager::Get()->GetNodeStatistics(self->homeid, nodeid, &data);

		Local <Object> stats = Nan::New<Object>();
		AddIntegerProp(stats, sentCnt, data.m_sentCnt);
		AddIntegerProp(stats, sentFailed, data.m_sentFailed);
		AddIntegerProp(stats, retries, data.m_retries);
		AddIntegerProp(stats, receivedCnt, data.m_receivedCnt);
		AddIntegerProp(stats, receivedDups, data.m_receivedDups);
		AddIntegerProp(stats, receivedUnsolicited, data.m_receivedUnsolicited);
		AddIntegerProp(stats, lastRequestRTT, data.m_lastRequestRTT);
		AddIntegerProp(stats, lastResponseRTT, data.m_lastResponseRTT);
		AddIntegerProp(stats, averageRequestRTT, data.m_averageRequestRTT);
		AddIntegerProp(stats, averageResponseRTT, data.m_averageResponseRTT);
		AddIntegerProp(stats, quality, data.m_quality);
		AddStringProp(stats, sentTS, data.m_sentTS);
		AddStringProp(stats, receivedTS, data.m_receivedTS);
		info.GetReturnValue().Set(stats);
	}


	/*
	* Reset the ZWave controller chip.  A hard reset is destructive and wipes
	* out all known configuration, a soft reset just restarts the chip.
	*/
	// ===================================================================
	NAN_METHOD(OZWDriver::HardReset)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->ResetController(self->homeid);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::SoftReset)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->SoftReset(self->homeid);
	}


	// ===================================================================
	NAN_METHOD(OZWDriver::GetControllerNodeId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	uint8 ctrlid = OpenZWave::Manager::Get()->GetControllerNodeId(self->homeid);
	 	info.GetReturnValue().Set(
			Nan::New<Integer>(ctrlid)
		);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::GetSUCNodeId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	uint8 sucid = OpenZWave::Manager::Get()->GetSUCNodeId(self->homeid);
	 	info.GetReturnValue().Set(
			Nan::New<Integer>(sucid)
		);
	}

	/* Query if the controller is a primary controller. The primary controller
	 * is the main device used to configure and control a Z-Wave network.
	 * There can only be one primary controller - all other controllers
	 * are secondary controllers.
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsPrimaryController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	bool isprimary = OpenZWave::Manager::Get()->IsPrimaryController(self->homeid);
	 	info.GetReturnValue().Set(Nan::New<Boolean>(isprimary));
	}

	/* Query if the controller is a static update controller. A Static
	 * Update Controller (SUC) is a controller that must never be moved
	 * in normal operation and which can be used by other nodes to
	 * receive information about network changes.
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsStaticUpdateController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	bool issuc = OpenZWave::Manager::Get()->IsStaticUpdateController(self->homeid);
	 	info.GetReturnValue().Set(Nan::New<Boolean>(issuc));
	}

	/* Query if the controller is using the bridge controller library.
	 * A bridge controller is able to create virtual nodes that can be
	 * associated with other controllers to enable events to be passed on.
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsBridgeController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	bool isbridge = OpenZWave::Manager::Get()->IsBridgeController(self->homeid);
	 	info.GetReturnValue().Set(Nan::New<Boolean>(isbridge));
	}

 	/* Get the version of the Z-Wave API library used by a controller.
 	 */
 	// ===================================================================
	NAN_METHOD(OZWDriver::GetLibraryVersion)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	std::string libver = OpenZWave::Manager::Get()->GetLibraryVersion(self->homeid);
	 	info.GetReturnValue().Set(
			Nan::New<String>(
				libver.c_str()
			).ToLocalChecked()
		);
	}

 	/* Get a string containing the Z-Wave API library type used by a
 	 * controller. The possible library types are:
 	 * 	Static Controller
 	 * 	Controller
 	 * 	Enhanced Slave
 	 * Slave
 	 * Installer
 	 * Routing Slave
 	 * Bridge Controller
 	 * Device Under Test
 	 *
 	 * The controller should never return a slave library type. For a
 	 * more efficient test of whether a controller is a Bridge Controller,
 	 * use the IsBridgeController method.
 	 */
 	// ===================================================================
	NAN_METHOD(OZWDriver::GetLibraryTypeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	std::string libtype = OpenZWave::Manager::Get()->GetLibraryTypeName (self->homeid);
	 	info.GetReturnValue().Set(
			Nan::New<String>(
				libtype.c_str()
			).ToLocalChecked()
		);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::GetSendQueueCount)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
	 	uint32 cnt = OpenZWave::Manager::Get()->GetSendQueueCount (self->homeid);
	 	info.GetReturnValue().Set(Nan::New<Integer>(cnt));
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::ToString)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		std::string s("OpenZWave Driver for home %d", self->homeid);
	 	info.GetReturnValue().Set(Nan::New<String>(s).ToLocalChecked());
	}

}
