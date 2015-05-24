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
	* Test network node. Sends a series of messages to a network node 
	* for testing network reliability.
	*/
	// ===================================================================
	Handle<v8::Value> OZW::TestNetworkNode(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t nummsg = args[1]->ToBoolean()->Value();

		OpenZWave::Manager::Get()->TestNetworkNode(homeid, nodeid, nummsg);

		return scope.Close(Undefined());
	}

	/*
	* Test network. Sends a series of messages to every node on the 
	* network for testing network reliability.
	*/
	// ===================================================================
	Handle<v8::Value> OZW::TestNetwork(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;
		
		uint8_t nummsg = args[0]->ToNumber()->Value();
		
		OpenZWave::Manager::Get()->HealNetwork(homeid, nummsg);
		
		return scope.Close(Undefined());
	}
	
	/*
	* Heal network node by requesting the node rediscover their neighbors.
	*/
	// ===================================================================
	Handle<v8::Value> OZW::HealNetworkNode(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t doRR = args[1]->ToBoolean()->Value();

		OpenZWave::Manager::Get()->HealNetworkNode(homeid, nodeid, doRR);

		return scope.Close(Undefined());
	}

	/*
	* Heal network by requesting node's rediscover their neighbors.
	* Sends a ControllerCommand_RequestNodeNeighborUpdate to every node.
	* Can take a while on larger networks.
	*/
	// ===================================================================
	Handle<v8::Value> OZW::HealNetwork(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		bool doRR = true;
		OpenZWave::Manager::Get()->HealNetwork(homeid, doRR);

		return scope.Close(Undefined());
	}
}
