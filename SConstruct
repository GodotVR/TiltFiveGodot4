#!/usr/bin/env python
import os
import sys
from pprint import pprint

env = SConscript('godot-cpp/SConstruct')

AddOption(
    '--no-gradle',
    dest='no_gradle',
    action='store_true',
    help='no not spawn gradle to build the android plugin')

AddOption(
    '--assemble',
    dest='assemble_example',
    action='store_true',
    help='assemble the plugin files into the example project')

AddOption(
    '--gd-install-dir',
    dest='gd_install_dir',
    type='string',
    nargs=1,
    action='store',
    metavar='DIR',
    help='godot project directory to copy the gdscript plugin to')

AddOption(
    '--cs-install-dir',
    dest='cs_install_dir',
    type='string',
    nargs=1,
    action='store',
    metavar='DIR',
    help='godot project directory to copy the c# plugin to')

gd_install_dir = GetOption('gd_install_dir')
cs_install_dir = GetOption('cs_install_dir')

assemble_example = GetOption('assemble_example') or gd_install_dir or cs_install_dir

build_aar_library = not GetOption('no_gradle') and env['platform'] == 'android'

VariantDir('build/src','extension/src', duplicate=False)
VariantDir('build/T5Integration','extension/T5Integration', duplicate=False)

tilt_five_headers_path = 'extension/TiltFiveNDK/include'
tilt_five_library_path = 'extension/TiltFiveNDK/lib/' + { 'windows' : 'win/x86_64', 'linux' : 'linux/x86_64', 'android' : 'android/arm64-v8a'}[env["platform"]]
tilt_five_library = {'windows' : 'TiltFiveNative.dll.if', 'linux' : 'libTiltFiveNative.so', 'android' : 'TiltFiveNative'}[env["platform"]]
tilt_five_jar = 'extension/TiltFiveNDK/lib/android/TiltFiveAndroidClient.jar'

bin_path = "{}/{}".format(env['platform'], env['arch'])

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
elif env['platform'] == 'linux':
    env['t5_shared_lib'] = 'libTiltFiveNative.so' 
    env['CXXFLAGS'].remove('-std=c++17')  
    env.Append(CXXFLAGS=['-std=c++20']) 
    env.Append(RPATH=env.Literal('\\$$ORIGIN' )) 
elif env['platform'] == 'android':
    env['t5_shared_lib'] = 'lib{}.so'.format(tilt_five_library)
    env['CXXFLAGS'].remove('-std=c++17')  
    env.Append(CXXFLAGS=['-std=c++20'])
    #env.Append(CPPDEFINES = ['ANDROID_CPP']) 
    env.Append(RPATH=env.Literal('\\$$ORIGIN' )) 

library = env.SharedLibrary(
    'build/bin/{}/libgdtiltfive{}{}'.format(bin_path,env['suffix'], env['SHLIBSUFFIX']),
    source=sources,
)

things_to_build = [library]

if build_aar_library:
	print("Building Android AAR library...")
	Execute(Copy('androidplugin/plugin/libs/TiltFiveAndroidClient.jar', tilt_five_jar))
	SConscript('androidplugin/SConstruct', exports="env")

if assemble_example:
	f1 = env.Command('example.gd/addons/tiltfive/bin/{}/libgdtiltfive{}{}'.format(bin_path,env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
	f2 = env.Command('example.gd/addons/tiltfive/bin/{}/{}'.format(bin_path,env['t5_shared_lib']), tilt_five_library_path + '/{}'.format(env['t5_shared_lib']), Copy('$TARGET', '$SOURCE') )
	f3 = env.Command('example.csharp/addons/tiltfive/bin/{}/libgdtiltfive{}{}'.format(bin_path,env['suffix'], env['SHLIBSUFFIX']), library, Copy('$TARGET', '$SOURCE') )
	f4 = env.Command('example.csharp/addons/tiltfive/bin/{}/{}'.format(bin_path,env['t5_shared_lib']), tilt_five_library_path + '/{}'.format(env['t5_shared_lib']), Copy('$TARGET', '$SOURCE') )
	things_to_build += [f1, f2, f3, f4]	
	if build_aar_library:
		f5 = env.Command('example.gd/addons/tiltfive/bin/android/gdtiltfive-debug.aar', 'androidplugin/plugin/build/outputs/aar/gdtiltfive-debug.aar', Copy('$TARGET', '$SOURCE') )
		f6 = env.Command('example.gd/addons/tiltfive/bin/android/gdtiltfive-release.aar', 'androidplugin/plugin/build/outputs/aar/gdtiltfive-release.aar', Copy('$TARGET', '$SOURCE') )
		f7 = env.Command('example.csharp/addons/tiltfive/bin/android/gdtiltfive-debug.aar', 'androidplugin/plugin/build/outputs/aar/gdtiltfive-debug.aar', Copy('$TARGET', '$SOURCE') )
		f8 = env.Command('example.csharp/addons/tiltfive/bin/android/gdtiltfive-release.aar', 'androidplugin/plugin/build/outputs/aar/gdtiltfive-release.aar', Copy('$TARGET', '$SOURCE') )
		things_to_build += [f5, f6, f7, f8]

if gd_install_dir:
	inst1 = env.Install("{}/addons".format(gd_install_dir) , "example.gd/addons/tiltfive")
	things_to_build += [inst1]

if cs_install_dir:
	inst2 = env.Install("{}/addons".format(cs_install_dir) , "example.csharp/addons/tiltfive")
	things_to_build += [inst2]

Default(things_to_build)

