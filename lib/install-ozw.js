//This file will download the most recent version of open zwave, build it, copy it to
//~\AppData\Local\OpenZwave, and set the environment variable OZW_HOME
//USAGE: node install-ozw-windows.js [node-gyp-args]

var fs = require('fs');
var ChildProcess = require('child_process');
var path = require('path');

if (/^win/.test(process.platform)) {
  if (process.env.OZW_HOME) {
    return;
  }
  console.log('Installing Open Z-Wave');
} else {
  return; //this script only currently implements the install process for windows
}

var originalPath = process.cwd();
var tempPath = path.resolve(require('os').tmpDir() + 'ozwinstall/' + Math.random().toString(36).substring(7));
var installPath =  path.resolve(process.env.HOMEPATH + '/AppData/Local/OpenZWave/');
var ozwSourceUrl = "https://github.com/OpenZWave/open-zwave/archive/master.zip";
var gypOptions = [];

fs.mkdirSync(tempPath);

process.chdir(tempPath);
console.log('Installing dependencies to ' + tempPath);
ChildProcess.execSync('npm install request unzip node-gyp wrench');
var request = require(tempPath + '/node_modules/request');
var unzip = require(tempPath + '/node_modules/unzip');
var gyp = require(tempPath + '/node_modules/node-gyp')();
var wrench = require(tempPath + '/node_modules/wrench');

