# Events
This is a list of all the events emitted by the library. This list is not exhaustive, please take a look at `src/callbacks.cc` for calls to `emit_cb()` for a full list.

## Driver events
Events emitted by the OpenZWave Driver class:


* `zwave.on('driver ready', function(homeid){...})`

the OpenZWave driver has initialised and scanning has started.  Returns a unique `homeid` which identifies this particular ZWave network.




* `zwave.on('driver failed', function(){...})`

The OpenZWave driver failed to initialise.




* `zwave.on('scan complete', function(){...})`

The initial network scan has finished.




## Node events:

* `zwave.on('node added', function(nodeid){...})`

A new node has been found on the network.  At this point you can allocate resources to hold information about this node.


* `zwave.on('node removed', function(nodeid){...})`

A node with the provided id has been just been removed from the network. You need to deallocate all resources for this nodeid.


* `zwave.on('node naming', function(nodeid, nodeinfo){...})`

Useful information about the node is returned as a plain JS object. It includes elements like 'manufacturer', 'product', 'type' and 'name' amongst others.



* `zwave.on('node available', function(nodeid, nodeinfo){...})`

This corresponds to OpenZWave's `EssentialNodeQueriesComplete` notification, which means that the node is now available for operation, but don't expect all of its info structures (nodeinfo, see below) to be filled in.



* `zwave.on('node ready', function(nodeid, nodeinfo){...})`

This corresponds to OpenZWave's `NodeQueriesComplete` notification. The node is now ready for operation, and information about the node is available in the `nodeinfo` object:

	- nodeinfo.manufacturer
	- nodeinfo.manufacturerid
	- nodeinfo.product
	- nodeinfo.producttype
	- nodeinfo.productid
	- nodeinfo.type
	- nodeinfo.name
	- nodeinfo.loc (location, renamed to avoid `location` keyword).




* `zwave.on('polling enabled/disabled', function(nodeid){...})`

Polling for a node has been enabled or disabled.




* `zwave.on('scene event', function(nodeid, sceneid){...});`  

This is fired when a scene event is received by the controller.
For example when you have your Aeon Labs Minimote setup with the following configuration:

	zwave.setConfigParam(nodeid, 241, 1, 1);
	zwave.setConfigParam(nodeid, 242, 1, 1);
	zwave.setConfigParam(nodeid, 243, 1, 1);
	zwave.setConfigParam(nodeid, 244, 1, 1);
	zwave.setConfigParam(nodeid, 250, 1, 1);

It would send:

	- sceneid of 1 when (1) is Pressed
	- sceneid of 2 when (1) is Held
	- sceneid of 3 when (2) is Pressed
	- etc.




* `zwave.on('node event', function(nodeid, data) {});`

	This event gets called when a Basic set command is received by the controller.
	This might indicate that the node has changed (eg due to manual operation using
	the local button or switch) or that a sensor device was triggered.
	As an example, an Aeon Labs Water sensor will fire this event and `data` would
	be 255 when water is detected and 0 when it isn't.




## Value events:

* `zwave.on('value added', function(nodeid, commandclass, valueId){...})`

A new ValueID has been discovered.  ValueIDs are compound structs ("objects") associated with a particular node, and are the parts of the device you can monitor or control. Please see [the official OpenZWave docs on ValueIDs](http://www.openzwave.com/dev/classOpenZWave_1_1ValueID.html) for more details.

ValueId's are split into command classes.  The most commonly used ones and
their unique identifiers are:

	- COMMAND_CLASS_SWITCH_BINARY (37)
	- COMMAND_CLASS_SWITCH_MULTILEVEL (38)

Binary switches can be controlled with `.setNodeOn()` and `.setNodeOff()`.
Dimmer (multi-level) devices can be set with `.setLevel()` (*if of course they
support the **BASIC** command class, which is not always supported by most dimmers!*
Use `setValue` instead)

	- COMMAND_CLASS_VERSION (134)
The version class is informational only and cannot be controlled.

The `valueId` object differs between command classes, and contains all the useful
information about values stored for the particular class. You can use this object
as the 1st argument in `setValue` to alter its state.




* `zwave.on('value changed', function(nodeid, commandclass, valueId){...})`

A valueId has changed.  Use this to keep track of value state across the network. When values are first discovered, the module enables polling on those values so that we will receive change messages. Prior to the 'node ready' event, there may be 'value changed' events even when no values were actually changed.




* `zwave.on('value refreshed', function(nodeid, commandclass, valueId){...})`

A node valueId has been updated from the Z-Wave network. Unlike 'value changed' which implies an actual change of state, this one simply means that the value has been refreshed from the device.




* `zwave.on('value removed', function(nodeid, commandclass, instance, index){...})`

A valueId has been removed.  Your program should then remove any references to that ValueID.




## Controller events:

* `zwave.on('controller command', function(nodeId, ctrlState, ctrlError, helpmsg){...})`

The ZWave Controller is reporting the result of the currently active command. Check out official OpenZWave documentation on [ControllerState](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#a5595393f6aac3175bb17f00cf53356a8) and [ControllerError](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#a16d2da7b78f8eefc79ef4046d8148e7c) so for instance, controller state #7 (remember: 0-based arrays) is **ControllerState_Completed** which is the result you should expect from successful controller command completion. A help string is also passed for display purposes.

*Please Note:* OpenZWave's management API has slightly changed with the inclusion of the security framework. As of version 1.3, OpenZWave has deprecated `BeginControllerCommand` and has added a separate method for each of the controller commands. This seems to have the side-effect that some extra management events are fired upon initialisation that were not being fired before. As such, be warned that you might get some callbacks on `controller command` even though you've not sent any actual controller commands.
