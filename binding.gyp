{
	"targets": [{
		"target_name": "openzwave_shared",
		"sources": [
			"src/callbacks.cc",
			"src/openzwave.cc",
			"src/openzwave-config.cc",
			"src/openzwave-driver.cc",
			"src/openzwave-groups.cc",
			"src/openzwave-management.cc",
			"src/openzwave-network.cc",
			"src/openzwave-nodes.cc",
			"src/openzwave-polling.cc",
			"src/openzwave-scenes.cc",
			"src/openzwave-values.cc",
			"src/utils.cc",
		],
		"conditions": [
			['OS=="solaris"', {
				"variables": {
					"OZW_INC"         : "<!(pkg-config --cflags-only-I libopenzwave | sed s/-I//g)",
					"OZW_LIB_PATH"    : "<!(pkg-config --variable=libdir libopenzwave)",
					"OZW_GITVERSION"  : "<!(pkg-config --variable=gitversion libopenzwave)",
					"OZW_ETC"         : "<!(pkg-config --variable=sysconfdir libopenzwave)",
					"OZW_DOC"         : "<!(pkg-config --variable=docdir libopenzwave)"
				},
				"defines": [
					"OPENZWAVE_ETC=<(OZW_ETC)/config",
					"OPENZWAVE_DOC=<!@(node -p -e \"'<(OZW_DOC)'.length ? '<(OZW_DOC)' : '/usr/local/share/doc/openzwave'\")",
					"OPENZWAVE_SECURITY=<!@(find <(OZW_INC) -name ZWSecurity.h | wc -l)",
					"OPENZWAVE_EXCEPTIONS=<!@(find <(OZW_INC) -name OZWException.h | wc -l)",
					"OPENZWAVE_BITSET=<!@(find <(OZW_INC) -name ValueBitSet.h | wc -l)"
				],
				"cflags": [ "-Wno-ignored-qualifiers -Wno-write-strings -Wno-unknown-pragmas" ],
				"link_settings": {
				    "libraries": [
						"-R/opt/local/lib/", "-L/opt/local/lib/", "-lopenzwave"
					]
				},
				"include_dirs": [
				    "<!(node -p -e \"require('path').dirname(require.resolve('nan'))\")",
				    '/opt/local/include/openzwave/',
				    '/opt/local/include/openzwave/value_classes/'
				]
			}],
			['OS=="mac"', {
				"variables": {
					"OZW_INC"         : "<!(pkg-config --cflags-only-I libopenzwave | sed s/-I//g)",
					"OZW_LIB_PATH"    : "<!(pkg-config --variable=libdir libopenzwave)",
					"OZW_GITVERSION"  : "<!(pkg-config --variable=gitversion libopenzwave)",
					"OZW_ETC"         : "<!(pkg-config --variable=sysconfdir libopenzwave)",
					"OZW_DOC"         : "<!(pkg-config --variable=docdir libopenzwave)"
				},
				"defines": [
					"OPENZWAVE_ETC=<(OZW_ETC)/config",
					"OPENZWAVE_DOC=<!@(node -p -e \"'<(OZW_DOC)'.length ? '<(OZW_DOC)' : '/usr/local/share/doc/openzwave'\")",
					"OPENZWAVE_SECURITY=<!@(find <(OZW_INC) -name ZWSecurity.h | wc -l)",
					"OPENZWAVE_EXCEPTIONS=<!@(find <(OZW_INC) -name OZWException.h | wc -l)",
					"OPENZWAVE_BITSET=<!@(find <(OZW_INC) -name ValueBitSet.h | wc -l)",
					"OPENZWAVE_VALUETYPE_FROM_ENUM=<!@(ldconfig -p | grep 'libopenzwave.so ' | awk '{print $4}' | xargs nm -g --demangle |grep OpenZWave::Value::GetTypeNameFromEnum | wc -l)",
					"OPENZWAVE_VALUETYPE_FROM_VALUEID=<!@(ldconfig -p | grep 'libopenzwave.so ' | awk '{print $4}' | xargs nm -g --demangle |grep OpenZWave::ValueID::GetTypeAsString | wc -l)",
				],
				"link_settings": {
				    "libraries": [
						"-L/usr/local/lib/", "-lopenzwave"
					]
				},
				"include_dirs": [
				    "<!(node -p -e \"require('path').dirname(require.resolve('nan'))\")",
				    '/usr/local/include/openzwave/',
				    '/usr/local/include/openzwave/value_classes/'
				],
				'xcode_settings': {
					'MACOSX_DEPLOYMENT_TARGET':'10.9',
					'OTHER_CFLAGS': [
						'-Wno-ignored-qualifiers -Wno-write-strings -Wno-unknown-pragmas'
					]
				}
			}],
			["OS=='linux'", {
				"variables": {
					"NODE"            : "<!(which node || which nodejs)",
					"OZW_INC"         : "<!(<(NODE) -p \"require('./lib/ozwpaths.js').includedir || '/usr/*/include'\")",
					"OZW_LIB_PATH"    : "<!(<(NODE) -p \"require('./lib/ozwpaths.js').libdir\")",
					"OZW_ETC"         : "<!(<(NODE) -p \"require('./lib/ozwpaths.js').sysconfdir\")",
					"OZW_DOC"         : "<!(<(NODE) -p \"require('./lib/ozwpaths.js').docdir\")"
				},
				"defines": [
					"OPENZWAVE_ETC=<!@(<(NODE) -p -e \"'<(OZW_ETC)'.length ? '<(OZW_ETC)' : '/usr/local/etc/openzwave'\")",
					"OPENZWAVE_DOC=<!@(<(NODE) -p -e \"'<(OZW_DOC)'.length ? '<(OZW_DOC)' : '/usr/local/share/doc/openzwave'\")",
					"OPENZWAVE_SECURITY=<!@(find <(OZW_INC) -name ZWSecurity.h | wc -l)",
					"OPENZWAVE_EXCEPTIONS=<!@(find <(OZW_INC) -name OZWException.h | wc -l)",
					"OPENZWAVE_BITSET=<!@(find <(OZW_INC) -name ValueBitSet.h | wc -l)",
					"OPENZWAVE_VALUETYPE_FROM_ENUM=<!@(ldconfig -p | grep 'libopenzwave.so ' | awk '{print $4}' | xargs nm -g --demangle |grep OpenZWave::Value::GetTypeNameFromEnum | wc -l)",
					"OPENZWAVE_VALUETYPE_FROM_VALUEID=<!@(ldconfig -p | grep 'libopenzwave.so ' | awk '{print $4}' | xargs nm -g --demangle |grep OpenZWave::ValueID::GetTypeAsString | wc -l)",
				],
				"link_settings": {
					"libraries": ["-lopenzwave"]
				},
				"include_dirs": [
					"<!(<(NODE) -p -e \"require('path').dirname(require.resolve('nan'))\")",
					"<(OZW_INC)",
					"<(OZW_INC)/value_classes"
				],
				"cflags": [ 
					"-Wno-ignored-qualifiers",
					"-Wno-write-strings",
					"-Wno-unknown-pragmas",
					"<!@(find <(OZW_INC) -name OZWException.h -exec echo -n \"-fexceptions\" \\;)"
				],
				"cflags_cc!": [ 
					"<!@(find <(OZW_INC) -name OZWException.h -exec echo -n \"-fno-exceptions\" \\;)"
				]
			}],
			['OS=="win"', {
				"variables": {
					"OZW_HOME": "<!(node lib/install-ozw.js --get-ozw-home)"
				},
				"include_dirs": [
					"<!(node -e \"require('nan')\")",
					"<(OZW_HOME)/include",
					"<(OZW_HOME)/include/value_classes"
				],
				"defines": [
					"OPENZWAVE_ETC=<(OZW_HOME)/config",
					"OPENZWAVE_SECURITY=1",
					"OPENZWAVE_EXCEPTIONS=1",
					"OPENZWAVE_BITSET=1",
					"OPENZWAVE_VALUETYPE_FROM_VALUEID=1"
				],
				'msvs_settings': {
					'VCCLCompilerTool': {
						'ExceptionHandling': 1
					},
					'VCLinkerTool': {
						'AdditionalDependencies': ['setupapi.lib', '<(OZW_HOME)/bin/OpenZWave.lib', 'dnsapi.lib', 'ws2_32.lib']
					}
				},
			    "link_settings": {
					"libraries": ["dnsapi.lib", "ws2_32.lib"]
		        }
			}]
		]
	}]
}
