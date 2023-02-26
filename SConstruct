#!/usr/bin/env python
import os
import sys

tilt_five_headers_path = "extension/TiltFiveNDK/include/include"
tilt_five_library_path = "extension/TiltFiveNDK/lib/win64"
tilt_five_library = "TiltFiveNative.dll.if"

VariantDir("build/src","extension/src", duplicate=False)
VariantDir("build/T5Integration","extension/T5Integration", duplicate=False)

env = SConscript("godot-cpp/SConstruct")
env['CXXFLAGS'].remove('/std:c++17')

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["extension/src/","extension/T5Integration/",tilt_five_headers_path])
sources = Glob("build/src/*.cpp")
sources += Glob('build/T5Integration/*.cpp')

env.Append(LIBPATH=[tilt_five_library_path])
env.Append(LIBS=[tilt_five_library, "Opengl32"])

if env['platform'] == "windows":
    env.Append(CXXFLAGS=['/std:c++20', '/Zc:__cplusplus'])
    library = env.SharedLibrary(
        "build/bin/libgdtiltfive{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

f1 = env.Command("example/addons/tiltfive/libgdtiltfive{}{}".format(env["suffix"], env["SHLIBSUFFIX"]), library, Copy('$TARGET', '$SOURCE') )
f2 = env.Command("example/addons/tiltfive/TiltFiveNative.dll", "extension/TiltFiveNDK/lib/win64/TiltFiveNative.dll", Copy('$TARGET', '$SOURCE') )

env.Alias('example', [f1, f2])

Default(library)
