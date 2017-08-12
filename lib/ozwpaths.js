/*
 * Copyright (c) 2016-2017 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

// helper script for binding.gyp to detect OpenZWave directories
// tries to 1) use pkg-config first, then 2) tries plain 'find', then finally
// 3) falls back to a default value
var fs = require('fs');
var cp = require('child_process');
var util = require('util');
var path = require('path');

function doScript(script, suppressException) {
  //console.log('---> ' + script);
  try {
    return(cp.execSync(script).toString());
  } catch(e) {
    //console.log('---> ' + e.toString());
    if (!suppressException) throw e;
    if (e.stdout) return e.stdout.toString();
  }
}

var configs = {
  includedir: {
    findpattern: 'OZWException.h',
    locations: ["/usr/local/include", "/usr/include"]
  },
  libdir: {
    findpattern: 'libopenzwave.so',
    locations: ["/usr/local/lib64", "/usr/local/lib", "/usr/lib64", "/usr/lib"]
  },
  sysconfdir: {
    findpattern: 'zwcfg.xsd',
    locations: ["/usr/local/etc", "/usr/etc", "/etc"]
  },
  docdir: {
    findpattern: 'Doxyfile.in',
    locations: ["/usr/local/share/doc", "/usr/doc"]
  }
}

var usepkgconfig = true;
try {
  cp.execSync("pkg-config --exists libopenzwave 2>/dev/null");
} catch (e) {
  usepkgconfig = false;
}

function getConfigItem(item) {
  // 1. try the easy way first: use pkg-config and libopenzwave.pc
  if (usepkgconfig) {
    var cmdfmt = "pkg-config --variable=%s libopenzwave";
    var cmd = util.format(cmdfmt, item);
    var result = cp.execSync(cmd).toString().split('\n')[0];
    if (result) return (result);
  }
  // 2. if pkg-config does NOT yeild a value, try using find
  var cmdfmt = "find %s -name %s 2>/dev/null";
  var fp = configs[item].findpattern;
  for (var i in configs[item].locations) {
    var loc = configs[item].locations[i];
    if (!(loc && fs.existsSync(loc))) continue;
    var cmd = util.format(cmdfmt, loc, fp);
    var dir = doScript(cmd, true);
    if (!dir) continue;
    return path.dirname(dir.split('\n')[0]);
  }
}

var paths = {};
Object.keys(configs).forEach(function(item) {
  paths[item] = getConfigItem(item);
});

//console.log('=====RESULTS=====');
//console.log(JSON.stringify(paths, null, 2));
module.exports = paths;
