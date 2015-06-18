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

zwave.on('polling enabled', function(nodeid) {
	console.log('node%d: polling ENABLED', nodeid);
});
zwave.on('polling disabled', function(nodeid) {
	console.log('node%d: polling DISABLED', nodeid);
});

var notificationCodes = {
	0: 'message complete',
	1: 'timeout',
	2: 'nop',
	3: 'node awake',
	4: 'node sleep',
	5: 'node dead (Undead Undead Undead)',
	6: 'node alive',
};
zwave.on('notification', function(nodeid, notif) {
	console.log('node%d: %s', nodeid, notificationCodes[notif]);
});

var ctrlState = {
	0: 'No command in progress',
	1: 'The command is starting',
	2: 'The command was cancelled',
	3: 'Command invocation had error(s) and was aborted',
	4: 'Controller is waiting for a user action',
	5: 'Controller command is on a sleep queue wait for device',
	6: 'The controller is communicating with the other device to carry out the command',
	7: 'The command has completed successfully',
	8: 'The command has failed',
	9: 'The controller thinks the node is OK',
	10: 'The controller thinks the node has failed',
};
var ctrlError = {
	0: 'No error',
	1: 'ButtonNotFound',
	2: 'NodeNotFound',
	3: 'NotBridge',
	4: 'NotSUC',
	5: 'NotSecondary',
	6: 'NotPrimary',
	7: 'IsPrimary',
	8: 'NotFound',
	9: 'Busy',
	10: 'Failed',
	11: 'Disabled',
	12: 'Overflow',
}
zwave.on('controller command', function (state, error) {
	console.log('controller command feedback: state:%d error:%d', ctrlState[state], ctrlError[error]);
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
