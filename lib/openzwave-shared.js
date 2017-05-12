/*
 * Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
 * Copyright (c) 2015-2017 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

console.log("initialising OpenZWave addon ("+addonFileName+")");
var addonModule = require(addonFileName);

/*
 Inheriting prototype EventEmitter
 We would use Object.assign, but that isn't supported in node 0.12.
 */
addonModule.Emitter.prototype.addListener = EventEmitter.prototype.addListener;
addonModule.Emitter.prototype.on = EventEmitter.prototype.on;
addonModule.Emitter.prototype.once = EventEmitter.prototype.once;
addonModule.Emitter.prototype.removeListener = EventEmitter.prototype.removeListener;
addonModule.Emitter.prototype.removeAllListeners = EventEmitter.prototype.removeAllListeners;
addonModule.Emitter.prototype.setMaxListeners = EventEmitter.prototype.setMaxListeners;
addonModule.Emitter.prototype.listeners = EventEmitter.prototype.listeners;
addonModule.Emitter.prototype.emit = EventEmitter.prototype.emit;

/*
 Inheriting these, only if they exist on EventEmitter.
 If we didn't, and added them either way, we would pollute the prototype with undefined keys.
 */
if (EventEmitter.prototype.getMaxListeners) {
	addonModule.Emitter.prototype.getMaxListeners = EventEmitter.prototype.getMaxListeners;
}
if (EventEmitter.prototype.prependListener) {
	addonModule.Emitter.prototype.prependListener = EventEmitter.prototype.prependListener;
}
if (EventEmitter.prototype.prependOnceListener) {
	addonModule.Emitter.prototype.prependOnceListener = EventEmitter.prototype.prependOnceListener;
}
if (EventEmitter.prototype.listenerCount) {
	addonModule.Emitter.prototype.listenerCount = EventEmitter.prototype.listenerCount;
}
if (EventEmitter.prototype.eventNames) {
	addonModule.Emitter.prototype.eventNames = EventEmitter.prototype.eventNames;
}

module.exports = addonModule.Emitter;