var ozwGyp = {
  "targets": [
    {
      "target_name": "openzwave",
      "type": "static_library",
      "sources": [
        "cpp/tinyxml/tinystr.cpp",
        "cpp/tinyxml/tinyxml.cpp",
        "cpp/tinyxml/tinyxmlerror.cpp",
        "cpp/tinyxml/tinyxmlparser.cpp",
        "cpp/src/aes/aes_modes.c",
        "cpp/src/aes/aescrypt.c",
        "cpp/src/aes/aeskey.c",
        "cpp/src/aes/aestab.c",
        "cpp/src/command_classes/Alarm.cpp",
        "cpp/src/command_classes/ApplicationStatus.cpp",
        "cpp/src/command_classes/Association.cpp",
        "cpp/src/command_classes/AssociationCommandConfiguration.cpp",
        "cpp/src/command_classes/Basic.cpp",
        "cpp/src/command_classes/BasicWindowCovering.cpp",
        "cpp/src/command_classes/Battery.cpp",
        "cpp/src/command_classes/CRC16Encap.cpp",
        "cpp/src/command_classes/ClimateControlSchedule.cpp",
        "cpp/src/command_classes/CentralScene.cpp",
        "cpp/src/command_classes/Clock.cpp",
        "cpp/src/command_classes/Color.cpp",
        "cpp/src/command_classes/CommandClass.cpp",
        "cpp/src/command_classes/CommandClasses.cpp",
        "cpp/src/command_classes/Configuration.cpp",
        "cpp/src/command_classes/ControllerReplication.cpp",
        "cpp/src/command_classes/DeviceResetLocally.cpp",
        "cpp/src/command_classes/DoorLock.cpp",
        "cpp/src/command_classes/DoorLockLogging.cpp",
        "cpp/src/command_classes/EnergyProduction.cpp",
        "cpp/src/command_classes/Hail.cpp",
        "cpp/src/command_classes/Indicator.cpp",
        "cpp/src/command_classes/Language.cpp",
        "cpp/src/command_classes/Lock.cpp",
        "cpp/src/command_classes/ManufacturerSpecific.cpp",
        "cpp/src/command_classes/Meter.cpp",
        "cpp/src/command_classes/MeterPulse.cpp",
        "cpp/src/command_classes/MultiCmd.cpp",
        "cpp/src/command_classes/MultiInstance.cpp",
        "cpp/src/command_classes/MultiInstanceAssociation.cpp",
        "cpp/src/command_classes/NoOperation.cpp",
        "cpp/src/command_classes/NodeNaming.cpp",
        "cpp/src/command_classes/Powerlevel.cpp",
        "cpp/src/command_classes/Proprietary.cpp",
        "cpp/src/command_classes/Protection.cpp",
        "cpp/src/command_classes/SceneActivation.cpp",
        "cpp/src/command_classes/Security.cpp",
        "cpp/src/command_classes/SensorAlarm.cpp",
        "cpp/src/command_classes/SensorBinary.cpp",
        "cpp/src/command_classes/SensorMultilevel.cpp",
        "cpp/src/command_classes/SwitchAll.cpp",
        "cpp/src/command_classes/SwitchBinary.cpp",
        "cpp/src/command_classes/SwitchMultilevel.cpp",
        "cpp/src/command_classes/SwitchToggleBinary.cpp",
        "cpp/src/command_classes/SwitchToggleMultilevel.cpp",
        "cpp/src/command_classes/ThermostatFanMode.cpp",
        "cpp/src/command_classes/ThermostatFanState.cpp",
        "cpp/src/command_classes/ThermostatMode.cpp",
        "cpp/src/command_classes/ThermostatOperatingState.cpp",
        "cpp/src/command_classes/ThermostatSetpoint.cpp",
        "cpp/src/command_classes/TimeParameters.cpp",
        "cpp/src/command_classes/UserCode.cpp",
        "cpp/src/command_classes/Version.cpp",
        "cpp/src/command_classes/WakeUp.cpp",
        "cpp/src/command_classes/ZWavePlusInfo.cpp",
        "cpp/src/value_classes/Value.cpp",
        "cpp/src/value_classes/ValueBool.cpp",
        "cpp/src/value_classes/ValueButton.cpp",
        "cpp/src/value_classes/ValueByte.cpp",
        "cpp/src/value_classes/ValueDecimal.cpp",
        "cpp/src/value_classes/ValueInt.cpp",
        "cpp/src/value_classes/ValueList.cpp",
        "cpp/src/value_classes/ValueRaw.cpp",
        "cpp/src/value_classes/ValueSchedule.cpp",
        "cpp/src/value_classes/ValueShort.cpp",
        "cpp/src/value_classes/ValueStore.cpp",
        "cpp/src/value_classes/ValueString.cpp",
        "cpp/src/platform/Controller.cpp",
        "cpp/src/platform/Event.cpp",
        "cpp/src/platform/FileOps.cpp",
        "cpp/src/platform/HidController.cpp",
        "cpp/src/platform/Log.cpp",
        "cpp/src/platform/Mutex.cpp",
        "cpp/src/platform/SerialController.cpp",
        "cpp/src/platform/Stream.cpp",
        "cpp/src/platform/Thread.cpp",
        "cpp/src/platform/TimeStamp.cpp",
        "cpp/src/platform/Wait.cpp",
        "cpp/src/Driver.cpp",
        "cpp/src/Group.cpp",
        "cpp/src/Manager.cpp",
        "cpp/src/Msg.cpp",
        "cpp/src/Node.cpp",
        "cpp/src/Notification.cpp",
        "cpp/src/Options.cpp",
        "cpp/src/Scene.cpp",
        "cpp/src/Utils.cpp",
        "cpp/src/ZWSecurity.cpp"
      ],
      "include_dirs": [
        "cpp/hidapi/hidapi",
        "cpp/src",
        "cpp/src/command_classes",
        "cpp/src/platform",
        "cpp/src/value_classes",
        "cpp/tinyxml"
      ],
      "configurations": {
        "Release": {
          "cflags": [
            "-Wno-ignored-qualifiers",
            "-Wno-tautological-undefined-compare",
            "-Wno-unknown-pragmas"
          ],
          "xcode_settings": {
            "OTHER_CFLAGS": [
              "-Wno-ignored-qualifiers",
              "-Wno-tautological-undefined-compare",
              "-Wno-unknown-pragmas"
            ]
          },
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': 1
            }
          },
        }
      },
      "conditions": [
        ['OS=="linux"', {
          "include_dirs": [ "cpp/src/platform/unix" ],          
          "sources": [
            "cpp/hidapi/linux/hid.c",
            "cpp/src/platform/unix/EventImpl.cpp",
            "cpp/src/platform/unix/FileOpsImpl.cpp",
            "cpp/src/platform/unix/LogImpl.cpp",
            "cpp/src/platform/unix/MutexImpl.cpp",
            "cpp/src/platform/unix/SerialControllerImpl.cpp",
            "cpp/src/platform/unix/ThreadImpl.cpp",
            "cpp/src/platform/unix/TimeStampImpl.cpp",
            "cpp/src/platform/unix/WaitImpl.cpp"
          ]
        }],
        ['OS=="mac"', {
          "include_dirs": [ "cpp/src/platform/unix" ],                  
          "sources": [
            "cpp/hidapi/mac/hid.c",
            "cpp/src/platform/unix/EventImpl.cpp",
            "cpp/src/platform/unix/FileOpsImpl.cpp",
            "cpp/src/platform/unix/LogImpl.cpp",
            "cpp/src/platform/unix/MutexImpl.cpp",
            "cpp/src/platform/unix/SerialControllerImpl.cpp",
            "cpp/src/platform/unix/ThreadImpl.cpp",
            "cpp/src/platform/unix/TimeStampImpl.cpp",
            "cpp/src/platform/unix/WaitImpl.cpp"
          ],
          "defines": [
            "DARWIN"
          ]
        }],
        ['OS=="win"', {
          "include_dirs": [ "cpp/src/platform/windows" ],          
          "sources": [
            "cpp/build/windows/winversion.cpp",
            "cpp/hidapi/windows/hid.cpp",
            "cpp/src/platform/windows/EventImpl.cpp",
            "cpp/src/platform/windows/FileOpsImpl.cpp",
            "cpp/src/platform/windows/LogImpl.cpp",
            "cpp/src/platform/windows/MutexImpl.cpp",
            "cpp/src/platform/windows/SerialControllerImpl.cpp",
            "cpp/src/platform/windows/ThreadImpl.cpp",
            "cpp/src/platform/windows/TimeStampImpl.cpp",
            "cpp/src/platform/windows/WaitImpl.cpp"
          ]          
        }],
      ]
    }
  ]
};



