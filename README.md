# TiltFiveGodot

TiltFiveGodot is **GDNative** extension for the Godot engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It implements Godot's **ARVRinterface** and has GDNative class called TiltFiveManager for scripts to connect 
glasses and handle connection events.

## Platforms

Currently only Windows 10/11 is supported because that is the only platform supported by Tilt Five. T5 linux support
is supposed to come at some point in the future and support for that platform will revisited when it becomes available. 

## Build

### Prerequisites

Make sure you have [SCons](https://scons.org/) installed and a C++20 compatible 
compiler like Visual C++ 2022. 

After cloning this repository be sure to get and build the submodules

```
git submodule init
git submodule update
cd godot-cpp
git submodule init
git submodule update
scons
scons target=release
```

### Building the extension

Scons should be run from an environment that has the Microsoft x64 development tools setup.

> `scons target=[debug | release]` Build the shared library. Result is in `build\bin`

> `scons example target=[debug | release]` Copy build products to the `example\addons\tilt-five`

> `scons zip target=[debug | release]` Create a zip archive of `example\addons`

Note that currently due to bugs the zip archive is not compatible with godot's import function.

## Basic usage 

After building the example you should be able to open the Godot project in the example directory
and run the default scene.

If you want to use the extension in your own project follow these steps.

1) Copy `example\addons` to your own projects root directory. 

2) From `addons\tilt-five\scenes` load the `t5-scene` and run. 

3) From here you should be able to follow documentation for usage of Godot's AR/VR system.

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Better docs and examples

## Acknowledgments

This was written by referring a lot to [GodotVR](https://github.com/GodotVR) code and reading 
[Godot's](https://github.com/godotengine/godot) source code. 
