
Handle<Value> OZW::Connect(const Arguments& args) {
	HandleScope scope;

	std::string path = (*String::Utf8Value(args[0]->ToString()));

	uv_async_init(uv_default_loop(), &async, async_cb_handler);

	context_obj = Persistent < Object > ::New(args.This());

	OpenZWave::Manager::Create();
	OpenZWave::Manager::Get()->AddWatcher(cb, NULL);
	OpenZWave::Manager::Get()->AddDriver(path);

	Handle<Value> argv[1] = { String::New("connected") };
	MakeCallback(context_obj, "emit", 1, argv);

	return Undefined();
}

Handle<Value> OZW::Disconnect(const Arguments& args) {
	HandleScope scope;

	std::string path = (*String::Utf8Value(args[0]->ToString()));

	OpenZWave::Manager::Get()->RemoveDriver(path);
	OpenZWave::Manager::Get()->RemoveWatcher(cb, NULL);
	OpenZWave::Manager::Destroy();
	OpenZWave::Options::Destroy();

	return scope.Close(Undefined());
}

/*
* Reset the ZWave controller chip.  A hard reset is destructive and wipes
* out all known configuration, a soft reset just restarts the chip.
*/
Handle<Value> OZW::HardReset(const Arguments& args) {
	HandleScope scope;

	OpenZWave::Manager::Get()->ResetController(homeid);

	return scope.Close(Undefined());
}

Handle<Value> OZW::SoftReset(const Arguments& args) {
	HandleScope scope;

	OpenZWave::Manager::Get()->SoftReset(homeid);

	return scope.Close(Undefined());
}

