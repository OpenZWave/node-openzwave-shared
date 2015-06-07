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
	* Return the node for this request.
	*/
	NodeInfo *get_node_info(uint8_t nodeid) {
		std::list<NodeInfo *>::iterator it;

		NodeInfo *node;

		for (it = znodes.begin(); it != znodes.end(); ++it) {
			node = *it;
			if (node->nodeid == nodeid)
				return node;
		}

		return NULL;
	}

	SceneInfo *get_scene_info(uint8_t sceneid) {
		std::list<SceneInfo *>::iterator it;

		SceneInfo *scene;

		for (it = zscenes.begin(); it != zscenes.end(); ++it) {
			scene = *it;
			if (scene->sceneid == sceneid)
				return scene;
		}

		return NULL;
	}

	std::string* printAllArgs(const Arguments& args) {
		std::cout << "PRINTING ALL ARGS: ";

		std::string* stringArray = new std::string[args.Length()];

		for (int i = 0; i < args.Length(); i++){
			std::string tempString(*v8::String::Utf8Value(args[i]));
			stringArray[i] = tempString;
			std::cout << tempString << ";";
		}

		return stringArray;
	}

	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value) {
		Local <Object> valobj = Object::New();

		char buffer[15];

		sprintf(buffer, "%d-%d-%d-%d", value.GetNodeId(), value.GetCommandClassId(), value.GetInstance(), value.GetIndex());

		valobj->Set(NanNew<String>("value_id"), NanNew<String>(buffer));

		/*
		* Common value types.
		*/
		valobj->Set(NanNew<String>("id"), Integer::New(value.GetId()));
		valobj->Set(NanNew<String>("node_id"), Integer::New(value.GetNodeId()));
		valobj->Set(NanNew<String>("class_id"), Integer::New(value.GetCommandClassId()));
		valobj->Set(NanNew<String>("type"), NanNew<String>(OpenZWave::Value::GetTypeNameFromEnum(value.GetType())));
		valobj->Set(NanNew<String>("genre"), NanNew<String>(OpenZWave::Value::GetGenreNameFromEnum(value.GetGenre())));
		valobj->Set(NanNew<String>("instance"), Integer::New(value.GetInstance()));
		valobj->Set(NanNew<String>("index"), Integer::New(value.GetIndex()));
		valobj->Set(NanNew<String>("label"), NanNew<String>(OpenZWave::Manager::Get()->GetValueLabel(value).c_str()));
		valobj->Set(NanNew<String>("units"), NanNew<String>(OpenZWave::Manager::Get()->GetValueUnits(value).c_str()));
		valobj->Set(NanNew<String>("read_only"), Boolean::New(OpenZWave::Manager::Get()->IsValueReadOnly(value))->ToBoolean());
		valobj->Set(NanNew<String>("write_only"), Boolean::New(OpenZWave::Manager::Get()->IsValueWriteOnly(value))->ToBoolean());
		valobj->Set(NanNew<String>("is_polled"), Boolean::New(OpenZWave::Manager::Get()->IsValuePolled(value))->ToBoolean());
		// XXX: verify_changes=
		// XXX: poll_intensity=
		valobj->Set(NanNew<String>("min"), Integer::New(OpenZWave::Manager::Get()->GetValueMin(value)));
		valobj->Set(NanNew<String>("max"), Integer::New(OpenZWave::Manager::Get()->GetValueMax(value)));

		/*
		* The value itself is type-specific.
		*/
		switch (value.GetType()) {
			case OpenZWave::ValueID::ValueType_Bool: {
				bool val;
				OpenZWave::Manager::Get()->GetValueAsBool(value, &val);
				valobj->Set(NanNew<String>("value"), Boolean::New(val)->ToBoolean());
				break;
			}
			case OpenZWave::ValueID::ValueType_Byte: {
				uint8_t val;
				OpenZWave::Manager::Get()->GetValueAsByte(value, &val);
				valobj->Set(NanNew<String>("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_Decimal: {
				float val;
				OpenZWave::Manager::Get()->GetValueAsFloat(value, &val);
				valobj->Set(NanNew<String>("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_Int: {
				int32_t val;
				OpenZWave::Manager::Get()->GetValueAsInt(value, &val);
				valobj->Set(NanNew<String>("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_List: {
				std::vector < std::string > items;
				OpenZWave::Manager::Get()->GetValueListItems(value, &items);
				Local < Array > values = Array::New(items.size());
				for (unsigned i = 0; i < items.size(); i++) {
					values->Set(Number::New(i), NanNew<String>(&items[i][0], items[i].size()));
				}
				valobj->Set(NanNew<String>("values"), values);
				std::string val;
				OpenZWave::Manager::Get()->GetValueListSelection(value, &val);
				valobj->Set(NanNew<String>("value"), NanNew<String>(val.c_str()));
				break;
			}
			case OpenZWave::ValueID::ValueType_Short: {
				int16_t val;
				OpenZWave::Manager::Get()->GetValueAsShort(value, &val);
				valobj->Set(NanNew<String>("value"), Integer::New(val));
				break;
			}
			case OpenZWave::ValueID::ValueType_String: {
				std::string val;
				OpenZWave::Manager::Get()->GetValueAsString(value, &val);
				valobj->Set(NanNew<String>("value"), NanNew<String>(val.c_str()));
				break;
			}
			/*
			* Buttons do not have a value.
			*/
			case OpenZWave::ValueID::ValueType_Button: {
				break;
			}
			case OpenZWave::ValueID::ValueType_Schedule: {
				break;
			}
			case OpenZWave::ValueID::ValueType_Raw: {
				break;
			}
			default: {
				fprintf(stderr, "unsupported value type: 0x%x\n", value.GetType());
				break;
			}
		}

		return valobj;
	}

	Local<Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value) {
		Local <Object> valobj = Object::New();

		char buffer[15];

		sprintf(buffer, "%d-%d-%d-%d", value.GetNodeId(), value.GetCommandClassId(), value.GetInstance(), value.GetIndex());

		valobj->Set(NanNew<String>("value_id"), NanNew<String>(buffer));

		/*
		* Common value types.
		*/
		valobj->Set(NanNew<String>("id"), Integer::New(value.GetId()));
		valobj->Set(NanNew<String>("node_id"), Integer::New(value.GetNodeId()));
		valobj->Set(NanNew<String>("class_id"), Integer::New(value.GetCommandClassId()));
		valobj->Set(NanNew<String>("type"), NanNew<String>(OpenZWave::Value::GetTypeNameFromEnum(value.GetType())));
		valobj->Set(NanNew<String>("genre"), NanNew<String>(OpenZWave::Value::GetGenreNameFromEnum(value.GetGenre())));
		valobj->Set(NanNew<String>("instance"), Integer::New(value.GetInstance()));
		valobj->Set(NanNew<String>("index"), Integer::New(value.GetIndex()));
		valobj->Set(NanNew<String>("label"), NanNew<String>(OpenZWave::Manager::Get()->GetValueLabel(value).c_str()));
		valobj->Set(NanNew<String>("units"), NanNew<String>(OpenZWave::Manager::Get()->GetValueUnits(value).c_str()));
		valobj->Set(NanNew<String>("read_only"), Boolean::New(OpenZWave::Manager::Get()->IsValueReadOnly(value))->ToBoolean());
		valobj->Set(NanNew<String>("write_only"), Boolean::New(OpenZWave::Manager::Get()->IsValueWriteOnly(value))->ToBoolean());
		// XXX: verify_changes=
		// XXX: poll_intensity=
		valobj->Set(NanNew<String>("min"), Integer::New(OpenZWave::Manager::Get()->GetValueMin(value)));
		valobj->Set(NanNew<String>("max"), Integer::New(OpenZWave::Manager::Get()->GetValueMax(value)));

		/*
		* The value itself is type-specific.
		*/
		switch (value.GetType()) {
		case OpenZWave::ValueID::ValueType_Bool: {
			bool val;
			OpenZWave::Manager::Get()->SceneGetValueAsBool(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), Boolean::New(val)->ToBoolean());
			break;
		}
		case OpenZWave::ValueID::ValueType_Byte: {
			uint8_t val;
			OpenZWave::Manager::Get()->SceneGetValueAsByte(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), Integer::New(val));
			break;
		}
		case OpenZWave::ValueID::ValueType_Decimal: {
			float val;
			OpenZWave::Manager::Get()->SceneGetValueAsFloat(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), Integer::New(val));
			break;
		}
		case OpenZWave::ValueID::ValueType_Int: {
			int32_t val;
			OpenZWave::Manager::Get()->SceneGetValueAsInt(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), Integer::New(val));
			break;
		}
		case OpenZWave::ValueID::ValueType_List: {
			std::vector < std::string > items;
			OpenZWave::Manager::Get()->GetValueListItems(value, &items);
			Local < Array > values = Array::New(items.size());
			for (unsigned i = 0; i < items.size(); i++) {
				values->Set(Number::New(i), NanNew<String>(&items[i][0], items[i].size()));
			}
			valobj->Set(NanNew<String>("values"), values);
			std::string val;
			OpenZWave::Manager::Get()->SceneGetValueListSelection(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), NanNew<String>(val.c_str()));
			break;
		}
		case OpenZWave::ValueID::ValueType_Short: {
			int16_t val;
			OpenZWave::Manager::Get()->SceneGetValueAsShort(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), Integer::New(val));
			break;
		}
		case OpenZWave::ValueID::ValueType_String: {
			std::string val;
			OpenZWave::Manager::Get()->SceneGetValueAsString(sceneId, value, &val);
			valobj->Set(NanNew<String>("value"), NanNew<String>(val.c_str()));
			break;
		}
		default: {
			fprintf(stderr, "unsupported scene value type: 0x%x\n", value.GetType());
			break;
		}
		}

		return valobj;
	}
}
