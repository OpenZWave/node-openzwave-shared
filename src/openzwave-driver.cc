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
	
	// ===================================================================
	NAN_METHOD(Connect)
	// ===================================================================
	{
		NanScope();

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		uv_async_init(uv_default_loop(), &async, async_cb_handler);

		context_obj = Persistent < Object > ::New(args.This());

		OpenZWave::Manager::Create();
		OpenZWave::Manager::Get()->AddWatcher(ozw_watcher_callback, NULL);
		OpenZWave::Manager::Get()->AddDriver(path);

		Handle<v8::Value> argv[1] = { NanNew<String>("connected") };
		MakeCallback(context_obj, "emit", 1, argv);

		NanReturnUndefined();
	}

	// ===================================================================
	NAN_METHOD(Disconnect)
	// ===================================================================
	{
		NanScope();

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		OpenZWave::Manager::Get()->RemoveDriver(path);
		OpenZWave::Manager::Get()->RemoveWatcher(ozw_watcher_callback, NULL);
		OpenZWave::Manager::Destroy();
		OpenZWave::Options::Destroy();

		NanReturnUndefined();
	}

	/*
	* Reset the ZWave controller chip.  A hard reset is destructive and wipes
	* out all known configuration, a soft reset just restarts the chip.
	*/
	// ===================================================================
	NAN_METHOD(HardReset)
	// ===================================================================
	{
		NanScope();

		OpenZWave::Manager::Get()->ResetController(homeid);

		NanReturnUndefined();
	}
	
	// ===================================================================
	NAN_METHOD(SoftReset)
	// ===================================================================
	{
		NanScope();

		OpenZWave::Manager::Get()->SoftReset(homeid);

		NanReturnUndefined();
	}
	
	// ===================================================================
	NAN_METHOD(BeginControllerCommand)
	// ===================================================================
	{
		NanScope();
		
		std::string ctrcmd = (*String::Utf8Value(args[0]->ToString()));
		uint8_t    nodeid1 = 0xff;
		uint8_t    nodeid2 = 0;
		bool highpower = false;
		if (args.Length() > 1) {
			highpower = args[1]->ToBoolean()->Value();
			if (args.Length() > 2) {
				nodeid1 = args[2]->ToNumber()->Value();
				if (args.Length() > 3) {
					nodeid2 = args[3]->ToNumber()->Value();
				}
			}
		}
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
		NanReturnUndefined();
	}
	
	// ===================================================================
	NAN_METHOD(CancelControllerCommand)
	// ===================================================================
	{
		NanScope();
		OpenZWave::Manager::Get()->CancelControllerCommand (homeid);
		NanReturnUndefined();
	}
	
	// ===================================================================
	NAN_METHOD(GetControllerNodeId)
	// ===================================================================
	{
		NanScope();
	 	uint8 ctrlid = OpenZWave::Manager::Get()->GetControllerNodeId (homeid);
	 	NanReturnValue(Integer::New(ctrlid));
	}

	// ===================================================================
	NAN_METHOD(GetSUCNodeId)
	// ===================================================================
	{
		NanScope();
	 	uint8 sucid = OpenZWave::Manager::Get()->GetSUCNodeId (homeid);
	 	NanReturnValue(Integer::New(sucid));
	}
	 
	/* Query if the controller is a primary controller. The primary controller 
	 * is the main device used to configure and control a Z-Wave network. 
	 * There can only be one primary controller - all other controllers 
	 * are secondary controllers. 
	 */
	// ===================================================================
	NAN_METHOD(IsPrimaryController)
	// ===================================================================
	{
		NanScope();
	 	bool isprimary = OpenZWave::Manager::Get()->IsPrimaryController (homeid);
	 	NanReturnValue(Boolean::New(isprimary));
	}
 
	/* Query if the controller is a static update controller. A Static 
	 * Update Controller (SUC) is a controller that must never be moved 
	 * in normal operation and which can be used by other nodes to 
	 * receive information about network changes. 
	 */
	// ===================================================================
	NAN_METHOD(IsStaticUpdateController)
	// ===================================================================
	{
		NanScope();
	 	bool issuc = OpenZWave::Manager::Get()->IsStaticUpdateController (homeid);
	 	NanReturnValue(Boolean::New(issuc));
	}
 
	/* Query if the controller is using the bridge controller library. 
	 * A bridge controller is able to create virtual nodes that can be 
	 * associated with other controllers to enable events to be passed on. 
	 */ 
	// ===================================================================
	NAN_METHOD(IsBridgeController)
	// ===================================================================
	{
		NanScope();
	 	bool isbridge = OpenZWave::Manager::Get()->IsBridgeController (homeid);
	 	NanReturnValue(Boolean::New(isbridge));
	}

 	/* Get the version of the Z-Wave API library used by a controller. 
 	 */
 	// ===================================================================
	NAN_METHOD(GetLibraryVersion)
	// ===================================================================
	{
		NanScope();
	 	std::string libver = OpenZWave::Manager::Get()->GetLibraryVersion (homeid);
	 	NanReturnValue(NanNew<String>(libver.c_str()));
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
	NAN_METHOD(GetLibraryTypeName)
	// ===================================================================
	{
		NanScope();
	 	std::string libtype = OpenZWave::Manager::Get()->GetLibraryTypeName (homeid);
	 	NanReturnValue(NanNew<String>(libtype.c_str()));
	}

	// ===================================================================
	NAN_METHOD(GetSendQueueCount)
	// ===================================================================
	{
		NanScope();
	 	uint32 cnt = OpenZWave::Manager::Get()->GetSendQueueCount (homeid);
	 	NanReturnValue(Integer::New(cnt));
	}

}
