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
	DriverMap*  drivers;

	std::string ozw_userpath;
	std::string ozw_config_path  = stringify( OPENZWAVE_ETC );

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
		//
		Local < FunctionTemplate > t_ozw = Nan::New<FunctionTemplate>(OZW::New);
		t_ozw->SetClassName(Nan::New("OZW").ToLocalChecked());
		t_ozw->InstanceTemplate()->SetInternalFieldCount(1);
		// openzwave-driver.cc
		Nan::SetPrototypeMethod(t_ozw, "connect", OZW::Connect);
		Nan::SetPrototypeMethod(t_ozw, "disconnect", OZW::Disconnect);
		Nan::Set(target, Nan::New<String>("Emitter").ToLocalChecked(), t_ozw->GetFunction());
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
		//
		drivers =  new DriverMap();
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
				} else if (keyname == "ConfigPath") {
					ozw_config_path.assign(argvalstr);
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
		//
		ctx_obj = Nan::Persistent<Object>(info.This());

		// init new libuv loop
		uv_async_init(uv_default_loop(), &async, async_cb_handler);

		Local<Function> callbackHandle = Nan::Get( info.This(),
			Nan::New<String>("emit").ToLocalChecked()
		).ToLocalChecked()
		 .As<Function>();

		emit_cb = new Nan::Callback(callbackHandle);
		std::cout << "Openzwave options create\n";
		//  OPENZWAVE INIT
		::OpenZWave::Options::Create(ozw_config_path, ozw_userpath, option_overrides);
		::OpenZWave::Options::Get()->Lock();
		// create the singleton Manager
		::OpenZWave::Manager::Create();
		// add the watcher callback
		::OpenZWave::Manager::Get()->AddWatcher(ozw_watcher_callback, NULL);
		//
		info.GetReturnValue().Set(info.This());
	}
}
/* TODO: destructor
 * 		OpenZWave::Manager::Get()->RemoveWatcher(ozw_watcher_callback, NULL);
		OpenZWave::Manager::Destroy();
		OpenZWave::Options::Destroy();
 */
NODE_MODULE(openzwave_shared, OZW::init)
