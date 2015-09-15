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
	/*
	* Gets the neighbors for a node
	*/
	// ===================================================================
	NAN_METHOD(OZW::GetNodeNeighbors)
	// ===================================================================
	{
		NanScope();
		uint8* neighbors; 

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8 numNeighbors = OpenZWave::Manager::Get()->GetNodeNeighbors(homeid, nodeid, &neighbors);
		Local<Array> o_neighbors = NanNew<Array>(numNeighbors);

		for (uint8 nr = 0; nr < numNeighbors; nr++) {
			o_neighbors->Set(NanNew<Integer>(nr), NanNew<Integer>(neighbors[nr]));
		}

		delete[] neighbors;

		NanReturnValue( o_neighbors );		
	}
	
	// =================================================================
	NAN_METHOD(OZW::SetNodeOn)
	// =================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		OpenZWave::Manager::Get()->SetNodeOn(homeid, nodeid);
		
		NanReturnUndefined();
	}
	
	// =================================================================
	NAN_METHOD(OZW::SetNodeOff)
	// =================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		OpenZWave::Manager::Get()->SetNodeOff(homeid, nodeid);
		
		NanReturnUndefined();
	}
	
	// ===================================================================
	NAN_METHOD(OZW::SwitchAllOn)
	// ===================================================================
	{
		NanScope();

		OpenZWave::Manager::Get()->SwitchAllOn(homeid);

		NanReturnUndefined();
	}

	// ===================================================================
	NAN_METHOD(OZW::SwitchAllOff)
	// ===================================================================
	{
		NanScope();

		OpenZWave::Manager::Get()->SwitchAllOff(homeid);

		NanReturnUndefined();
	}

	/*
	* Write a new location string to the device, if supported.
	*/
	// ===================================================================
	NAN_METHOD(OZW::SetLocation)
	// ===================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		std::string location = (*String::Utf8Value(args[1]->ToString()));

		OpenZWave::Manager::Get()->SetNodeLocation(homeid, nodeid, location);

		NanReturnUndefined();
	}

	/*
	* Write a new name string to the device, if supported.
	*/
	// ===================================================================
	NAN_METHOD(OZW::SetName)
	// ===================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		std::string name = (*String::Utf8Value(args[1]->ToString()));

		OpenZWave::Manager::Get()->SetNodeName(homeid, nodeid, name);

		NanReturnUndefined();
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
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		
		OpenZWave::Manager::Get()->RefreshNodeInfo(homeid, nodeid);
		
		NanReturnUndefined();
	}

	NAN_METHOD(OZW::GetNodeManufacturerName)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerName(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}
	
	NAN_METHOD(OZW::RequestNodeState)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->RequestNodeState(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::RequestNodeDynamic)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->RequestNodeDynamic(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::IsNodeListeningDevice)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->IsNodeListeningDevice(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::IsNodeFrequentListeningDevice)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->IsNodeFrequentListeningDevice(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::IsNodeBeamingDevice)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->IsNodeBeamingDevice(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::IsNodeRoutingDevice)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->IsNodeRoutingDevice(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::IsNodeSecurityDevice)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		bool result = OpenZWave::Manager::Get()->IsNodeSecurityDevice(homeid, nodeid);

		NanReturnValue(NanNew<Boolean>(result));
	}

	NAN_METHOD(OZW::GetNodeMaxBaudRate)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint32_t result = OpenZWave::Manager::Get()->GetNodeMaxBaudRate(homeid, nodeid);

		NanReturnValue(NanNew<Uint32>(result));
	}

	NAN_METHOD(OZW::GetNodeVersion)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint8_t result = OpenZWave::Manager::Get()->GetNodeVersion(homeid, nodeid);

		NanReturnValue(NanNew<Integer>(result));
	} 

	NAN_METHOD(OZW::GetNodeSecurity)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint8_t result = OpenZWave::Manager::Get()->GetNodeSecurity(homeid, nodeid);

		NanReturnValue(NanNew<Integer>(result));
	} 

	NAN_METHOD(OZW::GetNodeBasic)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint8_t result = OpenZWave::Manager::Get()->GetNodeBasic(homeid, nodeid);

		NanReturnValue(NanNew<Integer>(result));
	} 

	NAN_METHOD(OZW::GetNodeGeneric)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint8_t result = OpenZWave::Manager::Get()->GetNodeGeneric(homeid, nodeid);

		NanReturnValue(NanNew<Integer>(result));
	} 

	NAN_METHOD(OZW::GetNodeSpecific)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		uint8_t result = OpenZWave::Manager::Get()->GetNodeSpecific(homeid, nodeid);

		NanReturnValue(NanNew<Integer>(result));
	}

	NAN_METHOD(OZW::GetNodeType)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeType(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeProductName)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeProductName(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeName)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeName(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeLocation)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeLocation(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeManufacturerId)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeManufacturerId(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeProductType)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeProductType(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::GetNodeProductId)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();

		std::string result = OpenZWave::Manager::Get()->GetNodeProductId(homeid, nodeid);

		NanReturnValue(NanNew<String>(result.c_str()));
	}

	NAN_METHOD(OZW::SetNodeManufacturerName)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		std::string name = (*String::Utf8Value(args[1]->ToString()));

		OpenZWave::Manager::Get()->SetNodeManufacturerName(homeid, nodeid, name);

		NanReturnUndefined();
	}

	NAN_METHOD(OZW::SetNodeProductName)
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		std::string name = (*String::Utf8Value(args[1]->ToString()));

		OpenZWave::Manager::Get()->SetNodeProductName(homeid, nodeid, name);

		NanReturnUndefined();
	}
	
}
