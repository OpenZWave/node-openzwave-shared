## Example

The test program below connects to a Z-Wave network, scans for all nodes and
values, and prints out information about the network.  
**When the network has become ready**, the library will call 'scan complete' 
and the script will then issue a *beginControllerCommand* 
to the driver so as to add a new node to the ZWave network. 
Remember to hit `^C` to end the script.

```js
var ZWave = require('./lib/openzwave-shared.js');
var zwave = new ZWave({
	modpath: __dirname,
	consoleoutput: false,
	logging: false,
	saveconfig: false,
	driverattempts: 3,
	pollinterval: 500,
	suppressrefresh: true,
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
    console.log('====> scan complete, hit ^C to finish.');
    // Add a new device to the ZWave controller
    zwave.beginControllerCommand('AddDevice', true);
});

zwave.on('controller command', function(r,s) {
    console.log('controller commmand feedback: r=%d, s=%d',r,s);
});

zwave.connect('/dev/ttyUSB0');

process.on('SIGINT', function() {
    console.log('disconnecting...');
    zwave.disconnect();
    process.exit();
});
```

Sample output from this program:

```sh
$ nodejs test2.js 
initialising OpenZWave addon...
Could Not Open OZW Log File.
2015-09-24 00:41:00.447 Error, Cannot find a path to the configuration files at /home/ekarak/src/node-openzwave-shared/../deps/open-zwave/config/, Using /usr/local/etc/openzwave/ instead...
2015-09-24 00:41:00.447 Info, Reading /usr/local/etc/openzwave/options.xml for Options
2015-09-24 00:41:00.447 Warning, Failed to Parse options.xml: Failed to open file
~~~~ emit_cb:0x1515e30 isEmpty? 0
2015-09-24 00:41:00.447 Always, OpenZwave Version 1.3.938 Starting Up
2015-09-24 00:41:00.448 Info, mgr,     Added driver for controller /dev/ttyUSB0
2015-09-24 00:41:00.448 Info,   Opening controller /dev/ttyUSB0
2015-09-24 00:41:00.448 Info, Trying to open serial port /dev/ttyUSB0 (attempt 1)
2015-09-24 00:41:00.448 Info, Serial port /dev/ttyUSB0 opened (attempt 1)
2015-09-24 00:41:00.448 Detail, contrlr, Queuing (Command) FUNC_ID_ZW_GET_VERSION: 0x01, 0x03, 0x00, 0x15, 0xe9
2015-09-24 00:41:00.449 Detail, contrlr, Queuing (Command) FUNC_ID_ZW_MEMORY_GET_ID: 0x01, 0x03, 0x00, 0x20, 0xdc
2015-09-24 00:41:00.449 Detail, contrlr, Queuing (Command) FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES: 0x01, 0x03, 0x00, 0x05, 0xf9
2015-09-24 00:41:00.449 Detail, contrlr, Queuing (Command) FUNC_ID_SERIAL_API_GET_CAPABILITIES: 0x01, 0x03, 0x00, 0x07, 0xfb
2015-09-24 00:41:00.449 Detail, contrlr, Queuing (Command) FUNC_ID_ZW_GET_SUC_NODE_ID: 0x01, 0x03, 0x00, 0x56, 0xaa
2015-09-24 00:41:00.449 Detail, 
2015-09-24 00:41:00.449 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x15) - FUNC_ID_ZW_GET_VERSION: 0x01, 0x03, 0x00, 0x15, 0xe9
2015-09-24 00:41:00.451 Detail, contrlr,   Received: 0x01, 0x10, 0x01, 0x15, 0x5a, 0x2d, 0x57, 0x61, 0x76, 0x65, 0x20, 0x32, 0x2e, 0x34, 0x38, 0x00, 0x01, 0x98
2015-09-24 00:41:00.451 Detail, 
2015-09-24 00:41:00.451 Info, contrlr, Received reply to FUNC_ID_ZW_GET_VERSION:
2015-09-24 00:41:00.451 Info, contrlr,     Static Controller library, version Z-Wave 2.48
2015-09-24 00:41:00.451 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.451 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.451 Detail, 
2015-09-24 00:41:00.451 Detail, contrlr, Removing current message
2015-09-24 00:41:00.451 Detail, 
2015-09-24 00:41:00.451 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x20) - FUNC_ID_ZW_MEMORY_GET_ID: 0x01, 0x03, 0x00, 0x20, 0xdc
2015-09-24 00:41:00.453 Detail, contrlr,   Received: 0x01, 0x08, 0x01, 0x20, 0x00, 0xd8, 0xc6, 0x6b, 0x01, 0xa2
2015-09-24 00:41:00.453 Detail, 
2015-09-24 00:41:00.453 Info, contrlr, Received reply to FUNC_ID_ZW_MEMORY_GET_ID. Home ID = 0x00d8c66b.  Our node ID = 1
2015-09-24 00:41:00.453 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.453 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.453 Detail, 
2015-09-24 00:41:00.453 Detail, contrlr, Removing current message
2015-09-24 00:41:00.453 Detail, 
2015-09-24 00:41:00.453 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x05) - FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES: 0x01, 0x03, 0x00, 0x05, 0xf9
2015-09-24 00:41:00.454 Detail, contrlr,   Received: 0x01, 0x04, 0x01, 0x05, 0x14, 0xeb
2015-09-24 00:41:00.454 Detail, 
2015-09-24 00:41:00.454 Info, contrlr, Received reply to FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES:
2015-09-24 00:41:00.454 Info, contrlr,     There is a SUC ID Server (SIS) in this network.
2015-09-24 00:41:00.454 Info, contrlr,     The PC controller is an inclusion static update controller (SUC).
2015-09-24 00:41:00.454 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.454 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.454 Detail, 
2015-09-24 00:41:00.454 Detail, contrlr, Removing current message
2015-09-24 00:41:00.454 Detail, 
2015-09-24 00:41:00.454 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x07) - FUNC_ID_SERIAL_API_GET_CAPABILITIES: 0x01, 0x03, 0x00, 0x07, 0xfb
2015-09-24 00:41:00.459 Detail, contrlr,   Received: 0x01, 0x2b, 0x01, 0x07, 0x02, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0xfe, 0x80, 0xfe, 0x88, 0x0f, 0x00, 0x00, 0x00, 0xfb, 0x9f, 0x7f, 0x80, 0x07, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19
2015-09-24 00:41:00.459 Detail, 
2015-09-24 00:41:00.459 Info, contrlr,  Received reply to FUNC_ID_SERIAL_API_GET_CAPABILITIES
2015-09-24 00:41:00.459 Info, contrlr,     Serial API Version:   2.82
2015-09-24 00:41:00.459 Info, contrlr,     Manufacturer ID:      0x0000
2015-09-24 00:41:00.459 Info, contrlr,     Product Type:         0x0001
2015-09-24 00:41:00.459 Info, contrlr,     Product ID:           0x0001
2015-09-24 00:41:00.459 Detail, contrlr, Queuing (Command) FUNC_ID_ZW_GET_RANDOM: 0x01, 0x04, 0x00, 0x1c, 0x20, 0xc7
2015-09-24 00:41:00.459 Detail, contrlr, Queuing (Command) FUNC_ID_SERIAL_API_GET_INIT_DATA: 0x01, 0x03, 0x00, 0x02, 0xfe
2015-09-24 00:41:00.459 Detail, contrlr, Queuing (Command) FUNC_ID_SERIAL_API_SET_TIMEOUTS: 0x01, 0x05, 0x00, 0x06, 0x64, 0x0f, 0x97
2015-09-24 00:41:00.459 Detail, contrlr, Queuing (Command) FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION: 0x01, 0x07, 0x00, 0x03, 0x01, 0x02, 0x01, 0x00, 0xf9
2015-09-24 00:41:00.459 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.459 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.459 Detail, 
2015-09-24 00:41:00.459 Detail, contrlr, Removing current message
2015-09-24 00:41:00.459 Detail, 
2015-09-24 00:41:00.459 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x56) - FUNC_ID_ZW_GET_SUC_NODE_ID: 0x01, 0x03, 0x00, 0x56, 0xaa
2015-09-24 00:41:00.461 Detail, contrlr,   Received: 0x01, 0x04, 0x01, 0x56, 0x01, 0xad
2015-09-24 00:41:00.461 Detail, 
2015-09-24 00:41:00.461 Info, contrlr, Received reply to GET_SUC_NODE_ID.  Node ID = 1
2015-09-24 00:41:00.461 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.461 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.461 Detail, 
2015-09-24 00:41:00.461 Detail, contrlr, Removing current message
2015-09-24 00:41:00.461 Detail, 
2015-09-24 00:41:00.461 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x1c) - FUNC_ID_ZW_GET_RANDOM: 0x01, 0x04, 0x00, 0x1c, 0x20, 0xc7
2015-09-24 00:41:00.513 Detail, contrlr,   Received: 0x01, 0x25, 0x01, 0x1c, 0x01, 0x20, 0xe0, 0x2a, 0x03, 0x3b, 0x1c, 0x21, 0xb2, 0x72, 0x72, 0x99, 0x65, 0x45, 0x7f, 0x7a, 0xc0, 0xac, 0x70, 0xf4, 0xb2, 0x43, 0x11, 0x16, 0x28, 0x5f, 0x07, 0xc3, 0x1e, 0x9e, 0x7b, 0xc2, 0x55, 0x04, 0xe2
2015-09-24 00:41:00.513 Detail, 
2015-09-24 00:41:00.514 Info, contrlr, Received reply to FUNC_ID_ZW_GET_RANDOM: true
2015-09-24 00:41:00.514 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.514 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.514 Detail, 
2015-09-24 00:41:00.514 Detail, contrlr, Removing current message
2015-09-24 00:41:00.514 Detail, 
2015-09-24 00:41:00.514 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x02) - FUNC_ID_SERIAL_API_GET_INIT_DATA: 0x01, 0x03, 0x00, 0x02, 0xfe
2015-09-24 00:41:00.562 Detail, contrlr,   Received: 0x01, 0x25, 0x01, 0x02, 0x04, 0x08, 0x1d, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0xc9
2015-09-24 00:41:00.562 Detail, 
2015-09-24 00:41:00.562 Info, mgr,     Driver with Home ID of 0x00d8c66b is now ready.
2015-09-24 00:41:00.562 Info, 
2015-09-24 00:41:00.562 Info, contrlr, Received reply to FUNC_ID_SERIAL_API_GET_INIT_DATA:
2015-09-24 00:41:00.562 Info, contrlr,     Node 001 - Known
2015-09-24 00:41:00.562 Detail, Node001, AdvanceQueries queryPending=0 queryRetries=0 queryStage=Probe1 live=1
2015-09-24 00:41:00.563 Detail, Node001, QueryStage_Probe1
2015-09-24 00:41:00.563 Detail, Node001, QueryStage_Associations
2015-09-24 00:41:00.563 Detail, Node001, QueryStage_Neighbors
2015-09-24 00:41:00.563 Detail, contrlr, Requesting routing info (neighbor list) for Node 1
2015-09-24 00:41:00.563 Detail, Node001, Queuing (Command) Get Routing Info (Node=1): 0x01, 0x07, 0x00, 0x80, 0x01, 0x00, 0x00, 0x03, 0x7a
2015-09-24 00:41:00.563 Detail, Node001, Queuing (Query) Query Stage Complete (Neighbors)
2015-09-24 00:41:00.563 Info, contrlr,     Node 002 - Known
2015-09-24 00:41:00.563 Detail, Node002, AdvanceQueries queryPending=0 queryRetries=0 queryStage=Probe live=1
2015-09-24 00:41:00.563 Detail, Node002, QueryStage_Probe
2015-09-24 00:41:00.563 Info, Node002, NoOperation::Set - Routing=true
2015-09-24 00:41:00.563 Detail, Node002, Queuing (NoOp) NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x01, 0xc1
2015-09-24 00:41:00.563 Detail, Node002, Queuing (Query) Query Stage Complete (Probe)
2015-09-24 00:41:00.563 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.563 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.563 Detail, 
2015-09-24 00:41:00.563 Detail, contrlr, Removing current message
2015-09-24 00:41:00.563 Detail, 
2015-09-24 00:41:00.563 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x06) - FUNC_ID_SERIAL_API_SET_TIMEOUTS: 0x01, 0x05, 0x00, 0x06, 0x64, 0x0f, 0x97
scanning homeid=0xd8c66b...
2015-09-24 00:41:00.565 Detail, contrlr,   Received: 0x01, 0x05, 0x01, 0x06, 0x96, 0x0f, 0x64
2015-09-24 00:41:00.565 Detail, 
2015-09-24 00:41:00.565 Info, contrlr, Received reply to FUNC_ID_SERIAL_API_SET_TIMEOUTS
2015-09-24 00:41:00.565 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:00.565 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:00.565 Detail, 
2015-09-24 00:41:00.565 Detail, contrlr, Removing current message
2015-09-24 00:41:00.565 Detail, 
2015-09-24 00:41:00.565 Info, contrlr, Sending (Command) message (Callback ID=0x00, Expected Reply=0x00) - FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION: 0x01, 0x07, 0x00, 0x03, 0x01, 0x02, 0x01, 0x00, 0xf9
2015-09-24 00:41:00.566 Detail, contrlr, Removing current message
2015-09-24 00:41:00.566 Detail, 
2015-09-24 00:41:00.566 Info, Node001, Sending (Command) message (Callback ID=0x00, Expected Reply=0x80) - Get Routing Info (Node=1): 0x01, 0x07, 0x00, 0x80, 0x01, 0x00, 0x00, 0x03, 0x7a
2015-09-24 00:41:00.571 Detail, Node001,   Received: 0x01, 0x20, 0x01, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c
2015-09-24 00:41:00.571 Detail, 
2015-09-24 00:41:00.571 Info, Node001, Received reply to FUNC_ID_ZW_GET_ROUTING_INFO
2015-09-24 00:41:00.571 Info, Node001,     Neighbors of this node are:
2015-09-24 00:41:00.571 Info, Node001,     Node 2
2015-09-24 00:41:00.571 Detail, Node001,   Expected reply was received
2015-09-24 00:41:00.571 Detail, Node001,   Message transaction complete
2015-09-24 00:41:00.571 Detail, 
2015-09-24 00:41:00.571 Detail, Node001, Removing current message
2015-09-24 00:41:00.571 Detail, 
2015-09-24 00:41:00.571 Info, Node002, Sending (NoOp) message (Callback ID=0x01, Expected Reply=0x13) - NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x01, 0xc1
2015-09-24 00:41:00.576 Detail, Node002,   Received: 0x01, 0x04, 0x01, 0x13, 0x01, 0xe8
2015-09-24 00:41:00.576 Detail, Node002,   ZW_SEND_DATA delivered to Z-Wave stack
2015-09-24 00:41:00.797 Detail, Node002,   Received: 0x01, 0x05, 0x00, 0x13, 0x01, 0x01, 0xe9
2015-09-24 00:41:00.797 Detail, Node002,   ZW_SEND_DATA Request with callback ID 0x01 received (expected 0x01)
2015-09-24 00:41:00.797 Info, Node002, WARNING: ZW_SEND_DATA failed. No ACK received - device may be asleep.
2015-09-24 00:41:00.797 Warning, Node002, WARNING: Device is not a sleeping node.
2015-09-24 00:41:00.797 Info, Node002, QueryStageRetry stage Probe requested stage Probe max 3 retries 0 pending 1
2015-09-24 00:41:00.797 Detail, Node002,   Expected reply was received
2015-09-24 00:41:00.797 Detail, Node002,   Message transaction complete
2015-09-24 00:41:00.797 Detail, 
2015-09-24 00:41:00.797 Detail, Node002, Removing current message
2015-09-24 00:41:00.797 Detail, Node001, Query Stage Complete (Neighbors)
2015-09-24 00:41:00.797 Detail, Node001, AdvanceQueries queryPending=0 queryRetries=0 queryStage=Session live=1
2015-09-24 00:41:00.797 Detail, Node001, QueryStage_Session
2015-09-24 00:41:00.797 Detail, Node001, QueryStage_Dynamic
2015-09-24 00:41:00.797 Detail, Node001, QueryStage_Configuration
2015-09-24 00:41:00.797 Detail, Node001, QueryStage_Complete
2015-09-24 00:41:00.797 Warning, CheckCompletedNodeQueries m_allNodesQueried=0 m_awakeNodesQueried=0
2015-09-24 00:41:00.797 Warning, CheckCompletedNodeQueries all=0, deadFound=0 sleepingOnly=0
node2: nop
2015-09-24 00:41:00.797 Detail, Node002, Query Stage Complete (Probe)
2015-09-24 00:41:00.797 Detail, Node002, AdvanceQueries queryPending=0 queryRetries=1 queryStage=Probe live=1
2015-09-24 00:41:00.797 Detail, Node002, QueryStage_Probe
2015-09-24 00:41:00.797 Info, Node002, NoOperation::Set - Routing=true
2015-09-24 00:41:00.798 Detail, Node002, Queuing (NoOp) NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x02, 0xc2
2015-09-24 00:41:00.798 Detail, Node002, Queuing (Query) Query Stage Complete (Probe)
2015-09-24 00:41:00.798 Detail, 
2015-09-24 00:41:00.798 Info, Node002, Sending (NoOp) message (Callback ID=0x02, Expected Reply=0x13) - NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x02, 0xc2
node1: Zensys, Controller
node1: name="", type="Static PC Controller", location=""
node1: class 32
node1:   Basic=0
2015-09-24 00:41:00.806 Detail, Node002,   Received: 0x01, 0x04, 0x01, 0x13, 0x01, 0xe8
2015-09-24 00:41:00.806 Detail, Node002,   ZW_SEND_DATA delivered to Z-Wave stack
2015-09-24 00:41:01.103 Detail, Node002,   Received: 0x01, 0x05, 0x00, 0x13, 0x02, 0x01, 0xea
2015-09-24 00:41:01.103 Detail, Node002,   ZW_SEND_DATA Request with callback ID 0x02 received (expected 0x02)
2015-09-24 00:41:01.103 Info, Node002, WARNING: ZW_SEND_DATA failed. No ACK received - device may be asleep.
2015-09-24 00:41:01.103 Warning, Node002, WARNING: Device is not a sleeping node.
2015-09-24 00:41:01.103 Info, Node002, QueryStageRetry stage Probe requested stage Probe max 3 retries 1 pending 1
2015-09-24 00:41:01.103 Detail, Node002,   Expected reply was received
2015-09-24 00:41:01.103 Detail, Node002,   Message transaction complete
2015-09-24 00:41:01.103 Detail, 
2015-09-24 00:41:01.103 Detail, Node002, Removing current message
2015-09-24 00:41:01.103 Detail, Node002, Query Stage Complete (Probe)
2015-09-24 00:41:01.103 Detail, Node002, AdvanceQueries queryPending=0 queryRetries=2 queryStage=Probe live=1
2015-09-24 00:41:01.103 Detail, Node002, QueryStage_Probe
2015-09-24 00:41:01.103 Info, Node002, NoOperation::Set - Routing=true
2015-09-24 00:41:01.103 Detail, Node002, Queuing (NoOp) NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x03, 0xc3
2015-09-24 00:41:01.103 Detail, Node002, Queuing (Query) Query Stage Complete (Probe)
2015-09-24 00:41:01.103 Detail, 
2015-09-24 00:41:01.103 Info, Node002, Sending (NoOp) message (Callback ID=0x03, Expected Reply=0x13) - NoOperation_Set (Node=2): 0x01, 0x09, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x25, 0x03, 0xc3
node2: nop
2015-09-24 00:41:01.108 Detail, Node002,   Received: 0x01, 0x04, 0x01, 0x13, 0x01, 0xe8
2015-09-24 00:41:01.108 Detail, Node002,   ZW_SEND_DATA delivered to Z-Wave stack
2015-09-24 00:41:01.489 Detail, Node002,   Received: 0x01, 0x05, 0x00, 0x13, 0x03, 0x01, 0xeb
2015-09-24 00:41:01.489 Detail, Node002,   ZW_SEND_DATA Request with callback ID 0x03 received (expected 0x03)
2015-09-24 00:41:01.489 Info, Node002, WARNING: ZW_SEND_DATA failed. No ACK received - device may be asleep.
2015-09-24 00:41:01.489 Warning, Node002, WARNING: Device is not a sleeping node.
2015-09-24 00:41:01.489 Error, Node002, ERROR: node presumed dead
2015-09-24 00:41:01.489 Warning, CheckCompletedNodeQueries m_allNodesQueried=0 m_awakeNodesQueried=0
2015-09-24 00:41:01.489 Warning, CheckCompletedNodeQueries all=1, deadFound=1 sleepingOnly=1
2015-09-24 00:41:01.489 Info,          Node query processing complete except for dead nodes.
2015-09-24 00:41:01.489 Info, Node002, QueryStageRetry stage Probe requested stage Probe max 3 retries 2 pending 1
2015-09-24 00:41:01.489 Detail, Node002,   Expected reply was received
2015-09-24 00:41:01.489 Detail, Node002,   Message transaction complete
2015-09-24 00:41:01.489 Detail, 
2015-09-24 00:41:01.489 Detail, Node002, Removing current message
2015-09-24 00:41:01.489 Detail, Node002, Query Stage Complete (Probe)
2015-09-24 00:41:01.489 Detail, Node002, AdvanceQueries queryPending=0 queryRetries=0 queryStage=Probe live=0
node2: nop
====> scan complete, hit ^C to finish.
2015-09-24 00:41:01.490 Detail, contrlr, Queuing (Controller) Add Device
2015-09-24 00:41:01.490 Info, Add Device
node2: node dead
2015-09-24 00:41:01.490 Detail, contrlr, Queuing (Command) AddDevice: 0x01, 0x05, 0x00, 0x4a, 0x81, 0x04, 0x35
2015-09-24 00:41:01.490 Detail, 
2015-09-24 00:41:01.490 Info, contrlr, Sending (Command) message (Callback ID=0x04, Expected Reply=0x4a) - AddDevice: 0x01, 0x05, 0x00, 0x4a, 0x81, 0x04, 0x35
2015-09-24 00:41:01.493 Detail, contrlr,   Received: 0x01, 0x07, 0x00, 0x4a, 0x04, 0x01, 0x00, 0x00, 0xb7
2015-09-24 00:41:01.493 Detail, 
2015-09-24 00:41:01.493 Info, contrlr, FUNC_ID_ZW_ADD_NODE_TO_NETWORK:
2015-09-24 00:41:01.493 Info, contrlr, ADD_NODE_STATUS_LEARN_READY
2015-09-24 00:41:01.493 Detail, contrlr,   Expected callbackId was received
2015-09-24 00:41:01.493 Detail, contrlr,   Expected reply was received
2015-09-24 00:41:01.493 Detail, contrlr,   Message transaction complete
2015-09-24 00:41:01.493 Detail, 
2015-09-24 00:41:01.493 Detail, contrlr, Removing current message
2015-09-24 00:41:01.493 Info, WriteNextMsg Controller nothing to do
controller commmand feedback: r=4, s=0
scan complete, hit ^C to finish.
```

Remove `2>/dev/null` to get verbose output of all incoming notification types
and additional debug information.
