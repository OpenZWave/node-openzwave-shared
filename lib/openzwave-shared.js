/*
 * Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
 * Copyright (c) 2015 Elias Karakoulakis <elias.karakoulakis@gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

var fs = require('fs');
var EventEmitter = require('events').EventEmitter;

var debugAddon     = __dirname + '/../build/Debug/openzwave_shared.node'; 
var releaseAddon   = __dirname + '/../build/Release/openzwave_shared.node'; 
var addonFileName  = ((fs.existsSync(debugAddon)) ? debugAddon : releaseAddon);

// console.log("initialising OpenZWave addon ("+addonFileName+")");
var addonModule = require(addonFileName);

/* 
 * we need a proxy EventEmitter instance because apparently there's 
 * no (easy?) way to inherit an EventEmitter (JS code) from C++
 **/
var ee = new EventEmitter();

addonModule.Emitter.prototype.addListener = function(evt, callback) {
	ee.addListener(evt, callback);
}
addonModule.Emitter.prototype.on = addonModule.Emitter.prototype.addListener;
addonModule.Emitter.prototype.emit = function(evt, arg1, arg2, arg3, arg4) {
	ee.emit(evt, arg1, arg2, arg3, arg4);
}
addonModule.Emitter.prototype.removeListener = function(evt, callback) {
	ee.removeListener(evt, callback);
}
addonModule.Emitter.prototype.removeAllListeners = function(evt) {
	ee.removeAllListeners(evt);
}

module.exports = addonModule.Emitter;
