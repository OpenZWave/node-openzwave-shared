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
