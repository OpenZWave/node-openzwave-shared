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
	
	/* OpenZWave scene management functions */
	// ===================================================================
	NAN_METHOD(OZW::CreateScene)
	// ===================================================================
	{
		Nan::HandleScope();

		std::string label = (*String::Utf8Value(info[0]->ToString()));

		SceneInfo *scene;

		uint8 sceneid = OpenZWave::Manager::Get()->CreateScene();

		if (sceneid > 0) {
			OpenZWave::Manager::Get()->SetSceneLabel(sceneid, label);
			scene = new SceneInfo();
			scene->sceneid = sceneid;
			scene->label = label;
			mutex::scoped_lock sl(zscenes_mutex);
			zscenes.push_back(scene);
		}

		info.GetReturnValue().Set(Nan::New< Integer >(sceneid));
	}

	// ===================================================================
	NAN_METHOD(OZW::RemoveScene)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t sceneid = info[0]->ToNumber()->Value();

		SceneInfo *scene;

		if ((scene = get_scene_info(sceneid))) {
			OpenZWave::Manager::Get()->RemoveScene(sceneid);
			mutex::scoped_lock sl(zscenes_mutex);
			zscenes.remove(scene);
		}

		
	}

	// ===================================================================
	NAN_METHOD(OZW::GetScenes)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t numscenes = OpenZWave::Manager::Get()->GetNumScenes();
		SceneInfo *scene;

		if (numscenes != zscenes.size()) {
			{
				mutex::scoped_lock sl(zscenes_mutex);
				zscenes.clear();
			}
			uint8_t *sceneids;
			sceneids = new uint8_t[numscenes];

			OpenZWave::Manager::Get()->GetAllScenes(&sceneids);

			for (unsigned i = 0; i < numscenes; i++) {
				scene = new SceneInfo();
				scene->sceneid = sceneids[i];

				scene->label = OpenZWave::Manager::Get()->GetSceneLabel(sceneids[i]);
				mutex::scoped_lock sl(zscenes_mutex);
				zscenes.push_back(scene);
			}
		}

		Local<Array> scenes = Nan::New<Array>(zscenes.size());
		std::list<SceneInfo *>::iterator it;
		unsigned j = 0;

		for (it = zscenes.begin(); it != zscenes.end(); ++it) {
			scene = *it;

			Local <Object> info = Nan::New<Object>();
			info->Set(
				Nan::New<String>("sceneid").ToLocalChecked(), 
				Nan::New<Integer>(scene->sceneid)
			);
			info->Set(
				Nan::New<String>("label").ToLocalChecked(),   
				Nan::New<String>(scene->label.c_str()).ToLocalChecked()
			);

			scenes->Set(Nan::New<Integer>(j++), info);
		}
		
		info.GetReturnValue().Set(scenes);
	}

	// ===================================================================
	NAN_METHOD(OZW::AddSceneValue)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t sceneid = info[0]->ToNumber()->Value();
		uint8_t nodeid = info[1]->ToNumber()->Value();
		uint8_t comclass = info[2]->ToNumber()->Value();
		uint8_t instance = info[3]->ToNumber()->Value();
		uint8_t index = info[4]->ToNumber()->Value();

		NodeInfo *node;
		std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if (((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {

					switch ((*vit).GetType()) {
						case OpenZWave::ValueID::ValueType_Bool: {
							//bool val; OpenZWave::Manager::Get()->GetValueAsBool(*vit, &val);
							bool val = info[5]->ToBoolean()->Value();
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Byte: {
							//uint8_t val; OpenZWave::Manager::Get()->GetValueAsByte(*vit, &val);
							uint8_t val = info[5]->ToInteger()->Value();
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Decimal: {
							//float val; OpenZWave::Manager::Get()->GetValueAsFloat(*vit, &val);
							float val = info[5]->ToNumber()->NumberValue();
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Int: {
							//int32_t val; OpenZWave::Manager::Get()->GetValueAsInt(*vit, &val);
							int32_t val = info[5]->ToInteger()->Value();
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_List: {
							//std::string val; OpenZWave::Manager::Get()->GetValueListSelection(*vit, &val);
							std::string val = (*String::Utf8Value(info[5]->ToString()));
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_Short: {
							//int16_t val; OpenZWave::Manager::Get()->GetValueAsShort(*vit, &val);
							int16_t val = info[5]->ToInteger()->Value();
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
							break;
						}
						case OpenZWave::ValueID::ValueType_String: {
							//std::string val; OpenZWave::Manager::Get()->GetValueAsString(*vit, &val);
							std::string val = (*String::Utf8Value(info[5]->ToString()));
							OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
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

		
	}

	// ===================================================================
	NAN_METHOD(OZW::RemoveSceneValue)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t sceneid = info[0]->ToNumber()->Value();
		uint8_t nodeid = info[1]->ToNumber()->Value();
		uint8_t comclass = info[2]->ToNumber()->Value();
		uint8_t instance = info[3]->ToNumber()->Value();
		uint8_t index = info[4]->ToNumber()->Value();

		SceneInfo *scene;

		std::list<OpenZWave::ValueID>::iterator vit;

		if ((scene = get_scene_info(sceneid))) {
			for (vit = scene->values.begin(); vit != scene->values.end(); ++vit) {
				if (((*vit).GetNodeId() == nodeid) && ((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {
					OpenZWave::Manager::Get()->RemoveSceneValue(sceneid, *vit);
					scene->values.erase(vit);
					break;
				}
			}
		}

		
	}

	// ===================================================================
	NAN_METHOD(OZW::SceneGetValues)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t sceneid = info[0]->ToNumber()->Value();

		std::vector<OpenZWave::ValueID> values;
		std::vector<OpenZWave::ValueID>::iterator vit;

		OpenZWave::Manager::Get()->SceneGetValues(sceneid, &values);

		SceneInfo *scene;

		if ((scene = get_scene_info(sceneid))) {
			scene->values.clear();

			Local<Array> v8values = Nan::New<Array>(scene->values.size());

			unsigned j = 0;

			for (vit = values.begin(); vit != values.end(); ++vit) {
				mutex::scoped_lock sl(zscenes_mutex);
				scene->values.push_back(*vit);

				v8values->Set(Nan::New<Integer>(j++), zwaveSceneValue2v8Value(sceneid, *vit));
			}
			info.GetReturnValue().Set(v8values);
		}
		
		
	}

	// ===================================================================
	NAN_METHOD(OZW::ActivateScene)
	// ===================================================================
	{
		Nan::HandleScope();

		uint8_t sceneid = info[0]->ToNumber()->Value();

		SceneInfo *scene;

		if ((scene = get_scene_info(sceneid))) {
			OpenZWave::Manager::Get()->ActivateScene(sceneid);
		}

		
	}
}
