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
	NAN_METHOD(GetNodeNeighbors)
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

		NanReturnValue( o_neighbors );		
	}
	
	// ===================================================================
	NAN_METHOD(SwitchAllOn)
	// ===================================================================
	{
		NanScope();

		OpenZWave::Manager::Get()->SwitchAllOn(homeid);

		NanReturnUndefined();
	}

	// ===================================================================
	NAN_METHOD(SwitchAllOff)
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
	NAN_METHOD(SetLocation)
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
	NAN_METHOD(SetName)
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
	NAN_METHOD(RefreshNodeInfo)
	// ===================================================================
	{
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		
		OpenZWave::Manager::Get()->RefreshNodeInfo(homeid, nodeid);
		
		NanReturnUndefined();
	}
	
}
