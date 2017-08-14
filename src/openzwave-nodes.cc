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
	* Gets the neighbors for a node
	*/
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeNeighbors)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8* neighbors;
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 numNeighbors = OpenZWave::Manager::Get()->GetNodeNeighbors(self->homeid, nodeid, &neighbors);
		Local<Array> o_neighbors = Nan::New<Array>(numNeighbors);
		if (numNeighbors > 0) {
			for (uint8 nr = 0; nr < numNeighbors; nr++) {
				o_neighbors->Set(Nan::New<Integer>(nr), Nan::New<Integer>(neighbors[nr]));
			}
			delete neighbors;
		}
		info.GetReturnValue().Set( o_neighbors );
	}

	// =================================================================
	NAN_METHOD(OZWDriver::SetNodeOn)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->SetNodeOn(self->homeid, nodeid);
	}

	// =================================================================
	NAN_METHOD(OZWDriver::SetNodeOff)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->SetNodeOff(self->homeid, nodeid);
	}

	// Generic dimmer control
	// =================================================================
	NAN_METHOD(OZWDriver::SetNodeLevel)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, level");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Integer>(info[0]).ToLocalChecked()->Value();
		uint8 level  = Nan::To<Integer>(info[1]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->SetNodeLevel(self->homeid, nodeid, level);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::SwitchAllOn)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->SwitchAllOn(self->homeid);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::SwitchAllOff)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		OpenZWave::Manager::Get()->SwitchAllOff(self->homeid);
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::PressButton)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::ValueID* ozwvid = populateValueId(info);
		if (ozwvid) {
			OpenZWave::Manager::Get()->PressButton(*ozwvid);
		}
	}

	// ===================================================================
	NAN_METHOD(OZWDriver::ReleaseButton)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::ValueID* ozwvid = populateValueId(info);
		if (ozwvid) {
			OpenZWave::Manager::Get()->ReleaseButton(*ozwvid);
		}
	}
	/*
	* Write a new location string to the device, if supported.
	*/
	// ===================================================================
	NAN_METHOD(OZWDriver::SetNodeLocation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, location");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string location = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeLocation(self->homeid, nodeid, location);
	}

	/*
	* Write a new name string to the device, if supported.
	*/
	// ===================================================================
	NAN_METHOD(OZWDriver::SetNodeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, name");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeName(self->homeid, nodeid, name);
	}

	/*
	* Trigger the fetching of fixed data about a node. Causes the node's
	* data to be obtained from the Z-Wave network in the same way as if
	* it had just been added. This method would normally be called
	* automatically by OpenZWave, but if you know that a node has been
	* changed, calling this method will force a refresh of all of the
	* data held by the library. This can be especially useful for
	* devices that were asleep when the application was first run.
	* This is the same as the query state starting from the beginning.
	*/
	// ===================================================================
	NAN_METHOD(OZWDriver::RefreshNodeInfo)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->RefreshNodeInfo(self->homeid, nodeid);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeManufacturerName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerName(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::RequestNodeState)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->RequestNodeState(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::RequestNodeDynamic)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->RequestNodeDynamic(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsNodeListeningDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeListeningDevice(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsNodeFrequentListeningDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeFrequentListeningDevice(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsNodeBeamingDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeBeamingDevice(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsNodeRoutingDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeRoutingDevice(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::IsNodeSecurityDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeSecurityDevice(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeMaxBaudRate)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8  nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint32 result = OpenZWave::Manager::Get()->GetNodeMaxBaudRate(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Uint32>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeVersion)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeVersion(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeSecurity)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeSecurity(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeBasic)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeBasic(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeGeneric)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeGeneric(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeSpecific)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeSpecific(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeType)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeType(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeProductName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductName(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeName(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeLocation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeLocation(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeManufacturerId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerId(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeProductType)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductType(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::GetNodeProductId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductId(self->homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::SetNodeManufacturerName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeManufacturerName(self->homeid, nodeid, name);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZWDriver::SetNodeProductName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		OZWDriver* self = ObjectWrap::Unwrap<OZWDriver>(info.This());
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeProductName(self->homeid, nodeid, name);
	}

}
