#### ZWave management commands

**BeginControllerCommand** was until version 1.3 the only management call to the
OpenZWave API. In Version 1.3 new functionality for *secure devices* was introduced
and this also meant deprecating this particular call. In the newer versions of
the library, there are discrete management API calls for each of the management
functions, and openzwave-shared adapts to the underlying OZW version, exposing
the underlying API calls.

## Security mode
If your OZW version is 1.3 or greater, then there's a whole bunch of new calls

  zwave.addNode()
  zwave.removeNode()
  zwave.RemoveFailedNode)
  zwave.HasNodeFailed)
  zwave.RequestNodeNeighborUpdate)
  zwave.AssignReturnRoute)
  zwave.DeleteAllReturnRoutes)
  zwave.SendNodeInformation)
  zwave.CreateNewPrimary)
  zwave.ReceiveConfiguration)
  zwave.ReplaceFailedNode)
  zwave.TransferPrimaryRole)
  zwave.RequestNetworkUpdate)
  zwave.ReplicationSend)
  zwave.CreateButton)
  zwave.DeleteButton)


## Legacy mode (`BeginControllerCommand`)
If your OZW version is 1.2 or earlier, use `BeginControllerCommand` for doing
all management stuff with your ZWave network. This
as it allows for performing management functions on the ZWave network.
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
