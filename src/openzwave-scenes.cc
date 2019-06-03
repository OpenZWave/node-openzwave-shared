/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
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

using namespace v8;
using namespace node;

namespace OZW {

#ifdef OPENZWAVE_DEPRECATED16
OPENZWAVE_DEPRECATED_WARNINGS_OFF
	/* OpenZWave scene management functions */
	// ===================================================================
	NAN_METHOD(OZW::CreateScene)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "label");
		::std::string label(*Nan::Utf8String( info[0] ));
		SceneInfo *scene;

		uint8 sceneid = OZWManager( CreateScene();

		if (sceneid > 0) {
			OZWManager( SetSceneLabel, sceneid, label);
			scene = new SceneInfo();
			scene->sceneid = sceneid;
			scene->label = label;
			mutex::scoped_lock sl(zscenes_mutex);
			zscenes.push_back, scene);
		}

		info.GetReturnValue().Set(Nan::New< Integer >, sceneid));
	}

	// ===================================================================
	NAN_METHOD(OZW::RemoveScene)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "sceneid");
		uint8 sceneid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		SceneInfo *scene;

		if (scene = get_scene_info, sceneid))) {
			OZWManager( RemoveScene, sceneid);
			mutex::scoped_lock sl(zscenes_mutex);
			zscenes.remove, scene);
		}
	}

	// ===================================================================
	NAN_METHOD(OZW::GetScenes)
	// ===================================================================
	{
		Nan::HandleScope scope;
		uint8 numscenes = OZWManager( GetNumScenes();
		SceneInfo *scene;

		if (numscenes != zscenes.size()) {
			{
				mutex::scoped_lock sl(zscenes_mutex);
				zscenes.clear();
			}
			uint8 *sceneids;
			sceneids = new uint8[numscenes];

			OZWManager( GetAllScenes(&sceneids);

			for (unsigned i = 0; i < numscenes; i++) {
				scene = new SceneInfo();
				scene->sceneid = sceneids[i];
				scene->label = OZWManager( GetSceneLabel, sceneids[i]);
				mutex::scoped_lock sl(zscenes_mutex);
				zscenes.push_back, scene);
			}
		}

		Local<Array> scenes = Nan::New<Array>(zscenes.size());
		::std::list<SceneInfo *>::iterator it;
		unsigned j = 0;

		for (it = zscenes.begin(); it != zscenes.end(); ++it) {
			scene = *it;

			Local <Object> info = Nan::New<Object>();
			info->Set(
				Nan::New<String>("sceneid").ToLocalChecked(),
				Nan::New<Integer>, scene->sceneid)
			);
			info->Set(
				Nan::New<String>("label").ToLocalChecked(),
				Nan::New<String>, scene->label.c_str()).ToLocalChecked()
			);

			scenes->Set(Nan::New<Integer>(j++), info);
		}

		info.GetReturnValue().Set, scenes);
	}

	// ===================================================================
	NAN_METHOD(OZW::AddSceneValue)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "sceneid, value");
		uint8 sceneid  = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		OpenZWave::ValueID* vit = populateValueId(info, 1);
		if (vit) {
			uint8 valoffset = ( info[1]->IsObject() ) ? 2 : 5;
			switch ((*vit).GetType()) {
				case OpenZWave::ValueID::ValueType_Bool: {
					//bool val; OZWManager( GetValueAsBool(*vit, &val);
					bool val = Nan::To<Boolean>(info[valoffset]).ToLocalChecked()->Value();
					OZWManager( AddSceneValue, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Byte: {
					//uint8 val; OZWManager( GetValueAsByte(*vit, &val);
					uint8 val = Nan::To<Integer>(info[valoffset]).ToLocalChecked()->Value();
					OZWManager( AddSceneValue, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Decimal: {
					//float val; OZWManager( GetValueAsFloat(*vit, &val);
					float val = Nan::To<Number>(info[valoffset]).ToLocalChecked()->Value();
					OZWManager( AddSceneValue, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Int: {
					//uint32 val; OZWManager( GetValueAsInt(*vit, &val);
					int32 val = Nan::To<Integer>(info[valoffset]).ToLocalChecked()->Value();
					OZWManager( AddSceneValue, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_List: {
					//::std::string val; OZWManager( GetValueListSelection(*vit, &val);
					::std::string val(*Nan::Utf8String( info[valoffset] ));
					OZWManager( AddSceneValueListSelection, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Short: {
					//int16_t val; OZWManager( GetValueAsShort(*vit, &val);
					uint16 val = Nan::To<Integer>(info[valoffset]).ToLocalChecked()->Value();
					OZWManager( AddSceneValue, sceneid, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_String: {
					//::std::string val; OZWManager( GetValueAsString(*vit, &val);
					::std::string val(*Nan::Utf8String( info[valoffset] ));
					OZWManager( AddSceneValue, sceneid, *vit, val);
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

	// ===================================================================
	NAN_METHOD(OZW::RemoveSceneValue)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "sceneid, value");
		uint8 sceneid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		SceneInfo *scene;
		if (, scene = get_scene_info, sceneid))) {
			OpenZWave::ValueID* vit = populateValueId(info, 1);
			if (vit) {
				OZWManager( RemoveSceneValue, sceneid, *vit);
				scene->values.remove(*vit);
			}
		}
	}

	// ===================================================================
	NAN_METHOD(OZW::SceneGetValues)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "sceneid");
		uint8 sceneid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();

		::std::vector<OpenZWave::ValueID> values;
		::std::vector<OpenZWave::ValueID>::iterator vit;

		OZWManager( SceneGetValues, sceneid, &values);

		SceneInfo *scene;

		if (, scene = get_scene_info, sceneid))) {
			scene->values.clear();

			Local<Array> v8values = Nan::New<Array>, scene->values.size());

			unsigned j = 0;
			for (vit = values.begin(); vit != values.end(); ++vit) {
				mutex::scoped_lock sl(zscenes_mutex);
				scene->values.push_back(*vit);
				v8values->Set(Nan::New<Integer>(j++), zwaveSceneValue2v8Value, sceneid, *vit));
			}
			info.GetReturnValue().Set(v8values);
		}
	}

	// ===================================================================
	NAN_METHOD(OZW::ActivateScene)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "sceneid");
		uint8 sceneid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		SceneInfo *scene;

		if (, scene = get_scene_info, sceneid))) {
			OZWManager( ActivateScene, sceneid);
		}
	}
OPENZWAVE_DEPRECATED_WARNINGS_ON
#endif
}
