var ZWaveModule = require('./lib/openzwave-shared.js');
var zwave = new ZWaveModule();
for (var k in zwave) {
	console.log(k);
}
console.log(zwave.emit);

