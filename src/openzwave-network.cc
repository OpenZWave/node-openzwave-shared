/*
* Heal network node by requesting the node rediscover their neighbors.
*/
Handle<Value> OZW::HealNetworkNode(const Arguments& args)
{
	HandleScope scope;

	uint8_t nodeid = args[0]->ToNumber()->Value();
	uint8_t doRR = args[1]->ToBoolean()->Value();

	OpenZWave::Manager::Get()->HealNetworkNode(homeid, nodeid, doRR);

	return scope.Close(Undefined());
}

/*
* Heal network by requesting node's rediscover their neighbors.
* Sends a ControllerCommand_RequestNodeNeighborUpdate to every node.
* Can take a while on larger networks.
*/
Handle<Value> OZW::HealNetwork(const Arguments& args)
{
	HandleScope scope;

	bool doRR = true;
	OpenZWave::Manager::Get()->HealNetwork(homeid, doRR);

	return scope.Close(Undefined());
}


