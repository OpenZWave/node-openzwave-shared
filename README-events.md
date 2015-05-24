### Events

The supported events are:

#### `.on('connected', function(){})`

We have connected to an OpenZWave node.

#### `.on('driver ready', function(homeid){})`

The OpenZWave driver has initialised and scanning has started.  Returns a
unique `homeid` which identifies this particular network.

#### `.on('driver failed', function(){})`

The OpenZWave driver failed to initialise.

#### `.on('node added', function(nodeid){})`

A new node has been found on the network.  At this point you can allocate
resources to hold information about this node.

#### `.on('value added', function(nodeid, commandclass, value){})`

A new value has been discovered.  Values are associated with a particular node,
and are the parts of the device you can monitor or control.

Values are split into command classes.  The classes currently supported and
their unique identifiers are:

* `COMMAND_CLASS_SWITCH_BINARY` (37)
* `COMMAND_CLASS_SWITCH_MULTILEVEL` (38)
* `COMMAND_CLASS_VERSION` (134)

Binary switches can be controlled with `.setNodeOn()` and `.setNodeOff()`.

Multi-level devices can be set with `.setLevel()`.

The version class is informational only and cannot be controlled.

The `value` object differs between command classes, and contains all the useful
information about values stored for the particular class.

#### `.on('value changed', function(nodeid, commandclass, value){})`

A value has changed.  Use this to keep track of value state across the network.
When values are first discovered, the module enables polling on those values so
that we will receive change messages.

Prior to the 'node ready' event, there may be 'value changed' events even when
no values were actually changed.

#### `.on('value removed', function(nodeid, commandclass, index){})`

A value has been removed.  Use the index to calculate the offset where a
command class can contain multiple values.

#### `.on('node ready', function(nodeid, nodeinfo){})`

A node is now ready for operation, and information about the node is available
in the `nodeinfo` object:

* `nodeinfo.manufacturer`
* `nodeinfo.product`
* `nodeinfo.type`
* `nodeinfo.loc` (location, renamed to avoid `location` keyword).

#### `.on('controller command', function(controllerState, controllerError){})`

The ZWave Controller is reporting the result of the currently active command. 
Check out official OpenZWave documentation on 
[ControllerState](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#a5595393f6aac3175bb17f00cf53356a8)
and
[ControllerError](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#a16d2da7b78f8eefc79ef4046d8148e7c)

#### `.on('scan complete', function(){})`

The initial network scan has finished.
