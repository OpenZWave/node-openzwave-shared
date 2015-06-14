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
	* Generic value set.
	*/
	// =================================================================
	NAN_METHOD(OZW::SetValue)
	// =================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t comclass = args[1]->ToNumber()->Value();
		uint8_t instance = args[2]->ToNumber()->Value();
		uint8_t index = args[3]->ToNumber()->Value();

		NodeInfo *node;
		std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if (((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {

					switch ((*vit).GetType()) {
						case OpenZWave::ValueID::ValueType_Bool: {
							bool val = args[4]->ToBoolean()->Value();
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Byte: {
							uint8_t val = args[4]->ToInteger()->Value();
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Decimal: {
							float val = args[4]->ToNumber()->NumberValue();
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Int: {
							int32_t val = args[4]->ToInteger()->Value();
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_List: {
							std::string val = (*String::Utf8Value(args[4]->ToString()));
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Short: {
							int16_t val = args[4]->ToInteger()->Value();
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_String: {
							std::string val = (*String::Utf8Value(args[4]->ToString()));
							OpenZWave::Manager::Get()->SetValue(*vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Schedule: {
							break;
						}
						case OpenZWave::ValueID::ValueType_Button: {
							break;
						}
						case OpenZWave::ValueID::ValueType_Raw: {
							break;
						}
					}
				}
			}
		}

		NanReturnUndefined();
	}
}
