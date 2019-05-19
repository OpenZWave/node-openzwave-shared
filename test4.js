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

function do_disconnect() {
	console.log('\n\n== Driver Statistics: %j',
		zwave.getDriverStatistics());

	console.log('disconnecting...');
	zwave.disconnect(zwp);

	zwave.updateOptions({
		ConsoleOutput: true,
		Logging: true,
		SaveConfiguration: false,
		DriverMaxAttempts: 3,
		PollInterval: 500,
		SuppressValueRefresh: true,
	});

	zwave.connect(zwp);
	setTimeout(function(){
		zwave.disconnect(zwp);
		process.exit();
	}, 5000);
}

var zwavedriverpaths = {
	"darwin": '/dev/cu.usbmodem1411',
	"linux": '/dev/ttyACM0',
	"windows": '\\\\.\\COM3'
}

var zwp = zwavedriverpaths[os.platform()];
console.log("Connecting to %s",zwp);
zwave.connect(zwp);
process.on('SIGINT', do_disconnect);
setTimeout(do_disconnect, 3000);
