//This file will download the most recent version of open zwave, build it, copy it to
//~\AppData\Local\OpenZwave, and set the environment variable OZW_HOME
//USAGE: node install-ozw-windows.js [node-gyp-args]

var fs = require('fs');
var ChildProcess = require('child_process');
var path = require('path');
var request, unzip, gyp, wrench; //these are dynamically required later on

var originalPath = process.cwd();
var tempPath = path.resolve(require('os').tmpdir() + '/ozwinstall-' + Math.random().toString(36).substring(7));
var installPath =  path.resolve(process.env.HOMEPATH + '/AppData/Local/OpenZWave/');
var ozwSourceUrl = "https://github.com/OpenZWave/open-zwave/archive/master.zip";
var gypOptions = [];

var ozwGyp = {
  "targets": [
    {
      "target_name": "openzwave",
      "type": "static_library",
      "sources": [],  //these are populated dynamically by looking at the source code files
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
          ], 
          "sources/": [
            ['exclude', 'cpp/src/platform/windows/'],
            ['exclude', 'cpp/src/platform/winRT/']
          ]
        }],
        ['OS=="mac"', {
          "include_dirs": [ "cpp/src/platform/unix" ],                  
          "sources": [
            "cpp/hidapi/mac/hid.c",
          ], 
          "sources/": [
            ['exclude', 'cpp/src/platform/windows/'],
            ['exclude', 'cpp/src/platform/winRT/']
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
          ], 
          "sources/": [
            ['exclude', 'cpp/src/platform/unix/'],
            ['exclude', 'cpp/src/platform/winRT/']
          ],
          "link_settings": {
            "libraries": ["dnsapi.lib", "ws2_32.lib"]
          }
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
  if (/^win/.test(process.platform)) {
    if (process.env.OZW_HOME) {
      return;
    }
    console.log('Installing Open Z-Wave');
  } else {
    return; //this script only currently implements the install process for windows
  }
  
  fs.mkdirSync(tempPath);

  process.chdir(tempPath);
  console.log('Installing dependencies to ' + tempPath);
  ChildProcess.execSync('npm install request unzip node-gyp wrench');
  request = require(tempPath + '/node_modules/request');
  unzip = require(tempPath + '/node_modules/unzip');
  gyp = require(tempPath + '/node_modules/node-gyp')();
  wrench = require(tempPath + '/node_modules/wrench');
  

  
  gypOptions = opts.gyp || gypOptions;
  init();
  console.log('Downloading open zwave source.');
  
  download(ozwSourceUrl, tempPath + "/ozw.zip", function(err) {
    if (err) { return handleError(err); }
    
    console.log('Extracting open zwave source.');
    fs.createReadStream( tempPath + "/ozw.zip")
      .pipe(unzip.Extract({ path: tempPath + '/ozw' }))
      .on('close', function() {
        var sources = wrench.readdirSyncRecursive(tempPath + '/ozw/open-zwave-master/cpp');
        sources = sources.filter(function(f) {
          return f.match(/^(src|tinyxml)/) && f.match(/\.(c|cpp)$/);
        }).map(function(f) {
          return 'cpp/' + f.replace(/\\\\/g, '/').replace(/\\/g, '/');
        });
        ozwGyp.targets[0].sources = sources;
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
                //this isn't working for the build of node-ozw-shared immediately after, 
                //I think because it doesn't effect the parent environment, which is what propagates to npm install
                process.env.OZW_HOME = installPath;
              });
            });
          });
      })
      .on('error', handleError);
  });
}

if (require.main === module) {
  if (process.argv[2] === "--get-ozw-home") {
    if (process.env.OZW_HOME) {
      console.log(process.env.OZW_HOME.replace(/\\/gi,'/'));
    } else {
      console.log(installPath.replace(/\\/gi,'/'));
    }
  } else {
    module.exports({ gyp: process.argv.slice(2) });
  }
}
