// http://openzwave.com/dev/classOpenZWave_1_1Manager.html#aa11faf40f19f0cda202d2353a60dbf7b

/*
 * 
_homeId	The Home ID of the Z-Wave controller.
_command	The command to be sent to the controller.
_callback	pointer to a function that will be called at various stages during the command process to notify the user of progress or to request actions on the user's part. Defaults to NULL.
_context	pointer to user defined data that will be passed into to the callback function. Defaults to NULL.
_highPower	used only with the AddDevice, AddController, RemoveDevice and RemoveController commands. Usually when adding or removing devices, the controller operates at low power so that the controller must be physically close to the device for security reasons. If _highPower is true, the controller will operate at normal power levels instead. Defaults to false.
_nodeId	is the node ID used by the command if necessary.
_arg	is an optional argument, usually another node ID, that is used by the command.
* */
OpenZWave::Manager::Get()->BeginControllerCommand	(	uint32 const 	_homeId,
	Driver::ControllerCommand 	_command,
	Driver::pfnControllerCallback_t 	_callback = NULL,
	void * 	_context = NULL,
	bool 	_highPower = false,
	uint8 	_nodeId = 0xff,
	uint8 	_arg = 0 
	)	
	
	
	
	
	
OpenZWave::Manager::Get()->CancelControllerCommand	(	uint32 const 	_homeId	)	
