var ZWave = require('./lib/openzwave-shared.js');
var os = require('os');

var zwave = new ZWave({
  //ConsoleOutput: false
  ConsoleOutput: true
});

zwavedriverpaths = {
  "darwin": '/dev/cu.usbmodem1411',
  "linux": '/dev/ttyUSB0',
  "windows": '\\\\.\\COM3'
}

var nodes = [];
var driver;
var homeid = null;
zwave.on('driver ready', function(home_id, drv) {
  homeid = home_id;
  driver = drv;
  console.log('scanning homeid=0x%s... drv=%j', homeid.toString(16), drv);
});




console.log("connecting to " + zwavedriverpaths[os.platform()]);
zwave.connect(zwavedriverpaths[os.platform()]);

process.on('SIGINT', function() {
  console.log('disconnecting...');
  zwave.disconnect(zwavedriverpaths[os.platform()]);
  process.exit();
});
