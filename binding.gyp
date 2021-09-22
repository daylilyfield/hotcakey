{
  "targets": [
    {
      "target_name": "hotcakey",

      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "src/addon.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],

      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],

      "link_settings": {
		  	"libraries": [
		  		"-framework Carbon",
		  		"-framework ApplicationServices",
		  		"-lobjc",
		  		"-Wl,-rpath,@executable_path/.",
		  		"-Wl,-rpath,@loader_path/.",
		  		"-Wl,-rpath,<!(pwd)/build/Release/"
		  	]
		  },

      'xcode_settings': {
        'OTHER_CFLAGS': [
          "-std=c++11",
          "-stdlib=libc++"
        ],
      }
    }
  ]
}
