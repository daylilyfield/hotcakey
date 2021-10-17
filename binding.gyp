{
    "targets": [
        {
            "target_name": "hotcakey",

            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],

            "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],

            "defines": ["NAPI_VERSION=<(napi_build_version)"],

            "conditions": [
                [
                    "OS=='win'",
                    {
                        "defines": ["_HAS_EXCEPTIONS=1"],
                        "sources": [
                            "src/addon.cc",
                            "src/hotcakey/hotcakey.win.cc",
                            "src/hotcakey/utils/strings.cc",
                            "src/hotcakey/utils/logger.cc"
                        ],
                        "msvs_settings": {
                            "ClCompile": {
                                "LanguageStandard": "stdcpp17"
                            },
                            "VCCLCompilerTool": {
                                "ExceptionHandling": 1,
                                "AdditionalOptions": ["-std:c++17"]
                            }

                        }
                    }
                ],
                [
                    "OS=='mac'",
                    {
                        "sources": [
                            "src/addon.cc",
                            "src/hotcakey/hotcakey.mac.cc",
                            "src/hotcakey/utils/strings.cc",
                            "src/hotcakey/utils/logger.cc"
                        ],
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
                        "xcode_settings": {
                            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                            "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",
                            "MACOSX_DEPLOYMENT_TARGET": "10.7",
                            "CLANG_CXX_LIBRARY": "libc++",
                            "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
                        }
                    }
                ]
            ]
        }
    ]
}
