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
	CommandMap* ctrlCmdNames;

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
	extern "C" void init(Handle<Object> target) {
		HandleScope scope;

		Local < FunctionTemplate > t = FunctionTemplate::New(OZW::New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		t->SetClassName(String::New("OZW"));
		// openzwave-controller.cc
		NODE_SET_PROTOTYPE_METHOD(t, "beginControllerCommand", OZW::BeginControllerCommand); // ** new
		NODE_SET_PROTOTYPE_METHOD(t, "cancelControllerCommand", OZW::CancelControllerCommand); // ** new
		// openzwave-driver.cc
		NODE_SET_PROTOTYPE_METHOD(t, "connect", OZW::Connect);
		NODE_SET_PROTOTYPE_METHOD(t, "disconnect", OZW::Disconnect);
		// openzwave-network.cc
		NODE_SET_PROTOTYPE_METHOD(t, "testNetworkNode", OZW::TestNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "testNetwork", OZW::TestNetwork);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetworkNode", OZW::HealNetworkNode);
		NODE_SET_PROTOTYPE_METHOD(t, "healNetwork", OZW::HealNetwork);
		// openzwave-nodes.cc
		NODE_SET_PROTOTYPE_METHOD(t, "setLocation", OZW::SetLocation);
		NODE_SET_PROTOTYPE_METHOD(t, "setName", OZW::SetName);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOn", OZW::SwitchAllOn);
		NODE_SET_PROTOTYPE_METHOD(t, "switchAllOff", OZW::SwitchAllOff);
		NODE_SET_PROTOTYPE_METHOD(t, "getNodeNeighbors", OZW::GetNodeNeighbors);
		NODE_SET_PROTOTYPE_METHOD(t, "refreshNodeInfo", OZW::RefreshNodeInfo); // ** new
		// openzwave-values.cc
		NODE_SET_PROTOTYPE_METHOD(t, "setValue", OZW::SetValue);
		// openzwave-polling.cc
		NODE_SET_PROTOTYPE_METHOD(t, "enablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "disablePoll", OZW::EnablePoll);
		NODE_SET_PROTOTYPE_METHOD(t, "setPollInterval",  OZW::SetPollInterval);
		NODE_SET_PROTOTYPE_METHOD(t, "setPollIntensity",  OZW::SetPollIntensity); // ** new
		//
		NODE_SET_PROTOTYPE_METHOD(t, "hardReset", OZW::HardReset);
		NODE_SET_PROTOTYPE_METHOD(t, "softReset", OZW::SoftReset);
		// openzwave-scenes.cc
		NODE_SET_PROTOTYPE_METHOD(t, "createScene", OZW::CreateScene);
		NODE_SET_PROTOTYPE_METHOD(t, "removeScene", OZW::RemoveScene);
		NODE_SET_PROTOTYPE_METHOD(t, "getScenes", OZW::GetScenes);
		NODE_SET_PROTOTYPE_METHOD(t, "addSceneValue", OZW::AddSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "removeSceneValue", OZW::RemoveSceneValue);
		NODE_SET_PROTOTYPE_METHOD(t, "sceneGetValues", OZW::SceneGetValues);
		NODE_SET_PROTOTYPE_METHOD(t, "activateScene", OZW::ActivateScene);
		//
		NODE_SET_PROTOTYPE_METHOD(t, "setConfigParam", OZW::SetConfigParam);
		
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
