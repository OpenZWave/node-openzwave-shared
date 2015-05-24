node-openzwave-shared
=====================

This is a node.js add-on which wraps the [Open
Z-Wave](https://www.openzwave.com/) library to provide access to a
Z-Wave network from within node.js (server-side JavaScript.)

It is currently able to:
- scan a Z-Wave network and report on connected devices,
- write values to zwave nodes
- monitor the network for changes,
- healing nodes and/or the network
- perform management tasks (add/remove nodes, replace failed nodes etc)

*Important notice*

This library differs in that it links dynamically to an OpenZWave *shared library* 
(in contrast to statically linking OpenZWave in the node addon). 
Thus you need to have OpenZWave fully installed on your system (both the
compiled library AND the development headers) before trying to install this little baby.
This also means that its definately going to break if you upgrade your 
OZW without rebuilding this plugin. So don't control nuclear reactors 
with it, OK?

Manually compiling OpenZwave involves the usual dance of 
(`make && sudo make install`) OR you could try installing
a precompiled package (eg for Raspbian it should be like
`sudo apt-get install libopenzwave-dev`). Precompiled packages 
for various Linux flavours and architectures can be downloaded
from http://old.openzwave.com/snapshots/.

Be sure to install BOTH the binary (libopenzwave-x.y) AND the development
package (libopenzwave-dev).

## Install

The module currently builds only on Linux. On Linux you will need to
ensure the OpenZWave library and headers are installed first.

```
$ sudo npm install -g openzwave-shared
```

[Basic API usage](../blob/master/README-api.md)

[See list of supported EVENTS](../blob/master/README-events.md)

[See example usage](../blob/master/README-example.md)


## License

The [Open Z-Wave](https://code.google.com/p/open-zwave/) library that this
module heavily relies upon is licensed under the GPLv3.

Everything else (all the bits that I have written) is under the vastly more
sensible ISC license.
