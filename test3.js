var ZWaveModule = require('./lib/openzwave-shared.js');
var zwave = new ZWaveModule('/dev/ttyUSB0');

console.log(zwave);
