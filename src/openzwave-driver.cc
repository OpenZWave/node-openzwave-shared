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
	NAN_METHOD(OZW::Connect)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "path");

		::std::string path(*Nan::Utf8String( info[0] ));

		uv_async_init(uv_default_loop(), &async, async_cb_handler);

		OZW* self = ObjectWrap::Unwrap<OZW>(info.This());
		::std::string version("");
#if OPENZWAVE_EXCEPTIONS
    try {
#endif
		OpenZWave::Options::Create(self->config_path, self->userpath, self->option_overrides);
		OpenZWave::Options::Get()->Lock();
		OpenZWave::Manager::Create();
		OZWManager(AddWatcher, ozw_watcher_callback, NULL);
		OZWManager(AddDriver, path);
		version = OpenZWave::Manager::getVersionAsString();
#if OPENZWAVE_EXCEPTIONS
	} catch ( OpenZWave::OZWException& e ) {
		char buffer [200];
		snprintf(buffer, 200, "Exception connecting in %s(%d): %s",
			e.GetFile().c_str(), e.GetLine(), e.GetMsg().c_str());
		Nan::ThrowError( buffer );
	}
#endif
		Local < v8::Value > cbinfo[16];
		cbinfo[0] = Nan::New<String>("connected").ToLocalChecked();
		cbinfo[1] = Nan::New<String>(version).ToLocalChecked();

		emit_cb->Call(2, cbinfo, resource);
	}

	// ===================================================================
	NAN_METHOD(OZW::UpdateOptions)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "info");

		OZW* self = ObjectWrap::Unwrap<OZW>(info.This());
		::std::string ozw_userpath;
		::std::string ozw_config_path;

		::std::string option_overrides;
		bool log_initialisation = true;
		// Options are global for all drivers and can only be set once.
		if (info.Length() > 0) {
			Local < Object > opts = Nan::To<Object>(info[0]).ToLocalChecked();
			Local < Array > props = Nan::GetOwnPropertyNames(opts).ToLocalChecked();
			for (unsigned int i = 0; i < props->Length(); ++i) {
				Nan::MaybeLocal<Value> keymaybe =  Nan::Get(props, i);
				if(keymaybe.IsEmpty()) continue;
				Local<Value>  key       = keymaybe.ToLocalChecked();
				::std::string keyname   = *Nan::Utf8String(key);
				Local<Value>  argval    = Nan::Get(opts, key).ToLocalChecked();
				::std::string argvalstr = *Nan::Utf8String(argval);
				// UserPath is directly passed to Manager->Connect()
				// scan for OpenZWave options.xml in the nodeJS module's '/config' subdirectory
				if (keyname == "UserPath") {
					ozw_userpath.assign(argvalstr);
				} else if (keyname == "ConfigPath") {
					ozw_config_path.assign(argvalstr);
				} else if (keyname == "LogInitialisation") {
					log_initialisation = (Nan::To<bool>(argval) == Nan::Just(true));
				} else {
					option_overrides += " --" + keyname + " " + argvalstr;
				}
			}
		}

		// Update configuration data for connect.
		self->config_path = ozw_config_path;
		self->userpath = ozw_userpath;
		self->option_overrides = option_overrides;
		self->log_initialisation = log_initialisation;

	}

	// ===================================================================
	NAN_METHOD(OZW::Disconnect)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "path");
		::std::string path(*Nan::Utf8String( info[0] ));

		OZWManager( RemoveDriver, path );
		OZWManager( RemoveWatcher, ozw_watcher_callback, NULL);
#if OPENZWAVE_EXCEPTIONS
    try {
#endif
		OpenZWave::Manager::Destroy();
		OpenZWave::Options::Destroy();
#if OPENZWAVE_EXCEPTIONS
	} catch ( OpenZWave::OZWException& e ) {
		char buffer [200];
		snprintf(buffer, 200, "Exception disconnecting in %s(%d): %s",
			 e.GetFile().c_str(), e.GetLine(), e.GetMsg().c_str());
		Nan::ThrowError( buffer );
	}
#endif
// FIXME: seems some recent innocuous change in NaN causes the context (ctx_obj) to be freed.
// Therefore, deleting this V8 resource will cause V8 to crash. NOT deleting it could memleak
// when you're reloading the driver.
//
//		delete emit_cb;
//
	}

	/*
	* Reset the ZWave controller chip.  A hard reset is destructive and wipes
	* out all known configuration, a soft reset just restarts the chip.
	*/
	// ===================================================================
	NAN_METHOD(OZW::HardReset)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWManager(ResetController, homeid);
	}

	// ===================================================================
	NAN_METHOD(OZW::SoftReset)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWManager(SoftReset, homeid);
	}


	// ===================================================================
	NAN_METHOD(OZW::GetControllerNodeId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		uint8 ctrlid = -1;
		OZWManagerAssign(ctrlid, GetControllerNodeId, homeid);
		info.GetReturnValue().Set(
			Nan::New<Integer>(ctrlid)
		);
	}

	// ===================================================================
	NAN_METHOD(OZW::GetSUCNodeId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		uint8 sucid = -1;
		OZWManagerAssign(sucid, GetSUCNodeId, homeid);
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
	NAN_METHOD(OZW::IsPrimaryController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		bool isprimary = false;
		OZWManagerAssign(isprimary, IsPrimaryController, homeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(isprimary));
	}

	/* Query if the controller is a static update controller. A Static
	 * Update Controller (SUC) is a controller that must never be moved
	 * in normal operation and which can be used by other nodes to
	 * receive information about network changes.
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsStaticUpdateController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		bool issuc = false;
		OZWManagerAssign(issuc, IsStaticUpdateController, homeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(issuc));
	}

	/* Query if the controller is using the bridge controller library.
	 * A bridge controller is able to create virtual nodes that can be
	 * associated with other controllers to enable events to be passed on.
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsBridgeController)
	// ===================================================================
	{
		Nan::HandleScope scope;
		bool isbridge = false;
		OZWManagerAssign(isbridge, IsBridgeController, homeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(isbridge));
	}

	/* Get the version of the Z-Wave API library used by a controller.
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetLibraryVersion)
	// ===================================================================
	{
		Nan::HandleScope scope;
		::std::string libver("");
		OZWManagerAssign(libver, GetLibraryVersion, homeid);
		info.GetReturnValue().Set(
			Nan::New<String>(
				libver.c_str()
			).ToLocalChecked()
		);
	}

	/* Get a string containing the Z-Wave API library type used by a
	 * controller. The possible library types are:
	 *	Static Controller
	 *	Controller
	 *	Enhanced Slave
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
	NAN_METHOD(OZW::GetLibraryTypeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		::std::string libtype("");
		OZWManagerAssign(libtype, GetLibraryTypeName, homeid);
		info.GetReturnValue().Set(
			Nan::New<String>(
				libtype.c_str()
			).ToLocalChecked()
		);
	}

	// ===================================================================
	NAN_METHOD(OZW::GetSendQueueCount)
	// ===================================================================
	{
		Nan::HandleScope scope;
		uint32 cnt = 0;
		OZWManagerAssign(cnt, GetSendQueueCount, homeid);
		info.GetReturnValue().Set(Nan::New<Integer>(cnt));
	}

}
