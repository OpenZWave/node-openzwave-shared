node-openzwave-shared
=====================
[Linux![Linux Build Status](https://travis-ci.org/OpenZWave/node-openzwave-shared.svg?branch=master)](https://travis-ci.org/OpenZWave/node-openzwave-shared)
[Windows![Windows Build status](https://ci.appveyor.com/api/projects/status/txg360huomtpgc8o?svg=true)](https://ci.appveyor.com/project/ekarak/node-openzwave-shared)
[![Join the chat at https://gitter.im/OpenZWave/node-openzwave-shared](https://badges.gitter.im/OpenZWave/node-openzwave-shared.svg)](https://gitter.im/OpenZWave/node-openzwave-shared?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Google group](http://www.google.com/images/icons/product/groups-32.png)](https://groups.google.com/d/forum/node-openzwave-shared)

This is the homepage for *node-openzwave-shared*, the official binary add-on for
Node.js, which wraps [Open ZWave](http://openzwave.com/), a high quality C++
library for controlling Z-Wave networks.

<img src="https://nodejs.org/static/images/logos/nodejs.png" style="width: 200px"/>
<img src="http://www.openzwave.com/css/ozwlogo.png"/>

You can now easily control *and manage* your [ZWave](http://www.z-wave.com/)
devices (lights, dimmers, blinds, you name it) from within [Node.js](https://nodejs.org/)
applications. [This library also supports *secure* devices](../master/README-security.md)
(eg door locks) that require encryption.

All widely used Node.js versions are supported with the help of [NaN](https://github.com/nodejs/nan).
This add-on is currently tested against 0.12.x on ARM and 4.2.x on x86_64, but
it should also work on all supported Node.js versions (as early as 0.11.12) and
architectures.

Check out the [Node-Red integration project](https://github.com/OpenZWave/node-red-contrib-openzwave)
for an interesting use case, in which I've wired up ZWave and KNX devices
working together as one big happy automated home.

This addon is currently able to:
- *scan* a Z-Wave network and report on connected devices,
- *write* values to zwave nodes
- *monitor* the network for changes,
- *heal* nodes and/or the network
- *perform* management tasks (add/remove nodes, replace failed nodes, manage
	their group associations etc)

**API change notice (v1.4.0)**
`enablePoll()` and `disablePoll()` used to affect *only the first ValueID of any given command class*.
This is *wrong*, as for multi-instance devices you probably need to poll multiple ValueID's.
These calls now accept a valueId, in very much the same way as `setValue()` does. This means that
you now have to pass a valueID object (or its 4 constituents) for each value you want to enable/disable
polling for.


**Important notice**

This library differs from its [ancestor library](https://github.com/jperkin/node-openzwave)
in that it links *dynamically* to an OpenZWave **shared library**
by means of your system dynamic linker. This is  in contrast to
*statically* linking OpenZWave as part of the node.js addon.

Thus you need to have OpenZWave fully installed on your system (both the
compiled library AND the development headers) before trying to install this
little baby.
I know this diverges from the dominant npm paradigm, but with the shared lib approach:
  - compilation / installation is a lot faster and
  - OZW *minor* upgrades / bugfixes are way lot easier.

This also means that you need to be careful if you upgrade your
OZW library: you might need to rebuild this addon, otherwise you'd might
get api mismatch exceptions.

## Prerequisites

### Linux

You will need to ensure the OpenZWave library and headers are
installed first. You can do this one of two ways.

- Manually compiling OpenZwave involves the usual dance of 1) downloading its source tarball
([latest code from GitHub](https://github.com/OpenZWave/open-zwave/archive/master.zip) or
[a stable release](https://github.com/OpenZWave/open-zwave/releases)) or
from [the OpenZWave snapshots repository](http://old.openzwave.com/snapshots/)
and then 2) compiling it and installing on your system (`make && sudo make install && sudo ldconfig`)

- You could also install OpenZWave via a [precompiled package that's suitable for
your Linux distribution and architecture](http://old.openzwave.com/downloads/).
**Notice:** Be sure to install *BOTH the binary (libopenzwave-x.y) AND the
development package (libopenzwave-dev).*

### MacOS/X

The only dependency that you need before compiling is `pkg-config`. Then, you
could either pull down the OZW repo from Github and do a `make && sudo make install` ,
or you could try installing OpenZWave using `brew install open-zwave`.

### Windows

Since there is no standard installation location for Open Z-Wave on Windows, it
will be automatically downloaded, compiled, and installed when you install this module.

## Installation and test script

Whenever you have OpenZWave installed in your machine, then all you need to do is:

```
$ npm install openzwave-shared
```

To try it out, boot up NodeJS, and use the `.load` shell helper function to boot up a basic OpenZWave CLI. This will initialise and expose 1) a `zwave` object that you can use to send commands and 2) the `nodes` object to get a list of all nodes:

```js
$ node
> .load test2.js
...
...

// the 1st node is the USB controller stick
> console.log(nodes[1])
{ manufacturer: 'Aeotec',
  manufacturerid: '0x0086',
  product: 'Z-Stick S2',
  producttype: '0x0002',
  productid: '0x0001',
  type: 'Static PC Controller',
  name: '',
  loc: '',
  classes: { '32': { '0': [Object] } },
  ready: true }

// set dimmer (node 5) to 50%
> zwave.setValue(5,38,1,0,50);
undefined
> node5: changed: 38:Level:54->54
```

**Notice 1:** If you receive the error `cannot find -lopenzwave` on a 64-bit Linux
system, `libopenzwave.so` was likely compiled into `/usr/local/lib64`.
Run the terminal command `ld -lopenzwave --verbose` for a list of search
locations used. You can workaround this by providing a symlink to one of the
listed locations such as `/usr/local/lib`.
Run `sudo ln -s /usr/local/lib64/libopenzwave.so /usr/local/lib/libopenzwave.so`
creates symlink so that the file appears to be in the location that `ld` looks
in. Now `npm install` should work.

**Notice 2:** nodejs Debian/Raspbian package (v0.10.29) causes an issue REPLACE_INVALID_UTF8 (cf. https://github.com/nodejs/nan/issues/414). You can use node from [herokuapp](http://node-arm.herokuapp.com/) or upgrade some node packages: `sudo npm -g install npm node-gyp`.

In case you want to develop your application in TypeScript, there is also a TypeScript
declaration file available. Besides adding typechecking to your codebase this will also
help you to navigate the API and see the type of parameters available for each function. You can
download the
[latest version of the declaration file from GitHub](https://github.com/jbaron/openzwave-shared-tsd).

## Development documentation

- [Basic API usage](../master/README-api.md)
- [List of supported events](../master/README-events.md)
- [Management of nodes](../master/README-mgmt.md)
- [Security API](../master/README-security.md)
- [Usage example](../master/README-example.md)
- [Deprecated API calls in OpenZWave 1.6](../master/README-deprecated-16.md)
  
## Environment-specific documentation
- [Installing on Rapsbian](../master/README-raspbian.md)
- [Installing on Ubuntu 15.04](../master/README-ubuntu.md)

## License

The [Open Z-Wave](http://www.openzwave.com/) library that this
module heavily relies upon is licensed under the Lesser GPLv3.

Everything else (all the bits that I and Jonathan have written)
is under the vastly more sensible ISC license.
