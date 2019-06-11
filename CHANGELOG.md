# 1.4.0

Added TypeScript types.

# 1.4.1
Add SetValueLabel
Fix documentation typos

# 1.4.2
Upgrade NaN for proper Node.JS 10.x support

# 1.4.3
Fix inclusion paths for Windows builds
Add AppVeyor CI for Windows builds

# 1.4.4
Fix some compiler warnings
Add LogInitialisation boolean option

# 1.4.5
Deprecated Nan v8::Value::ToNumber() replaced by Uint32Value()
Deprecated v8::String::Utf8Value() replaced by Nan::Utf8String()
Update all emitted callbacks to use AsyncResource's

# 1.4.6 
bugfix/issue#217 - do not crash when no options object is passed to the constructor

# 1.4.7
issue #235 - added instanceids to add and remove association
Fix scene APIs #264
Fix scene values type 'list' (Partially fix #266) #267 

# 1.4.8
add support for SmartOS #272

# 1.6.0
Fix broken Node 12 builds (v8 7.4.xxx) #302
Initial support for OpenZWave 1.6 #300
  - implement exception macros
  - [deprecate several legacy OpenZWave calls](https://github.com/OpenZWave/open-zwave/wiki/OpenZWave-1.6-Release-Notes#deprecated-featuresmethods)
Updated Raspbian Readme to fix bug when installing on Raspbian stretch #271
iBetter typescript integration #263
