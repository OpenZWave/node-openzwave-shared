## API

Start by loading the addon with `require`:
```js
var OZW = require('openzwave-shared');
```
and then create a new instance of the addon:
```js
var zwave = new OZW();
```
You can also pass in an optional object specifying any desired option overrides:
```js
var zwave = new OZW({
	Logging: false,		// disable file logging (OZWLog.txt)
    ConsoleOutput: true // enable console logging
});
```
The default options are specified in `config/options.xml`. Please refer
[to the full list of OpenZWave options](https://github.com/OpenZWave/open-zwave/wiki/Config-Options)
for all the available options. If, for instance, you're using security devices
(e.g. door locks) then you should specify an encryption key.

The rest of the API is split into Functions and Events.  Messages from the
Z-Wave network are handled by `EventEmitter`, and you will need to listen for
specific events to correctly map the network.

### Functions

Connecting to the network:
```js
// for Linux/Mac OSX
zwave.connect('/dev/ttyUSB0');  // connect to a USB ZWave controller
zwave.disconnect('dev/ttyUSB0');// disconnect from the current connection

// for Windows, COM port #x notation is \\.\COMx
zwave.connect('\\\\.\\COM3');  // connect to a USB ZWave controller on COM3
zwave.disconnect('\\\\.\\COM3');// disconnect from the current connection on COM3
```
**Important notice**: the connect() call is asynchronous following the
node/v8 javascript paradigm.  This means that connect() will yield
control to your script *immediately*, but the underlying OpenZWave C++
library will *not be ready yet* to accept commands.
In fact, it can take some time (from a few seconds to a couple of
minutes!) to scan the ZWave network and set up its data structures.
So, be sure to register a "scan complete" callback, and after it gets called,
you can safely start issuing commands to your ZWave devices.

Modifying device state:
```js
/*
 * Set arbitrary values.
 */
// 1) by means of passing each individual ValueID constituent:
zwave.setValue(nodeid, commandclass, instance, index, value);
zwave.setValue(3,      37,           1,        0,     true); // node 3: turn on
zwave.setValue(3,      37,           1,        0,     false); // node 3: turn off
// dimmer node 5: set to 50%
zwave.setValue(5,      38,           1,        0,     50);
// 2) or by passing the valueID object (emitted by ValueAdded event):
zwave.setValue({ node_id:5, class_id: 38, instance:1, index:0}, 50);

/*
 * Turn a binary switch on/off.
 */
zwave.setNodeOn(3); // node 3: switch ON
zwave.setNodeOff(3);// node 3: switch OFF

/*
 * Set a multi-level device to the specified level (between 0-99).
 * See warning below
 */
zwave.setLevel(5, 50); // node 5: dim to 50%
```

*WARNING: setNodeOn/Off/Level _don't work reliably with all devices_*, as they are
mere aliases to the BASIC command class. Not all devices support this. Please
consult your device's manual to see if it supports this command class.
The 'standard' way to control your devices is by `setValue` which is also the
_only_ way to control multi-instance devices, such as the Fibaro FGS-221
(double in-wall 2x1,5kw relay) for example:
```js
zwave.setValue(8, 37, 1, 0, true); // node 8: turn on 1st relay
zwave.setValue(8, 37, 1, 0, false);// node 8: turn off 1st relay
zwave.setValue(8, 37, 2, 0, true); // node 8: turn on 2nd relay
zwave.setValue(8, 37, 2, 0, false);// node 8: turn off 2nd relay
```
Useful documentation on [command classes can be found on MiCasaVerde website](http://wiki.micasaverde.com/index.php/ZWave_Command_Classes)

Writing to device metadata (stored on the device itself):
```js
zwave.setNodeLocation(nodeid, location);    // arbitrary location string
zwave.setNodeName(nodeid, name);            // arbitrary name string
```

Polling a device for changes (not all devices require this):
```js
zwave.enablePoll(nodeid, commandclass, intensity);
zwave.disablePoll(nodeid, commandclass);
zwave.setPollInterval(nodeid, )
zwave.getPollInterval();
zwave.isPolled();
zwave.setPollIntensity();
zwave.getPollIntensity();
```

Association groups management:
```js
zwave.getNumGroups(nodeid);
zwave.getGroupLabel(nodeid, group);
zwave.getAssociations(nodeid, group);
zwave.getMaxAssociations(nodeid, group);
zwave.addAssociation(nodeid, group, target_nodeid);
zwave.removeAssociation(nodeid, group, target_nodeid);
```

Resetting the controller.  Calling `hardReset` will clear any associations, so use
carefully:
```js
zwave.hardReset();      // destructive! will wipe out all known configuration
zwave.softReset();      // non-destructive, just resets the chip
```

Scenes control:
```js
zwave.createScene(label); 	// create a scene and assign a label, return its numeric id.
zwave.removeScene(sceneId); // perform #GRExit
zwave.getScenes();			// get all scenes as an array
// add a zwave value to a scene
zwave.addSceneValue(sceneId, nodeId, commandclass, instance, index);
// remove a zwave value from a scene
zwave.removeSceneValue(sceneId, nodeId, commandclass, instance, index);
zwave.sceneGetValues(sceneId); // return array of values associated with this scene
zwave.activateScene(sceneId);  // The Show Must Go On...
```

ZWave network commands:
```js
zwave.healNetworkNode(nodeId, doReturnRoutes=false);
zwave.healNetwork();   // guru meditation
zwave.getNeighbors();
zwave.refreshNodeInfo(nodeid);
```

ZWave controller commands:
```js
// begin an async controller command on node1:
zwave.beginControllerCommand( "command name", highPower = false, node1_id, node2_id = null);  
// cancel controller command in progress
zwave.cancelControllerCommand();
// returns controller's node id
zwave.getControllerNodeId();
// returns static update controller node id
zwave.getSUCNodeId();
// is the OZW-managed controller the primary controller for this zwave network?
zwave.isPrimaryController();
// Query if the controller is a static update controller.
zwave.isStaticUpdateController();
// Query if the controller is using the bridge controller library.
zwave.isBridgeController();
// Get the version of the Z-Wave API library used by a controller.
zwave.getLibraryVersion();
// Get a string containing the Z-Wave API library type used by a controller
zwave.getLibraryTypeName();
//
zwave.getSendQueueCount();
```


### Configuration commands:
```js
zwave.requestAllConfigParams(nodeId);
zwave.requestConfigParam(nodeId, paramId);
zwave.setConfigParam(nodeId, paramId, paramValue, <sizeof paramValue>);
```
