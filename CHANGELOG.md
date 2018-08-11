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
