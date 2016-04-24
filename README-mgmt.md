#### ZWave management commands

**BeginControllerCommand** was until version 1.3 the only management call to the
OpenZWave API. In Version 1.3 new functionality for *secure devices* was introduced
and this also meant deprecating this particular call. In the newer versions of
the library, there are discrete management API calls for each of the management
functions, and openzwave-shared adapts to the underlying OZW version, exposing
the underlying API calls.

## Security enabled (OpenZWave >= 1.3)
If your OZW version is 1.3 or greater, then there's a whole bunch of new calls,
which effectively replace the old `BeginControllerCommand`.

-  zwave.addNode(doSecurity: boolean):
  Add a new device or controller with/without security. This is usually followed
  by starting the pair/unpair process on the device (eg. pressing a special
  inclusion button or quickly pressing the manual operation button three times)

-  zwave.removeNode():
  Remove a device or controller from the Z-Wave network. This is usually followed
  by starting the pair/unpair process on the device (eg by some inclusion button)

-  zwave.removeFailedNode(nodeid):
  Remove a *specific* failed node from the controller's memory.

-  zwave.hasNodeFailed(nodeid)
  Check whether a node is in the controller's failed nodes list.

-  zwave.requestNodeNeighborUpdate(nodeid)
  Get a node to rebuild its neighbour list.

-  zwave.assignReturnRoute(nodeid)
  Assign a network return routes to a device.

-  zwave.deleteAllReturnRoutes(nodeid)
  Delete all return routes from a device.

-  zwave.sendNodeInformation(nodeid)
  Send a NIF (node information frame)

-  zwave.createNewPrimary())
  Add a new controller to the Z-Wave network. Used when old primary fails. Requires SUC.

-  zwave.receiveConfiguration()
    Receive Z-Wave network configuration information from another controller.

-  zwave.replaceFailedNode(nideid)
  Replace a non-responding node with another. The node must be in the controller's list of failed nodes for this command to succeed.

-  zwave.transferPrimaryRole()
  Make a different controller the primary.

-  zwave.requestNetworkUpdate(nodeid)
  Request network information from the SUC/SIS.

-  zwave.replicationSend(nodeid)
  Send information from primary to secondary

-  zwave.createButton(nodeid, buttonid)
  Create an id that tracks handheld button presses

-  zwave.deleteButton(nodeid, buttonid)
  Delete id that tracks handheld button presses



## Legacy mode (`BeginControllerCommand`)
If your OZW version is 1.2 or earlier, use `beginControllerCommand` for doing
all management stuff with your ZWave network. This command has the following format:

`zwave.beginControllerCommand("command", highpower, node1, node2)`

  - The first argument is the command name, and its the *only mandatory*, which can be any of the following:

```
"AddDevice"
"CreateNewPrimary"
"ReceiveConfiguration"
"RemoveDevice"
"RemoveFailedNode"
"HasNodeFailed"
"ReplaceFailedNode"
"TransferPrimaryRole"
"RequestNetworkUpdate"
"RequestNodeNeighborUpdate"
"AssignReturnRoute"
"DeleteAllReturnRoutes"
"SendNodeInformation"
"ReplicationSend"
"CreateButton"
"DeleteButton"
```

    - The second argument ("highpower") is a boolean and tells OpenZWave to use low or high power mode - should be true/false. You shouldn't use high power for inclusions though.
    - The third argument is the first ZWave node to be passed to the command (if applicable for the command)
    - The fourth argument is the second ZWave node to be passed to the command

For a full description of what of these controller commands mean, please see
[the official OpenZWave documentation](http://www.openzwave.com/dev/classOpenZWave_1_1Driver.html#ac1a7f80c64bd9e5147be468b7b5a40d9)
