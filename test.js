/*
 * OpenZWave test program.
 */

var OpenZWave = require('./lib/openzwave-shared.js');
var os = require('os');

// this test assumes no actual ZWave controller exists on the system
// and is just a rudimentary check that the driver can initialise itself.
var zwave = new OpenZWave();

zwave.on('ping', function() {
    console.log('driver is emitting events properly');
	process.exit();
});
console.log('Testing if OZW is emitting events properly');
zwave.ping();

setTimeout(function() {
	console.log('timeout pinging the driver');
	process.exit(1);
}, 1000);
