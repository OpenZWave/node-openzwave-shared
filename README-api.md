## API

Typical usage pattern is to 
1) load the addon with `require`, and create a new instance with your options
2) bind the `driver ready` event to capture the created driver object 
3) then, use `connect` to add a driver to your ZWave controller(s):

```js
// step 1: initialize
var zwaveModule = require("openzwave-shared")({
    Logging: false,     // disable file logging (OZWLog.txt)
    ConsoleOutput: true // enable console logging
});
// step 2: bind the driver to your home network
var home;
zwave.on('driver ready', function(homeid, drv) {
  console.log('scanning homeid=0x%s... drv=%j', homeid.toString(16), drv);
  home = drv;
});
// step 3: connect!
zwave.connect('/dev/ttyUSB0');
```

The default options are specified in `config/options.xml`. Please refer
[to the full list of OpenZWave options](https://github.com/OpenZWave/open-zwave/wiki/Config-Options)
for all the available options. If, for instance, you're using security devices
(e.g. door locks) then you should specify an encryption key.

The `drv` object emitted from `driver ready` is effectively a convenience wrapper around all the available commands exposed by the `OpenZWave::Manager` interface, hiding from you the necessity to pass the home ID on each OpenZWave::Manager API call. If you only got multiple ZWave controllers you'd better use a hash to keep references to all your drivers:

```js
var drivers;
home.on('driver ready', function(homeid, drv) {
  console.log('scanning homeid=0x%s... drv=%j', homeid.toString(16), drv);
  drivers[homeid] = drv;
});
```

The rest of the API is split into Functions and Events.  Messages from the
Z-Wave network are handled by `EventEmitter`, and you will need to listen for
specific events to correctly map the network. The most important one is the `driver ready` message emitted when you call `connect` to add a driver for your ZWave controller (usually a USB stick, but it can also be an extension board or a serial module). 

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

**Controlling zwave valueIDs** is usually done by passing an [OpenZWave::ValueID](http://openzwave.com/dev/classOpenZWave_1_1ValueID.html) as a Javascipt object or as 4 discrete integer arguments:
- 1: ZWave Node Id,
- 2: Command Class,
- 3: Instance and
- 4: Index

For example if Zwave Node #3 is a binary switch, to turn it on and off, use
command class 37:

```js
home.setValue(3, 37,  1,  0,  true);  // node 3: turn on
home.setValue(3, 37,  1,  0,  false); // node 3: turn off
```

Another example: if Zwave Node #5 is a dimmer, use class 38:

```js
home.setValue(5,  38,  1,  0, 50); // 1) passing each individual ValueID constituent:
home.setValue({ node_id:5, class_id: 38, instance:1, index:0}, 50); // 2) or a valueID object (emitted by ValueAdded event):

/*
 * Turn a binary switch on/off.
 */
home.setNodeOn(3); // node 3: switch ON
home.setNodeOff(3);// node 3: switch OFF

/*
 * Set a multi-level device to the specified level (between 0-99).
 * See warning below
 */
home.setLevel(5, 50); // node 5: dim to 50%
```

*WARNING: setNodeOn/Off/Level _don't work reliably with all devices_*, as they are
mere aliases to the BASIC command class. Not all devices support this. Please
consult your device's manual to see if it supports this command class.
The 'standard' way to control your devices is by `setValue` which is also the
_only_ way to control multi-instance devices, such as the Fibaro FGS-221
(double in-wall 2x1,5kw relay) for example:

```js
home.setValue(8, 37, 1, 0, true); // node 8: turn on 1st relay
home.setValue(8, 37, 1, 0, false);// node 8: turn off 1st relay
home.setValue(8, 37, 2, 0, true); // node 8: turn on 2nd relay
home.setValue(8, 37, 2, 0, false);// node 8: turn off 2nd relay
```
Useful documentation on [command classes can be found on MiCasaVerde website](http://wiki.micasaverde.com/index.php/ZWave_Command_Classes)

Writing to device metadata (stored on the device itself):

```js
home.setNodeLocation(nodeid, location);    // arbitrary location string
home.setNodeName(nodeid, name);            // arbitrary name string
```

Polling a device for changes (not all devices require this):

```js
home.enablePoll({valueId}, intensity);
home.disablePoll({valueId});
home.setPollInterval(intervalMillisecs) // set the polling interval in msec
home.getPollInterval();  // return the polling interval
home.setPollIntensity({valueId}, intensity); // Set the frequency of polling (0=none, 1=every time through the list, 2-every other time, etc)
home.getPollIntensity({valueId});
home.isPolled({valueId});
```

Association groups management:

```js
home.getNumGroups(nodeid);
home.getGroupLabel(nodeid, group);
home.getAssociations(nodeid, group);
home.getMaxAssociations(nodeid, group);
home.addAssociation(nodeid, group, target_nodeid);
home.removeAssociation(nodeid, group, target_nodeid);
```

Resetting the controller.  Calling `hardReset` will clear any associations, so use
carefully:

```js
home.hardReset();      // destructive! will wipe out all known configuration
home.softReset();      // non-destructive, just resets the chip
```

Scenes control:

```js
home.createScene(label); 	// create a scene and assign a label, return its numeric id.
home.removeScene(sceneId); // perform #GRExit
home.getScenes();			// get all scenes as an array
// add a zwave value to a scene
home.addSceneValue(sceneId, nodeId, commandclass, instance, index);
// remove a zwave value from a scene
home.removeSceneValue(sceneId, nodeId, commandclass, instance, index);
home.sceneGetValues(sceneId); // return array of values associated with this scene
home.activateScene(sceneId);  // The Show Must Go On...
```

ZWave network commands:

```js
home.healNetworkNode(nodeId, doReturnRoutes=false);
home.healNetwork();   // guru meditation
home.getNeighbors();
home.refreshNodeInfo(nodeid);
```

ZWave controller commands:

```js
// begin an async controller command on node1:
home.beginControllerCommand( "command name", highPower = false, node1_id, node2_id = null);  
// cancel controller command in progress
home.cancelControllerCommand();
// returns controller's node id
home.getControllerNodeId();
// returns static update controller node id
home.getSUCNodeId();
// is the OZW-managed controller the primary controller for this zwave network?
home.isPrimaryController();
// Query if the controller is a static update controller.
home.isStaticUpdateController();
// Query if the controller is using the bridge controller library.
home.isBridgeController();
// Get the version of the Z-Wave API library used by a controller.
home.getLibraryVersion();
// Get a string containing the Z-Wave API library type used by a controller
home.getLibraryTypeName();
//
home.getSendQueueCount();
```


### Configuration commands:

```js
home.requestAllConfigParams(nodeId);
home.requestConfigParam(nodeId, paramId);
home.setConfigParam(nodeId, paramId, paramValue, <sizeof paramValue>);
```
