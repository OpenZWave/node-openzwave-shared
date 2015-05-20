/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2013 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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
	Handle<Value> OZW::SetConfigParam(const Arguments& args)
	{
		HandleScope scope;

		uint32_t homeid = args[0]->ToNumber()->Value();
		uint8_t nodeid = args[1]->ToNumber()->Value();
		uint8_t param = args[2]->ToNumber()->Value();
		int32_t value = args[3]->ToNumber()->Value();

		if (args.Length() < 5) {
			OpenZWave::Manager::Get()->SetConfigParam(homeid, nodeid, param, value);
		}
		else {
			uint8_t size = args[4]->ToNumber()->Value();
			OpenZWave::Manager::Get()->SetConfigParam(homeid, nodeid, param, value, size);
		}

		return scope.Close(Undefined());
	}
}
