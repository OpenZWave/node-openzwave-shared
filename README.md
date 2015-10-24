node-openzwave-shared
=====================

This is a node.js add-on for node 0.10.x *and* 0.12.x (and possibly
even for NodeJS's 3 and 4, with the help of NAN), which wraps the [Open
Z-Wave](http://www.openzwave.com/) library to provide access to a
Z-Wave network from within node.js (server-side JavaScript.)

<img src="https://nodejs.org/static/images/logos/nodejs.png" style="width: 200px"/>
<img src="http://www.openzwave.com/css/ozwlogo.png"/>

You can now easily control *and manage* your [ZWave](http://www.z-wave.com/) devices
(lights, dimmers, blinds, you name it) from within [NodeJS](https://nodejs.org/) applications.
Initial support for *secure* devices (eg door locks) that require encryption is also included.

Check out the [Node-Red integration project](https://github.com/OpenZWave/node-red-contrib-openzwave)
for an interesting use case, in which I've wired up ZWave and KNX devices working together as
one big happy automated home.

This addon is currently able to:
- *scan* a Z-Wave network and report on connected devices,
- *write* values to zwave nodes
- *monitor* the network for changes,
- *heal* nodes and/or the network
- *perform* management tasks (add/remove nodes, replace failed nodes, manage their group associations etc)

**Important notice**

This library differs from its [ancestor library](https://github.com/jperkin/node-openzwave)
in that it links *dynamically* to an OpenZWave **shared library**
by means of your system dynamic linker. This is  in contrast to
*statically* linking OpenZWave as part of the node.js addon.

Thus you need to have OpenZWave fully installed on your system (both the
compiled library AND the development headers) before trying to install this little baby.
I know this diverges from the dominant npm paradigm, but with the shared lib approach:
  - compilation / installation is a lot faster and
  - OZW *minor* upgrades / bugfixes are way lot easier.

This also means that you need to be careful if you upgrade your
OZW library: you might need to rebuild this addon, otherwise you'd might
get api mismatch exceptions.

## Prerequisites

### Linux/OSX

You will need to ensure the OpenZWave library and headers are
installed first. You can do this one of two ways.

- Manually compiling OpenZwave involves the usual dance of 1) downloading its source tarball
([latest code from GitHub](https://github.com/OpenZWave/open-zwave/archive/master.zip) or
[a stable release](https://github.com/OpenZWave/open-zwave/releases)) or
from [the OpenZWave snapshots repository](http://old.openzwave.com/snapshots/)
and then 2) compiling it and installing on your system (`make && sudo make install`)

- You could also install OpenZWave via a [precompiled package that's suitable for your Linux distribution and architecture](http://old.openzwave.com/downloads/).
**Notice:** Be sure to install *BOTH the binary (libopenzwave-x.y) AND the development
package (libopenzwave-dev).*

### Windows

Since there is no standard installation location for Open Z-Wave on Windows, it will be automatically downloaded, compiled, and installed when you install this module.

## Installation

**Node.JS >= 3.0 users**: please send me reports if the addon works or breaks.
I've had very bad experience with the NodeJS API quicksand already, and NAN
appears to not be able to keep up. The NodeJS API is truly a wizard's tribute to Ctrl+Z.

Whenever you have OpenZWave installed in your machine, then all you need to do is:
```
$ npm install openzwave-shared
```

## Development documentation

- [Basic API usage](../master/README-api.md)
- [List of supported events](../master/README-events.md)
- [Management of nodes](../master/README-mgmt.md)
- [Security API](../master/README-security.md)
- [Usage example](../master/README-example.md)

## Environment-specific documentation
- [Installing on Rapsbian](../master/README-raspbian.md)
- [Installing on Ubuntu 15.04](../master/README-ubuntu.md)

## License

The [Open Z-Wave](http://www.openzwave.com/) library that this
module heavily relies upon is licensed under the Lesser GPLv3.

Everything else (all the bits that I and Jonathan have written)
is under the vastly more sensible ISC license.
