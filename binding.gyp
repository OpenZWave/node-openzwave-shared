{
	"targets": [{
		"target_name": "openzwave",
		"sources": [
			"src/openzwave.cc"
		],
		'conditions': [
			['OS=="linux"', {
				"link_settings": {
					"libraries": ["-lopenzwave"]
				},
				"include_dirs": [
					'/usr/local/include/openzwave',
					'/usr/local/include/openzwave/value_classes',
					'/usr/include/openzwave',
					'/usr/include/openzwave/value_classes',
				],
				"cflags": [ "-Wno-ignored-qualifiers" ],
			}],
			['OS=="win"', {
				"include_dirs": [ 
					"deps/open-zwave/cpp/src/platform/windows" 
				],
				'msvs_settings': {
					'VCLinkerTool': {
						'AdditionalDependencies': ['setupapi.lib']
					}
				}
			}]
		],
	}]
}
