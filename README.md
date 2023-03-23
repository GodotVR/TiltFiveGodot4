# TiltFiveGodot

TiltFiveGodot is **GDExtension** for the Godot engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It extends Godot's **XRInterface** and has functions to connect glasses and signals for connection events.

This extension is not ready for general use yet. It is incomplete and currently requires a custom build of Godot.

## Platforms

Currently only Windows 10/11 is supported because that is the only platform supported by Tilt Five. T5 linux support
is supposed to come at some point in the future and support for that platform will revisited when it becomes available. 

## Build

### Prerequisites

Things you will need to know how to do.
* Use [scons](https://scons.org/) 
* Apply patches
* [Build a custom version of Godot](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html)
* [Build the C++ bindings](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html) for GDExtensions
* [Build GDExtensions](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)



### Building the extension

Scons should be run from an environment that has the Microsoft x64 development tools setup.

> `scons target=[debug | release]` Build the shared library. Result is in `build\bin`

> `scons example target=[debug | release]` Copy build products to the `example\addons\tilt-five`

## Basic usage 

Use the custom built version of Godot.

After building the example you should be able to open the Godot project in the example directory
and run the default scene.


## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Wand tacking
- Input
- Better docs and examples

## Acknowledgments

This was written by referring a lot to [GodotVR](https://github.com/GodotVR) code and reading 
[Godot's](https://github.com/godotengine/godot) source code. 
