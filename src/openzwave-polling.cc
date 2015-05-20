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
	* Enable/Disable polling on a COMMAND_CLASS basis.
	*/
	Handle<Value> OZW::EnablePoll(const Arguments& args) {
		HandleScope scope;

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t comclass = args[1]->ToNumber()->Value();
		NodeInfo *node;
		std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if ((*vit).GetCommandClassId() == comclass) {
					OpenZWave::Manager::Get()->EnablePoll((*vit), 1);
					break;
				}
			}
		}

		return scope.Close(Undefined());
	}

	Handle<Value> OZW::DisablePoll(const Arguments& args) {
		HandleScope scope;

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t comclass = args[1]->ToNumber()->Value();
		NodeInfo *node;
		std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if ((*vit).GetCommandClassId() == comclass) {
					OpenZWave::Manager::Get()->DisablePoll((*vit));
					break;
				}
			}
		}

		return scope.Close(Undefined());
	}
}
