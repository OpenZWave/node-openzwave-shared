## API

> This library is as similar as possible to the [C++ OpenZwave::Manager API](http://www.openzwave.com/dev/classOpenZWave_1_1Manager.html) for Node.js. Please note that this is not always possible to have the _exact_ same behaviour as in the C++ library.

Start by loading the addon with `require` and then create a new instance of the addon. You can also pass in an optional object specifying any desired option overrides:
```js
var OZW = require('openzwave-shared');
var zwave = new OZW();
// OR pass extra options
var zwave = new OZW({
    Logging: false,     // disable file logging (OZWLog.txt)
    ConsoleOutput: true // enable console logging
});
```

The underlying OpenZWave library is a effectively a singleton so you can simply do:
```js
var zwave = require("openzwave-shared")({
  ConsoleOutput: false
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

**Controlling zwave valueIDs** is usually done by passing the ValueID as a
Javascipt object or as 4 discrete integer arguments:
- 1: ZWave Node Id,
- 2: Command Class,
- 3: Instance and
- 4: Index

Some examples:

```js
// if Zwave Node #3 is a binary switch, to turn it on and off, use command class 37
zwave.setValue(3, 37,  1,  0,  true);  // node 3: turn on
zwave.setValue(3, 37,  1,  0,  false); // node 3: turn off
zwave.setValue({ node_id:3, class_id: 37, instance:1, index:0}, false); // the same turn-off command using an object
// if Zwave Node #5 is a dimmer, use class 38:
zwave.setValue(5,  38,  1,  0, 50); // 1) passing each individual ValueID constituent:
zwave.setValue({ node_id:5, class_id: 38, instance:1, index:0}, 50); // 2) or a valueID object (emitted by ValueAdded event):
// The 'standard' way to control your devices is by `setValue` which is also the
// _only_ way to control multi-instance devices, such as the Fibaro FGS-221
// (double in-wall 2x1,5kw relay) for example:
zwave.setValue(8, 37, 1, 0, true); // node 8: turn on 1st relay
zwave.setValue(8, 37, 1, 0, false);// node 8: turn off 1st relay
zwave.setValue(8, 37, 2, 0, true); // node 8: turn on 2nd relay
zwave.setValue(8, 37, 2, 0, false);// node 8: turn off 2nd relay
```

Useful documentation on [command classes can be found on MiCasaVerde website](http://wiki.micasaverde.com/index.php/ZWave_Command_Classes)

Writing to device metadata (stored in the `zwcfg-<homeId>.xml` file, under `UserPath`):
```js
zwave.setNodeLocation(nodeid, location);    // arbitrary location string
zwave.setNodeName(nodeid, name);            // arbitrary name string
```

Polling a device for changes (not all devices require this):
```js
zwave.enablePoll({valueId}, intensity);
zwave.disablePoll({valueId});
zwave.setPollInterval(intervalMillisecs) // set the polling interval in msec
zwave.getPollInterval();  // return the polling interval
zwave.setPollIntensity({valueId}, intensity); // Set the frequency of polling (0=none, 1=every time through the list, 2-every other time, etc)
zwave.getPollIntensity({valueId});
zwave.isPolled({valueId});
```

Association groups management:
```js
zwave.getNumGroups(nodeid);
zwave.getGroupLabel(nodeid, group);
zwave.getAssociations(nodeid, group);
zwave.getMaxAssociations(nodeid, group);
zwave.addAssociation(nodeid, group, target_nodeid);
zwave.removeAssociation(nodeid, group, target_nodeid);
zwave.isGroupMultiInstance(nodeid, group);
```

Resetting the controller.  Calling `hardReset` will clear any associations, so use
carefully:
```js
zwave.hardReset();      // destructive! will wipe out all known configuration
zwave.softReset();      // non-destructive, just resets the chip
```

ZWave network commands:
```js
zwave.healNetworkNode(nodeId, doReturnRoutes=false);
zwave.healNetwork();   // guru meditation
zwave.getNodeNeighbors();
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

You can always refer to [the official OpenZWave::Manager API](http://www.openzwave.com/dev/classOpenZWave_1_1Manager.html) for more details on calling Manager methods. The aim of this wrapper is to provide a 1-to-1 mapping to all available methods, with the only change here being that the first letter of each method is downcased (eg. `RequestNodeInfo` in C++ is named `requestNodeInfo` in Javascript)
