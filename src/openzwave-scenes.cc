/* OpenZWave scene management functions */

Handle<Value> OZW::CreateScene(const Arguments& args) {
	HandleScope scope;

	std::string label = (*String::Utf8Value(args[0]->ToString()));

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

	return scope.Close(Undefined());
}

Handle<Value> OZW::RemoveScene(const Arguments& args) {
	HandleScope scope;

	uint8_t sceneid = args[0]->ToNumber()->Value();

	SceneInfo *scene;

	if ((scene = get_scene_info(sceneid))) {
		OpenZWave::Manager::Get()->RemoveScene(sceneid);
		mutex::scoped_lock sl(zscenes_mutex);
		zscenes.remove(scene);
	}

	return scope.Close(Undefined());
}

Handle<Value> OZW::GetScenes(const Arguments& args) {
	HandleScope scope;

	uint8_t numscenes = OpenZWave::Manager::Get()->GetNumScenes();
	Local <Value> cbargs[16];
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

	Local<Array> scenes = Array::New(zscenes.size());
	std::list<SceneInfo *>::iterator it;
	unsigned j = 0;

	for (it = zscenes.begin(); it != zscenes.end(); ++it) {
		scene = *it;

		Local <Object> info = Object::New();
		info->Set(String::NewSymbol("sceneid"), Integer::New(scene->sceneid));
		info->Set(String::NewSymbol("label"), String::New(scene->label.c_str()));

		scenes->Set(Number::New(j++), info);
	}

	cbargs[0] = String::New("scenes list");
	cbargs[1] = scenes;

	MakeCallback(context_obj, "emit", 2, cbargs);

	return scope.Close(Undefined());
}

Handle<Value> OZW::AddSceneValue(const Arguments& args) {
	HandleScope scope;

	uint8_t sceneid = args[0]->ToNumber()->Value();
	uint8_t nodeid = args[1]->ToNumber()->Value();
	uint8_t comclass = args[2]->ToNumber()->Value();
	uint8_t instance = args[3]->ToNumber()->Value();
	uint8_t index = args[4]->ToNumber()->Value();

	NodeInfo *node;
	std::list<OpenZWave::ValueID>::iterator vit;

	if ((node = get_node_info(nodeid))) {
		for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
			if (((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {

				switch ((*vit).GetType()) {
					case OpenZWave::ValueID::ValueType_Bool: {
						//bool val; OpenZWave::Manager::Get()->GetValueAsBool(*vit, &val);
						bool val = args[5]->ToBoolean()->Value();
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_Byte: {
						//uint8_t val; OpenZWave::Manager::Get()->GetValueAsByte(*vit, &val);
						uint8_t val = args[5]->ToInteger()->Value();
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_Decimal: {
						//float val; OpenZWave::Manager::Get()->GetValueAsFloat(*vit, &val);
						float val = args[5]->ToNumber()->NumberValue();
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_Int: {
						//int32_t val; OpenZWave::Manager::Get()->GetValueAsInt(*vit, &val);
						int32_t val = args[5]->ToInteger()->Value();
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_List: {
						//std::string val; OpenZWave::Manager::Get()->GetValueListSelection(*vit, &val);
						std::string val = (*String::Utf8Value(args[5]->ToString()));
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_Short: {
						//int16_t val; OpenZWave::Manager::Get()->GetValueAsShort(*vit, &val);
						int16_t val = args[5]->ToInteger()->Value();
						OpenZWave::Manager::Get()->AddSceneValue(sceneid, *vit, val);
						break;
					}
					case OpenZWave::ValueID::ValueType_String: {
						//std::string val; OpenZWave::Manager::Get()->GetValueAsString(*vit, &val);
						std::string val = (*String::Utf8Value(args[5]->ToString()));
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

	return scope.Close(Undefined());
}

Handle<Value> OZW::RemoveSceneValue(const Arguments& args) {
	HandleScope scope;

	uint8_t sceneid = args[0]->ToNumber()->Value();
	uint8_t nodeid = args[1]->ToNumber()->Value();
	uint8_t comclass = args[2]->ToNumber()->Value();
	uint8_t instance = args[3]->ToNumber()->Value();
	uint8_t index = args[4]->ToNumber()->Value();

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

	return scope.Close(Undefined());
}

Handle<Value> OZW::SceneGetValues(const Arguments& args) {
	HandleScope scope;

	uint8_t sceneid = args[0]->ToNumber()->Value();

	std::vector<OpenZWave::ValueID> values;

	std::vector<OpenZWave::ValueID>::iterator vit;

	OpenZWave::Manager::Get()->SceneGetValues(sceneid, &values);

	SceneInfo *scene;

	if ((scene = get_scene_info(sceneid))) {
		scene->values.clear();

		Local <Value> cbargs[16];

		Local<Array> v8values = Array::New(scene->values.size());

		unsigned j = 0;

		for (vit = values.begin(); vit != values.end(); ++vit) {
			mutex::scoped_lock sl(zscenes_mutex);
			scene->values.push_back(*vit);

			v8values->Set(Number::New(j++), zwaveSceneValue2v8Value(sceneid, *vit));
		}

		cbargs[0] = String::New("scene values list");
		cbargs[1] = v8values;

		MakeCallback(context_obj, "emit", 2, cbargs);
	}

	return scope.Close(Undefined());
}

Handle<Value> OZW::ActivateScene(const Arguments& args) {
	HandleScope scope;

	uint8_t sceneid = args[0]->ToNumber()->Value();

	SceneInfo *scene;

	if ((scene = get_scene_info(sceneid))) {
		OpenZWave::Manager::Get()->ActivateScene(sceneid);
	}

	return scope.Close(Undefined());
}

