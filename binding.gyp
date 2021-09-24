{
  "targets": [
    {
      "target_name": "hotcakey",

      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],

      "sources": ["src/addon.cc", "src/hotcakey/hotcakey.mac.cc", "src/hotcakey/utils/strings.cc"],

      "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],

      "defines": ["NAPI_ENABLE_CPP_EXCEPTIONS"],

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

      "conditions": [
        [
          "OS=='win'",
          {
            "defines": ["_HAS_EXCEPTIONS=1"],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": ["-std:c++17"]
              }
            },
            "msbuild_settings": {
              "ClCompile": {
                "LanguageStandard": "stdcpp17"
              }
            }
          }
        ],
        [
          "OS=='mac'",
          {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
            }
          }
        ]
      ]
    }
  ]
}