function init() {
  console.log('Temp Path:', tempPath);
  console.log('Install Path:', installPath);
  
  wrench.rmdirSyncRecursive(installPath, true);
  wrench.mkdirSyncRecursive(installPath);
}

function download(url, dest, cb) {
  //doesn't handle error response codes
  request({uri: url})
      .pipe(fs.createWriteStream(dest))
      .on('close', cb)
      .on('error', cb);
}

function build(gypArgs, cb) {
  try {
    fs.unlinkSync('./cpp/build/windows/winversion.cpp');
  } catch (e) {  }
  
  ChildProcess.execFile('GIT-VS-VERSION-GEN.bat',
    ['../../', 'winversion.cpp'], 
    { cwd: './cpp/build/windows' },
    function (error, stdout, stderr) {
      if (error) {
          console.log(error.stack);
          console.log('Error code: '+error.code);
          console.log('Signal received: '+error.signal);
      }
    });
   
  var args = ['node','.'];
  if (gypArgs) {
    args = args.concat(gypArgs);
  }    
  gyp.parseArgv(args);
  
  gyp.commands.clean([], function(err) {
    if (err) return cb(err);
    gyp.commands.configure([], function(err) {
      if (err) return cb(err);
      gyp.commands.build([], cb);
    });
  });
}

function copyFiles() {
  wrench.copyDirSyncRecursive(tempPath + '/ozw/open-zwave-master/config', installPath + '/config');
  wrench.copyDirSyncRecursive(tempPath + '/ozw/open-zwave-master/cpp/src',  installPath + '/include', { filter: /^.*\.(?!h)[^.]+$/ });
  wrench.copyDirSyncRecursive(tempPath + '/ozw/open-zwave-master/build/Release',  installPath + '/bin');
}

function handleError(err) {
  throw err;
}

module.exports = function(opts) {
  gypOptions = opts.gyp || gypOptions;
  init();
  console.log('Downloading open zwave source.');
  
  download(ozwSourceUrl, tempPath + "/ozw.zip", function(err) {
    if (err) { return handleError(err); }
    
    console.log('Extracting open zwave source.');
    fs.createReadStream( tempPath + "/ozw.zip")
      .pipe(unzip.Extract({ path: tempPath + '/ozw' }))
      .on('close', function() {
        fs.writeFile(tempPath + '/ozw/open-zwave-master/binding.gyp', JSON.stringify(ozwGyp, null, 4), function(err) {
            if(err) handleError;
            
            console.log('Building open zwave');
            process.chdir(tempPath + "/ozw/open-zwave-master");                  
            build(gypOptions, function(err) {
              if (err) handleError(err);
              process.chdir(originalPath);        
              copyFiles();
              //set environment variable    
              ChildProcess.exec('SETX OZW_HOME "' + installPath + '"',function(error, stdout, stderr) {
                // command output is in stdout
              });
            });
          });
      })
      .on('error', handleError);
  });
}

if (require.main === module) {
  module.exports({ gyp: process.argv.slice(2) });
}