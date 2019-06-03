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

var fs = require("fs");
var EventEmitter = require("events").EventEmitter;
require("./polyfill");

var debugAddon = __dirname + "/../build/Debug/openzwave_shared.node";
var releaseAddon = __dirname + "/../build/Release/openzwave_shared.node";
var addonFileName = fs.existsSync(debugAddon) ? debugAddon : releaseAddon;

//console.log("loading "+addonFileName);
var addonModule = require(addonFileName);

for (var k in EventEmitter.prototype) {
	addonModule.Emitter.prototype[k] = EventEmitter.prototype[k];
}

var enums = require("./enums");

var instance;
function init(options) {
	if (!instance) instance = new addonModule.Emitter(options);
	return instance;
}

module.exports = Object.assign(init, { default: init }, enums);
