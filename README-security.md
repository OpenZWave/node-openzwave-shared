#### TODO: *step in and complete this guide!*
As I currently do not own any security devices, please feel free to contribute
your experience with openzwave-shared and the new features in OpenZWave 1.3

Ideally this document should describe
- *How to set up your OZW installation for security*: Initialise the addon with a NetworkKey that's simply a string of 16 bytes in hex format, separated by commas:
```js
var zwave = new ZWave({
    Logging: true,
    ConsoleOutput: true,
    NetworkKey: "0xCA,0xFE,0xBA,0xBE,.... " // <16 bytes total>
});
```
suggestion: Use [the random.org generator](https://www.random.org/cgi-bin/randbyte?nbytes=16&format=h)
to get a fresh set of 16 random hex numbers (but don't forget to add the "0x" prefix and the commas! )

- How to add a new secure device into our controller
- How to ..?

A good starting point would be:
https://github.com/OpenZWave/open-zwave/wiki/Adding-Security-Devices-to-OZW
