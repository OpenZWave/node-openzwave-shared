/*
* Generic value set.
*/
Handle<Value> OZW::SetValue(const Arguments& args) {
	HandleScope scope;

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

	return scope.Close(Undefined());
}


