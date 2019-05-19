var ZWave = require('./lib/openzwave-shared.js');
var os = require('os');

var zwave = new ZWave({
  ConsoleOutput: true
});

zwavedriverpaths = {
  "darwin": '/dev/cu.usbmodem1411',
  "linux": '/dev/ttyUSB0',
  "windows": '\\\\.\\COM3'
}

var nodes = [];
var homeid = null;
zwave.on('driver ready', function(home_id) {
  homeid = home_id;
  console.log('scanning homeid=0x%s...', homeid.toString(16));
});

zwave.on('driver failed', function() {
  console.log('failed to start driver');
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

zwave.on('node event', function(nodeid, data) {
  console.log('node%d event: Basic set %d', nodeid, data);
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
      console.log('node%d:   %s=%s', nodeid, values[idx]['label'], values[
        idx]['value']);
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
  console.log('====> scan complete');
  // set dimmer node 5 to 50%
  //    zwave.setValue(5,38,1,0,50);
  //zwave.setValue({node_id:5,	class_id: 38,	instance:1,	index:0}, 50 );
  zwave.requestAllConfigParams(3);
});

zwave.on('controller command', function(n, rv, st, msg) {
  console.log(
    'controller commmand feedback: %s node==%d, retval=%d, state=%d', msg,
    n, rv, st);
});

console.log("connecting to " + zwavedriverpaths[os.platform()]);
zwave.connect(zwavedriverpaths[os.platform()]);

process.on('SIGINT', function() {
  console.log('disconnecting...');
  zwave.disconnect(zwavedriverpaths[os.platform()]);
  process.exit();
});
