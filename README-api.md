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
zwave.setValue(3, 37, 1, 0, true); // node 3: turn on (the hard way)
zwave.setValue(3, 37, 1, 0, false); // node 3: turn off (the hard way)

/*
 * Set arbitrary values.
 */
zwave.setValue(nodeid, commandclass, instance, index, value);
```

This is most useful for multi-instance devices, such as the Fibaro FGS-221 eg:
```js
zwave.setValue(8, 37, 1, 0, true); // node 8: turn on 1st relay
zwave.setValue(8, 37, 1, 0, false);// node 8: turn off 1st relay
zwave.setValue(8, 37, 2, 0, true); // node 8: turn on 2nd relay
zwave.setValue(8, 37, 2, 0, false);// node 8: turn off 2nd relay
```
Useful documentation on [command classes can be found on MiCasaVerde website](http://wiki.micasaverde.com/index.php/ZWave_Command_Classes)

Writing to device metadata (stored on the device itself):

```js
zwave.setLocation(nodeid, location);    // arbitrary location string
zwave.setName(nodeid, name);            // arbitrary name string
```

Polling a device for changes (not all devices require this):

```js
zwave.enablePoll(nodeid, commandclass);
zwave.disablePoll(nodeid, commandclass);
zwave.setPollInterval(nodeid, 
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

ZWave network management:
```js
zwave.hardReset();      // destructive! will wipe out all configuration stored in the chip
zwave.softReset();      // non-destructive, just resets the chip
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

### Controller commands
**BeginControllerCommand** is the most powerful command in the OpenZWave API, as it allows for performing management functions on the ZWave network.

    - The first argument is the command name, and its the only mandatory arg ,which can be any of the following:
    
```
["AddDevice"]
["CreateNewPrimary"]
["ReceiveConfiguration"]
["RemoveDevice"]
["RemoveFailedNode"]
["HasNodeFailed"]
["ReplaceFailedNode"]
["TransferPrimaryRole"]
["RequestNetworkUpdate"]
["RequestNodeNeighborUpdate"]
["AssignReturnRoute"]
["DeleteAllReturnRoutes"]
["SendNodeInformation"]
["ReplicationSend"]
["CreateButton"]
["DeleteButton"]
```


    - The second argument is a boolean ("highpower") - should be true/false
    - The third argument is the first ZWave node to be passed to the command (if applicable for the command)
    - The fourth argument is the second ZWave node to be passed to the command

For a full description of what of these controller commands mean, please see
[the official OpenZWave documentation](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#ac1a7f80c64bd9e5147be468b7b5a40d9)

### Configuration commands:
```js
zwave.requestAllConfigParams(nodeId);
zwave.requestConfigParam(nodeId, paramId);
zwave.setConfigParam(nodeId, paramId, paramValue, <sizeof paramValue>);

```
