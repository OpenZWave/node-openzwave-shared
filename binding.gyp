{
  "targets": [
    {
      "target_name": "openzwave",
      "sources": [
        "src/openzwave.cc"
      ],
      "include_dirs": [
        '<!@(pkg-config libopenzwave --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config libopenzwave --cflags-only-I | sed s/-I//g)/value_classes',
      ],
	  "link_settings": {
	    "libraries": [
	      "-lopenzwave"
	     ]
	  }
    }
  ]
}
