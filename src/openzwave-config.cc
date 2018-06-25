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
	/*
	* Set Config Parameters
	*/
	// ===================================================================
	NAN_METHOD(OZW::SetConfigParam)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(3, "nodeid, param, value");
		uint8 nodeid = info[0]->Uint32Value();
		uint8 param  = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		int32 value  = Nan::To<Number>(info[2]).ToLocalChecked()->Value();
		if (info.Length() < 4) {
			OpenZWave::Manager::Get()->SetConfigParam(homeid, nodeid, param, value);
		}
		else {
			uint8 size = Nan::To<Number>(info[3]).ToLocalChecked()->Value();
			OpenZWave::Manager::Get()->SetConfigParam(homeid, nodeid, param, value, size);
		}
	}

	/*
 	 * Request the value of a configurable parameter from a device. Some
 	 * devices have various parameters that can be configured to control
 	 * the device behaviour. These are not reported by the device over
 	 * the Z-Wave network, but can usually be found in the device's user
 	 * manual.
 	 * This method requests the value of a parameter from the
 	 * device, and then returns immediately, without waiting for a
 	 * response. If the parameter index is valid for this device, and
 	 * the device is awake, the value will eventually be reported via a
 	 * ValueChanged notification callback. The ValueID reported in the
 	 * callback will have an index set the same as _param and a command
 	 * class set to the same value as returned by a call to
 	 * Configuration::StaticGetCommandClassId.
	*/
	// ===================================================================
	NAN_METHOD(OZW::RequestConfigParam)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, param");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 param = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->RequestConfigParam(homeid, nodeid, param);
	}


	/*
	 * Request the values of all known configurable parameters from a
	 * device.
	 * */
	// ===================================================================
	NAN_METHOD(OZW::RequestAllConfigParams)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->RequestAllConfigParams (homeid, nodeid);
	}

}
