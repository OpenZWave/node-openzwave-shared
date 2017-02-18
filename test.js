/*
 * OpenZWave test program.
 */

var OpenZWave = require('./lib/openzwave-shared.js');
var os = require('os');

var zwave = new OpenZWave({
	ConsoleOutput: false,
	Logging: false,
	SaveConfiguration: false,
	DriverMaxAttempts: 3,
	PollInterval: 500,
	SuppressValueRefresh: true,
});
var nodes = [];

zwave.on('connected', function(homeid) {
	console.log('=================== CONNECTED! ====================');
});

zwave.on('driver ready', function(homeid) {
	console.log('=================== DRIVER READY! ====================');
	console.log('scanning homeid=0x%s...', homeid.toString(16));
});

zwave.on('driver failed', function() {
	console.log('failed to start driver');
	zwave.disconnect();
	process.exit();
});

zwave.on('node added', function(nodeid) {
	console.log('=================== NODE ADDED! ====================');
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
		nodeinfo.manufacturer ? nodeinfo.manufacturer : 'id=' + nodeinfo.manufacturerid,
		nodeinfo.product ? nodeinfo.product : 'product=' + nodeinfo.productid +
		', type=' + nodeinfo.producttype);
	console.log('node%d: name="%s", type="%s", location="%s"', nodeid,
		nodeinfo.name,
		nodeinfo.type,
		nodeinfo.loc);
	for (var comclass in nodes[nodeid]['classes']) {
		switch (comclass) {
			case 0x25: // COMMAND_CLASS_SWITCH_BINARY
			case 0x26: // COMMAND_CLASS_SWITCH_MULTILEVEL
				zwave.enablePoll(nodeid, comclass);
				break;
		}
		var values = nodes[nodeid]['classes'][comclass];
		console.log('node%d: class %d', nodeid, comclass);
		for (var idx in values)
			console.log('node%d:   %s=%s', nodeid, values[idx]['label'], values[idx][
				'value'
			]);
	}
});

zwave.on('notification', function(nodeid, notif, help) {
	console.log('node%d: notification(%d): %s', nodeid, notif, help);
});

zwave.on('scan complete', function() {
	console.log('scan complete, hit ^C to finish.');
});

function do_disconnect() {
	console.log('\n\n== Driver Statistics: %j',
		zwave.getDriverStatistics());
	Object.keys(nodes).forEach(function(nodeid) {
		console.log('== Node %d neighbors: %j',
			nodeid, zwave.getNodeNeighbors(nodeid));
		console.log('== Node %d Statistics: %j',
			nodeid, zwave.getNodeStatistics(nodeid));
	});
	console.log('disconnecting...');
	zwave.disconnect(zwp);
	process.exit();
}

var zwavedriverpaths = {
	"darwin": '/dev/cu.usbmodem1411',
	"linux": '/dev/ttyUSB0',
	"windows": '\\\\.\\COM3'
}
var zwp = zwavedriverpaths[os.platform()];
console.log("connecting to %s",zwp);
zwave.connect(zwp);
process.on('SIGINT', do_disconnect);
setTimeout(do_disconnect, 3000);
