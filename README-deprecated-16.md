## Usage of deprecated API calls in OpenZWave 1.6

There are numerous changes in the upcoming 1.6 release. This document describes these changes

### SetNodeOn/Off, SetLevel
Just define `OPENZWAVE_DEPRECATED` preprocessor symbol to have these included in the build:

```
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



## Scenes control:
Just define `OPENZWAVE_SCENES` preprocessor symbol to have these included in the build:

```js
zwave.createScene(label); 	// create a scene and assign a label, return its numeric id.
zwave.removeScene(sceneId); // perform #GRExit
zwave.getScenes();			// get all scenes as an array
// add a zwave value to a scene
zwave.addSceneValue(sceneId, nodeId, commandclass, instance, index, value);
zwave.addSceneValue(sceneId, { node_id:5, class_id: 38, instance:1, index:0}, 50); // Seconds arg can be a valueID object (emitted by ValueAdded event):
// remove a zwave value from a scene
zwave.removeSceneValue(sceneId, nodeId, commandclass, instance, index);
zwave.sceneGetValues(sceneId); // return array of values associated with this scene
zwave.activateScene(sceneId);  // The Show Must Go On...
```
