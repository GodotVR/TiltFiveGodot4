#!/usr/bin/env python
import os
import sys



VariantDir('build/src','extension/src', duplicate=False)
VariantDir('build/T5Integration','extension/T5Integration', duplicate=False)

env = SConscript('godot-cpp/SConstruct')
tilt_five_headers_path = 'extension/TiltFiveNDK/include'
tilt_five_library_path = 'extension/TiltFiveNDK/lib/' + { 'windows' : 'win/x86_64', 'linux' : 'linux/x86_64', 'android' : 'android/arm64-v8a'}[env["platform"]]
tilt_five_library = {'windows' : 'TiltFiveNative.dll.if', 'linux' : 'libTiltFiveNative.so', 'android' : 'libTiltFiveNative.so'}[env["platform"]]

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
    env['t5_shared_lib'] = 'TiltFiveNative.dll' 
    env['CXXFLAGS'].remove('/std:c++17')  
    env.Append(CXXFLAGS=['/std:c++20'])  
    env.Append(CXXFLAGS=['/Zc:__cplusplus'])
    library = env.SharedLibrary(
        'build/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']),
        source=sources,
    )
elif env['platform'] == 'linux':
    env['t5_shared_lib'] = 'libTiltFiveNative.so' 
    env['CXXFLAGS'].remove('-std=c++17')  
    env.Append(CXXFLAGS=['-std=c++20']) 
    env.Append(RPATH=env.Literal('\\$$ORIGIN' )) 
    library = env.SharedLibrary(
        'build/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']),
        source=sources,
    )
elif env['platform'] == 'android':
    env['t5_shared_lib'] = 'libTiltFiveNative.so' 
    env['CXXFLAGS'].remove('-std=c++17')  
    env.Append(CXXFLAGS=['-std=c++20'])
    env.Append(CXXFLAGS=['-stdlib=libc++'])  
    env.Append(CCFLAGS=['-fPIC']) 
    env.Append(RPATH=env.Literal('\\$$ORIGIN' )) 
    library = env.SharedLibrary(
        'build/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']),
        source=sources,
    )

f1 = env.Command('example.gd/addons/tiltfive/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
f2 = env.Command('example.gd/addons/tiltfive/bin/{}'.format(env['t5_shared_lib']), tilt_five_library_path + '/{}'.format(env['t5_shared_lib']), Copy('$TARGET', '$SOURCE') )
f3 = env.Command('example.csharp/addons/tiltfive/bin/libgdtiltfive{}{}'.format(env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
f4 = env.Command('example.csharp/addons/tiltfive/bin/{}'.format(env['t5_shared_lib']), tilt_five_library_path + '/{}'.format(env['t5_shared_lib']), Copy('$TARGET', '$SOURCE') )

env.Alias('example', [f1, f2, f3, f4])

Default(library)
