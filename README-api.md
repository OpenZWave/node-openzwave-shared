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
zwave.setPollInterval(nodeid http://www.openzwave.com/dev/classOpenZWave_1_1Manager.html#ac7032ff3978d645b6dcd3284a8055207
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
zwave.healNetworkNode (nodeId, doRR);
zwave.healNetwork();   // guru meditation
zwave.getNeighbors();
```

ZWave controller commands: (*work in progress!!!*)
```js
zwave.beginControllerCommand( "command", node1_id, node2_id );
zwave,cancelControllerCommand();
```
command can be "AddDevice", "RemoveDevice", "ReplaceFailedNode" etc.
see http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#ac1a7f80c64bd9e5147be468b7b5a40d9 for a list of controller commands
