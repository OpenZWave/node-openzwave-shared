node-openzwave-shared
=====================

This is a node.js add-on for node 0.10.x *and* 0.12.x which wraps the [Open
Z-Wave](https://www.openzwave.com/) library to provide access to a
Z-Wave network from within node.js (server-side JavaScript.)

You can now easily control *and manage* your [ZWave](http://www.z-wave.com/) devices 
(lights, dimmers, blinds, you name it) from within [NodeJS](https://nodejs.org/) applications.

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

- This library differs in that it links *dynamically* to an OpenZWave **shared 
library** by means of your system dynamic linker. This is  in contrast to 
*statically* linking OpenZWave in the node.js addon. 
Thus you need to have OpenZWave fully installed on your system (both the
compiled library AND the development headers) before trying to install this little baby.
I know this diverges from the dominant npm paradigm, but with the shared lib approach:
  - compilation / installation is a lot faster and
  - OZW minor upgrades / bugfixes are way lot easier.
  
This also means that you need to be careful if you upgrade your 
OZW library: you might need to rebuild this addon, otherwise you'd might
get api mismatch exceptions.

- The Node.js C++ API has changed dramatically between 0.10.x and 0.12.x
thus I'm using [the Native Abstractions for Node library] (https://www.npmjs.com/package/nan) 
to get this node addon to compile and run across all known versions. 

Manually compiling OpenZwave involves the usual dance of 
(`make && sudo make install`) OR you could try installing
a precompiled package (eg for Raspbian it should be like
`sudo apt-get install libopenzwave-dev`). Precompiled packages 
for various Linux flavours and architectures can be downloaded
from http://old.openzwave.com/snapshots/.

Be sure to install BOTH the binary (libopenzwave-x.y) AND the development
package (libopenzwave-dev).

## Install

The module currently builds only on Linux (and Windows, but its untested). 
On Linux you will need to ensure the OpenZWave library and headers are 
installed first. 
It should also compile in Windows, but you need to edit binding.gyp 
to set the paths for the OpenZWave library sources and libraries.

```
$ sudo npm install -g openzwave-shared
```

[Basic API usage](../master/README-api.md)

[See list of supported EVENTS](../master/README-events.md)

[See example usage](../master/README-example.md)


## License

The [Open Z-Wave](https://code.google.com/p/open-zwave/) library that this
module heavily relies upon is licensed under the GPLv3.

Everything else (all the bits that I have written) is under the vastly more
sensible ISC license.
