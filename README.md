node-openzwave-shared
=====================

This is a node.js add-on which wraps the [Open
Z-Wave](https://www.openzwave.com/) library to provide access to a
Z-Wave network from JavaScript.

It is currently able to scan a Z-Wave network, report on connected devices,
monitor the network for changes, and has rudimentary write support.

*Important notice*
This fork uses OpenZWave as a *shared library*, so you need to have 
OpenZWave fully installed on your system (both the compiled library
AND the development headers) before trying to install this little baby.
This also means that its definately going to break if you upgrade your 
OZW without rebuilding this plugin. So don't control nuclear reactors 
with it, OK?

Manually compiling OpenZwave involves the usual dance of 
(`make && sudo make install`) OR you could try installing
a precompiled package (eg for Raspbian it should be like
`sudo apt-get install libopenzwave-dev`). 

Precompiled packages for various Linux flavours and architectures
can also be downloaded from http://old.openzwave.com/snapshots/.
Be sure to install BOTH the binary (libopenzwave-x.y) AND the development
package (libopenzwave-dev).

## Install

The module currently builds only on Linux. On Linux you will need to
ensure the OpenZWave library and headers are installed first.

```
$ sudo npm install -g openzwave-shared
```

## API

Start by loading the addon and creating a new instance, specifying a path to
the USB device:

```js
var OZW = require('openzwave-shared');
var zwave = new OZW('/dev/ttyUSB0');
```

An optional object can be passed at creation time to alter the behavior of the
ZWave module.  The options currently supported and their defaults are:

```js
var zwave = new OZW('/dev/ttyUSB0', {
        logging: false,           // enable logging to OZW_Log.txt
        consoleoutput: false,     // copy logging to the console
        saveconfig: false,        // write an XML network layout
        driverattempts: 3,        // try this many times before giving up
        pollinterval: 500,        // interval between polls in milliseconds
        suppressrefresh: true,    // do not send updates if nothing changed
});
```

The rest of the API is split into Functions and Events.  Messages from the
Z-Wave network are handled by `EventEmitter`, and you will need to listen for
specific events to correctly map the network.

### Functions

Connecting to the network:

```js
zwave.connect();     // initialise and start a new driver.
zwave.disconnect();  // disconnect from the current connection
```

Modifying device state:

```js
/*
 * Set a multi-level device to the specified level (between 0-99).
 */
zwave.setLevel(5, 50); // node 5: dim to 50%

/*
 * Turn a binary switch on/off.
 */
zwave.setNodeOn(3); // node 3: switch ON
zwave.setNodeOff(3);// node 3: switch OFF

/*
 * Set arbitrary values.
 */
zwave.setValue(nodeid, commandclass, instance, index, value);
```

This is useful for multi-instance devices, such as the Fibaro FGS-221 eg:
```js
zwave.setValue(8, 37, 1, 0, true); // node 8: turn on 1st relay
zwave.setValue(8, 37, 1, 0, false);// node 8: turn off 1st relay
zwave.setValue(8, 37, 2, 0, true); // node 8: turn on 2nd relay
zwave.setValue(8, 37, 2, 0, false);// node 8: turn off 2nd relay
```


Writing to device metadata (stored on the device itself):

```js
zwave.setLocation(nodeid, location);    // arbitrary location string
zwave.setName(nodeid, name);            // arbitrary name string
```

Polling a device for changes (not all devices require this):

```js
zwave.enablePoll(nodeid, commandclass);
zwave.disablePoll(nodeid, commandclass);
```

Reset the controller.  Calling `hardReset` will clear any associations, so use
carefully:

```js
```

Scenes control:
```js
createScene
removeScene
getScenes
addSceneValue
removeSceneValue
sceneGetValues
activateScene
```

ZWave network management:
```js
zwave.hardReset();      // destructive! will wipe out all configuration stored in the chip
zwave.softReset();      // non-destructive, just resets the chip
zwave.healNetworkNode
zwave.healNetwork
zwave.getNeighbors
```


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

#### `.on('scan complete', function(){})`

The initial network scan has finished.

## Example

The test program below connects to a Z-Wave network, scans for all nodes and
values, and prints out information about the network.  It will then continue to
scan for changes until the user hits `^C`.

```js
/*
 * OpenZWave test program.
 */

var OpenZWave = require('openzwave-shared');

var zwave = new OpenZWave('/dev/ttyUSB0', {
	saveconfig: true,
});
var nodes = [];

zwave.on('driver ready', function(homeid) {
	console.log('scanning homeid=0x%s...', homeid.toString(16));
});

zwave.on('driver failed', function() {
	console.log('failed to start driver');
	zwave.disconnect();
	process.exit();
});

zwave.on('node added', function(nodeid) {
	nodes[nodeid] = {
		manufacturer: '',
		manufacturerid: '',
		product: '',
		producttype: '',
		productid: '',
		type: '',
		name: '',
		loc: '',
		classes: {},
		ready: false,
	};
});

zwave.on('value added', function(nodeid, comclass, value) {
	if (!nodes[nodeid]['classes'][comclass])
		nodes[nodeid]['classes'][comclass] = {};
	nodes[nodeid]['classes'][comclass][value.index] = value;
});

zwave.on('value changed', function(nodeid, comclass, value) {
	if (nodes[nodeid]['ready']) {
		console.log('node%d: changed: %d:%s:%s->%s', nodeid, comclass,
			    value['label'],
			    nodes[nodeid]['classes'][comclass][value.index]['value'],
			    value['value']);
	}
	nodes[nodeid]['classes'][comclass][value.index] = value;
});

zwave.on('value removed', function(nodeid, comclass, index) {
	if (nodes[nodeid]['classes'][comclass] &&
	    nodes[nodeid]['classes'][comclass][index])
		delete nodes[nodeid]['classes'][comclass][index];
});

zwave.on('node ready', function(nodeid, nodeinfo) {
	nodes[nodeid]['manufacturer'] = nodeinfo.manufacturer;
	nodes[nodeid]['manufacturerid'] = nodeinfo.manufacturerid;
	nodes[nodeid]['product'] = nodeinfo.product;
	nodes[nodeid]['producttype'] = nodeinfo.producttype;
	nodes[nodeid]['productid'] = nodeinfo.productid;
	nodes[nodeid]['type'] = nodeinfo.type;
	nodes[nodeid]['name'] = nodeinfo.name;
	nodes[nodeid]['loc'] = nodeinfo.loc;
	nodes[nodeid]['ready'] = true;
	console.log('node%d: %s, %s', nodeid,
		    nodeinfo.manufacturer ? nodeinfo.manufacturer
					  : 'id=' + nodeinfo.manufacturerid,
		    nodeinfo.product ? nodeinfo.product
				     : 'product=' + nodeinfo.productid +
				       ', type=' + nodeinfo.producttype);
	console.log('node%d: name="%s", type="%s", location="%s"', nodeid,
		    nodeinfo.name,
		    nodeinfo.type,
		    nodeinfo.loc);
	for (comclass in nodes[nodeid]['classes']) {
		switch (comclass) {
		case 0x25: // COMMAND_CLASS_SWITCH_BINARY
		case 0x26: // COMMAND_CLASS_SWITCH_MULTILEVEL
			zwave.enablePoll(nodeid, comclass);
			break;
		}
		var values = nodes[nodeid]['classes'][comclass];
		console.log('node%d: class %d', nodeid, comclass);
		for (idx in values)
			console.log('node%d:   %s=%s', nodeid, values[idx]['label'], values[idx]['value']);
	}
});

zwave.on('notification', function(nodeid, notif) {
	switch (notif) {
	case 0:
		console.log('node%d: message complete', nodeid);
		break;
	case 1:
		console.log('node%d: timeout', nodeid);
		break;
	case 2:
		console.log('node%d: nop', nodeid);
		break;
	case 3:
		console.log('node%d: node awake', nodeid);
		break;
	case 4:
		console.log('node%d: node sleep', nodeid);
		break;
	case 5:
		console.log('node%d: node dead', nodeid);
		break;
	case 6:
		console.log('node%d: node alive', nodeid);
		break;
        }
});

zwave.on('scan complete', function() {
	console.log('scan complete, hit ^C to finish.');
});

zwave.connect();

process.on('SIGINT', function() {
	console.log('disconnecting...');
	zwave.disconnect();
	process.exit();
});
```

Sample output from this program:

```sh
$ node test.js 2>/dev/null
scanning homeid=0x161db5f...
node1: Aeon Labs, Z-Stick S2
node1: name="", type="Static PC Controller", location=""
node1: class 32
node1:   Basic=0
node11: Everspring, AD142 Plug-in Dimmer Module
node11: name="", type="Multilevel Power Switch", location=""
node11: class 32
node11: class 38
node11:   Level=89
node11:   Bright=undefined
node11:   Dim=undefined
node11:   Ignore Start Level=true
node11:   Start Level=0
node11: class 39
node11:   Switch All=3073
node11: class 115
node11:   Powerlevel=3073
node11:   Timeout=0
node11:   Set Powerlevel=undefined
node11:   Test Node=0
node11:   Test Powerlevel=3072
node11:   Frame Count=0
node11:   Test=undefined
node11:   Report=undefined
node11:   Test Status=3072
node11:   Acked Frames=0
node11: class 117
node11:   Protection=3072
node11: class 134
node11:   Library Version=4
node11:   Protocol Version=2.64
node11:   Application Version=1.02
node12: Wenzhou TKB Control System, product=0103, type=0101
node12: name="", type="Binary Power Switch", location=""
node12: class 32
node12: class 37
node12:   Switch=true
node12: class 39
node12:   Switch All=3161
node12: class 134
node12:   Library Version=6
node12:   Protocol Version=3.40
node12:   Application Version=1.04
node13: Wenzhou TKB Control System, product=0103, type=0101
node13: name="", type="Binary Power Switch", location=""
node13: class 32
node13: class 37
node13:   Switch=true
node13: class 39
node13:   Switch All=3073
node13: class 134
node13:   Library Version=6
node13:   Protocol Version=3.40
node13:   Application Version=1.04
node10: Popp / Duwi, ZW ESJ Blind Control
node10: name="", type="Multiposition Motor", location=""
node10: class 32
node10: class 37
node10:   Switch=true
node10: class 38
node10:   Level=99
node10:   Bright=undefined
node10:   Dim=undefined
node10:   Ignore Start Level=true
node10:   Start Level=0
node10: class 39
node10:   Switch All=3073
node10: class 117
node10:   Protection=3073
node10: class 134
node10:   Library Version=6
node10:   Protocol Version=2.51
node10:   Application Version=1.00
node10: class 135
node10:   Indicator=0
scan complete, hit ^C to finish.
^Cdisconnecting...
```

Remove `2>/dev/null` to get verbose output of all incoming notification types
and additional debug information.

## License

The [Open Z-Wave](https://code.google.com/p/open-zwave/) library that this
module heavily relies upon is licensed under the GPLv3.

Everything else (all the bits that I have written) is under the vastly more
sensible ISC license.
