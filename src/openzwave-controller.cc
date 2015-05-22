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
using namespace  ::OpenZWave;

static void controllerCommandCallback (
	Driver::ControllerState _state, 
	Driver::ControllerError _err, 
	void *_context )
{
	//return(NULL);
}

namespace OZW {
	// ===================================================================
	Handle<v8::Value> OZW::BeginControllerCommand(const Arguments& args) {
	// ===================================================================
		
		HandleScope scope;

		std::string ctrcmd = (*String::Utf8Value(args[0]->ToString()));
		uint8_t    nodeid1 = args[1]->ToNumber()->Value();
		uint8_t    nodeid2 = args[2]->ToNumber()->Value();
		
		CommandMap::const_iterator search = (*ctrlCmdNames).find(ctrcmd);
		if(search != (*ctrlCmdNames).end()) {
			
			/*
			 * BeginControllerCommand
			 * http://openzwave.com/dev/classOpenZWave_1_1Manager.html#aa11faf40f19f0cda202d2353a60dbf7b
			 * 
			_homeId	The Home ID of the Z-Wave controller.
			_command	The command to be sent to the controller.
			_callback	pointer to a function that will be called at various stages during the command process to notify the user of progress or to request actions on the user's part. Defaults to NULL.
			_context	pointer to user defined data that will be passed into to the callback function. Defaults to NULL.
			_highPower	used only with the AddDevice, AddController, RemoveDevice and RemoveController commands. Usually when adding or removing devices, the controller operates at low power so that the controller must be physically close to the device for security reasons. If _highPower is true, the controller will operate at normal power levels instead. Defaults to false.
			_nodeId	is the node ID used by the command if necessary.
			_arg	is an optional argument, usually another node ID, that is used by the command.
			* */
			OpenZWave::Manager::Get()->BeginControllerCommand (
				homeid,
				search->second,
				&::controllerCommandCallback,
				NULL, 	// void * 	_context = NULL,
				true,	// bool 	_highPower = false,
				nodeid1,// uint8 	_nodeId = 0xff,
				nodeid2	// uint8 	_arg = 0 
			);
		}
		return scope.Close(Undefined());	
	}
	
	// ===================================================================
	Handle<v8::Value> OZW::CancelControllerCommand(const Arguments& args) {
	// ===================================================================
		HandleScope scope;
		OpenZWave::Manager::Get()->CancelControllerCommand	(homeid);
		return scope.Close(Undefined());
	}
	

	
}
