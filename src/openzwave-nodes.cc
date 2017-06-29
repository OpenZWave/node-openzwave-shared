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
	NAN_METHOD(OZW::GetNodeNeighbors)
	// ===================================================================
	{
		Nan::HandleScope scope;
		uint8* neighbors;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 numNeighbors = OpenZWave::Manager::Get()->GetNodeNeighbors(homeid, nodeid, &neighbors);
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
	NAN_METHOD(OZW::SetNodeOn)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->SetNodeOn(homeid, nodeid);
	}

	// =================================================================
	NAN_METHOD(OZW::SetNodeOff)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->SetNodeOff(homeid, nodeid);
	}

	// Generic dimmer control
	// =================================================================
	NAN_METHOD(OZW::SetNodeLevel)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, level");
		uint8 nodeid = Nan::To<Integer>(info[0]).ToLocalChecked()->Value();
		uint8 level  = Nan::To<Integer>(info[1]).ToLocalChecked()->Value();
    printf("Setting home %x node %d level %d\n", homeid , nodeid, level);
		OpenZWave::Manager::Get()->SetNodeLevel(homeid, nodeid, level);
	}

	// ===================================================================
	NAN_METHOD(OZW::SwitchAllOn)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::Manager::Get()->SwitchAllOn(homeid);
	}

	// ===================================================================
	NAN_METHOD(OZW::SwitchAllOff)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::Manager::Get()->SwitchAllOff(homeid);
	}

	// ===================================================================
	NAN_METHOD(OZW::PressButton)
	// ===================================================================
	{
		Nan::HandleScope scope;
		OpenZWave::ValueID* ozwvid = populateValueId(info);
		if (ozwvid) {
			OpenZWave::Manager::Get()->PressButton(*ozwvid);
		}
	}

	// ===================================================================
	NAN_METHOD(OZW::ReleaseButton)
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
	NAN_METHOD(OZW::SetNodeLocation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, location");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string location = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeLocation(homeid, nodeid, location);
	}

	/*
	* Write a new name string to the device, if supported.
	*/
	// ===================================================================
	NAN_METHOD(OZW::SetNodeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, name");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeName(homeid, nodeid, name);
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
	NAN_METHOD(OZW::RefreshNodeInfo)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::Manager::Get()->RefreshNodeInfo(homeid, nodeid);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeManufacturerName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerName(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::RequestNodeState)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->RequestNodeState(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::RequestNodeDynamic)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->RequestNodeDynamic(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsNodeListeningDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeListeningDevice(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsNodeFrequentListeningDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeFrequentListeningDevice(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsNodeBeamingDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeBeamingDevice(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsNodeRoutingDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeRoutingDevice(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsNodeSecurityDevice)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		bool result = OpenZWave::Manager::Get()->IsNodeSecurityDevice(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Boolean>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeMaxBaudRate)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8  nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint32 result = OpenZWave::Manager::Get()->GetNodeMaxBaudRate(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Uint32>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeVersion)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeVersion(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeSecurity)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeSecurity(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeBasic)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeBasic(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeGeneric)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeGeneric(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeSpecific)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 result = OpenZWave::Manager::Get()->GetNodeSpecific(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(result));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeType)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeType(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeProductName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductName(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeName(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeLocation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeLocation(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeManufacturerId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerId(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeProductType)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductType(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetNodeProductId)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string result = OpenZWave::Manager::Get()->GetNodeProductId(homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::SetNodeManufacturerName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeManufacturerName(homeid, nodeid, name);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::SetNodeProductName)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		std::string name = (*String::Utf8Value(Nan::To<String>(info[1]).ToLocalChecked()));
		OpenZWave::Manager::Get()->SetNodeProductName(homeid, nodeid, name);
	}

}
