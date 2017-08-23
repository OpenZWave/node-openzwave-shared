/*
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
#include <algorithm>

using namespace v8;
using namespace node;

namespace OZW {

	// ===================================================================
	NAN_METHOD(OZW::Connect)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "path");
		std::string path = (*String::Utf8Value(info[0]->ToString()));
		OpenZWave::Manager::Get()->AddDriver(path);
	}

	// ===================================================================
	NAN_METHOD(OZW::Disconnect)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "path");
		std::string path = (*String::Utf8Value(info[0]->ToString()));

		OpenZWave::Manager::Get()->RemoveDriver(path);
/*
*/
		delete emit_cb;
	}

//namespace OZW
}
