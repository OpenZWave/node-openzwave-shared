var ZWaveModule = require('./lib/openzwave-shared.js');
var zwave = new ZWaveModule();
for (var k in zwave.ee) {
	console.log(k);
}

