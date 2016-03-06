## Example

The test program below connects to a Z-Wave network, scans for all nodes and
values, and prints out information about the network.

**When the network has become ready**, the library will call 'scan complete'
and the script will then 1) issue a `setValue` command to set a dimmer (node 5)
at 50%, and then issue a command to begin the inclusion process for a new zwave
device. This means calling `beginControllerCommand` OR `addNode` depending on
which version of the OpenZWave API you've linked against.

Remember to hit `^C` to end the script.

```js
var ZWave = require('openzwave-shared');
var zwave = new ZWave();

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
    // set dimmer node 5 to 50%
    //zwave.setValue(5,38,1,0,50);
    zwave.setValue( {node_id:5, class_id: 38, instance:1, index:0}, 50);
    // Add a new device to the ZWave controller
    if (zwave.hasOwnProperty('beginControllerCommand')) {
      // using legacy mode (OpenZWave version < 1.3) - no security
      zwave.beginControllerCommand('AddDevice', true);
    } else {
      // using new security API
      // set this to 'true' for secure devices eg. door locks
      zwave.addNode(false);
    }
});

zwave.on('controller command', function(r,s) {
    console.log('controller commmand feedback: r=%d, s=%d',r,s);
});

zwave.connect('/dev/ttyUSB0');

process.on('SIGINT', function() {
    console.log('disconnecting...');
    zwave.disconnect('/dev/ttyUSB0');
    process.exit();
});
```

Sample output from this program:

```sh
$ nodejs test2.js
initialising OpenZWave addon (/home/ekarak/src/node-openzwave-shared/lib/../build/Debug/openzwave_shared.node)
scanning homeid=0xcafebabe...
node2: nop
node1: Zensys, Controller
node1: name="", type="Static PC Controller", location=""
node1: class 32
node1:   Basic=0
node2: nop
node2: nop
====> scan complete, hit ^C to finish.
node2: node dead
controller commmand feedback: r=4, s=0
^Cdisconnecting...
```
