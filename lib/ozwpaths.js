/*
 * Copyright (c) 2016 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

var configs = {
  includedir: {
    findpattern: 'OZWException.h',
    locations: ["/usr/include/", "/usr/local/include/"],
    default: "/usr/local/include/openzwave/"
  },
  libdir: {
    findpattern: 'libopenzwave.so',
    locations: ["/usr/lib*", "/usr/local/lib*"],
    default: "/usr/local/lib64/"
  },
  sysconfdir: {
    findpattern: 'zwcfg.xsd',
    locations: ["/usr/etc/", "/usr/local/etc/", "/etc/"],
    default: "/usr/local/etc/openzwave/"
  },
  docdir: {
    findpattern: 'openzwave-*',
    locations: ["/usr/local/share/doc/"],
    default: "/usr/local/share/doc/openzwave-1.4.2278"
  }
}

var usepkgconfig = true;
try {
  cp.execSync("pkg-config --exists libopenzwave");
} catch (e) {
  usepkgconfig = false;
}

function getConfigItem(item) {
  // 1. try using pkg-config
  if (usepkgconfig) {
    var cmdfmt = "pkg-config --variable=%s libopenzwave";
    var cmd = util.format(cmdfmt, item);
    return (cp.execSync(cmd).toString().split('\n')[0]);
  } else {
    // 2. try using find
    var cmdfmt = "find %s ! -readable -prune -o -name %s -print";
    var fp = configs[item].findpattern;
    for (var i in configs[item].locations) {
      var loc = configs[item].locations[i];
      if (!fs.existsSync(loc)) continue;
      var cmd = util.format(cmdfmt, loc, fp);
      try {
        var dir = cp.execSync(cmd).toString().split('\n')[0];
        if (!dir) continue;
        return path.dirname(dir)
      } catch (e) {
        // console.log('not found in %s', loc);
      }
    }
  }
}

var paths = {};

// try the easy way first: use pkg-config and libopenzwave.pc
Object.keys(configs).forEach(function(item) {
  var ret = getConfigItem(item);
  // or fallback to the default value if automatic scan fails
  paths[item] = ret || configs[item].default;
});

//console.log('=====RESULTS=====');
//console.log(JSON.stringify(paths, null, 2));
module.exports = paths;
