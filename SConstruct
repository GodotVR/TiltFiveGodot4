#!/usr/bin/env python
import os
import sys



VariantDir('build/src','extension/src', duplicate=False)
VariantDir('build/T5Integration','extension/T5Integration', duplicate=False)

env = SConscript('godot-cpp/SConstruct')
env['CXXFLAGS'].remove('/std:c++17')
env.Append(CXXFLAGS=['/std:c++20'])
tilt_five_headers_path = 'extension/TiltFiveNDK/include'
tilt_five_library_path = 'extension/TiltFiveNDK/lib/' + { 'windows' : 'win/x86_64', 'linux' : 'linux/x86_64'}[env["platform"]]
tilt_five_library = 'TiltFiveNative.dll.if'

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['extension/src/','extension/T5Integration/',tilt_five_headers_path])
sources = Glob('build/src/*.cpp')
sources += Glob('build/T5Integration/*.cpp')

env.Append(LIBPATH=[tilt_five_library_path])
env.Append(LIBS=[tilt_five_library])

if env['platform'] == 'windows':
    env.Append(CXXFLAGS=['/Zc:__cplusplus'])
    library = env.SharedLibrary(
        'build/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']),
        source=sources,
    )

f1 = env.Command('example.gd/addons/tiltfive/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
f2 = env.Command('example.gd/addons/tiltfive/bin/TiltFiveNative.dll', tilt_five_library_path + '/TiltFiveNative.dll', Copy('$TARGET', '$SOURCE') )
f3 = env.Command('example.csharp/addons/tiltfive/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
f4 = env.Command('example.csharp/addons/tiltfive/bin/TiltFiveNative.dll', tilt_five_library_path + '/TiltFiveNative.dll', Copy('$TARGET', '$SOURCE') )

env.Alias('example', [f1, f2, f3, f4])

Default(library)
