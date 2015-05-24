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
	
	// ===================================================================
	Handle<v8::Value> OZW::Connect(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		uv_async_init(uv_default_loop(), &async, async_cb_handler);

		context_obj = Persistent < Object > ::New(args.This());

		OpenZWave::Manager::Create();
		OpenZWave::Manager::Get()->AddWatcher(ozw_watcher_callback, NULL);
		OpenZWave::Manager::Get()->AddDriver(path);

		Handle<v8::Value> argv[1] = { String::New("connected") };
		MakeCallback(context_obj, "emit", 1, argv);

		return Undefined();
	}

	// ===================================================================
	Handle<v8::Value> OZW::Disconnect(const Arguments& args)
	// ===================================================================
	{
		HandleScope scope;

		std::string path = (*String::Utf8Value(args[0]->ToString()));

		OpenZWave::Manager::Get()->RemoveDriver(path);
		OpenZWave::Manager::Get()->RemoveWatcher(ozw_watcher_callback, NULL);
		OpenZWave::Manager::Destroy();
		OpenZWave::Options::Destroy();

		return scope.Close(Undefined());
	}

}
