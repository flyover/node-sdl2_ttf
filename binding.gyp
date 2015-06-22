{
    'targets':
    [
        {
            'target_name': "node-sdl2_ttf",
            'include_dirs':
            [
                "<(module_root_dir)",
                "<!(node -e \"require('@flyover/node-sdl2/include')\")",
                "<!(node -e \"require('nan')\")"
            ],
            'sources':
            [
                "node-sdl2_ttf.cc"
            ],
            'conditions':
            [
                [
                    'OS=="linux"',
                    {
                        'include_dirs': [ "/usr/local/include/SDL2", "/usr/local/include/SDL2_ttf" ],
                        'cflags': [ "<!@(sdl2-config --cflags)" ],
                        'ldflags': [ "<!@(sdl2-config --libs)" ],
                        'libraries': [ "-lSDL2", "-lSDL2_ttf" ]
                    }
                ],
                [
                    'OS=="mac"',
                    {
                        'include_dirs': [ "/usr/local/include/SDL2", "/usr/local/include/SDL2_ttf" ],
                        'cflags': [ "<!@(sdl2-config --cflags)" ],
                        'ldflags': [ "<!@(sdl2-config --libs)" ],
                        'libraries': [ "-L/usr/local/lib", "-lSDL2", "-lSDL2_ttf" ]
                    }
                ],
                [
                    'OS=="win"',
                    {
                        'include_dirs': [ "$(SDL2_ROOT)/include", "$(SDL2_TTF_ROOT)/include" ],
                        'libraries':
                        [
                            "$(SDL2_ROOT)/lib/x64/SDL2.lib",
                            "$(SDL2_TTF_ROOT)/lib/x64/SDL2_ttf.lib"
                        ],
                        'copies':
                        [
                            {
                                'destination': "<!(echo %USERPROFILE%)/bin",
                                'files':
                                [
                                    "$(SDL2_TTF_ROOT)/lib/x64/SDL2_ttf.dll",
                                    "$(SDL2_TTF_ROOT)/lib/x64/libfreetype-6.dll",
                                    "$(SDL2_TTF_ROOT)/lib/x64/zlib1.dll"
                                ]
                            }
                        ]
                    }
                ]
            ]
        }
    ]
}
