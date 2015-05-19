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
