# TiltFiveGodot4

TiltFiveGodot4 is **GDExtension** for the Godot 4 engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It extends Godot's **XRInterface** and has functions to connect glasses and signals for connection events.

This extension is not ready for general use yet. It is incomplete and currently requires a custom build of Godot.

Godot must be run with the Compatibility renderer because Tilt Five does not currently support Vulcan.

## Platforms

Currently only Windows 10/11 is supported. Tilt Five Linux and Android support are recent additions and and support 
for those platforms will investigated. 

## Build

### Prerequisites

Things you will need to know how to do.
* Use [scons](https://scons.org/) 
* Apply patches
* [Build a custom version of Godot](https://docs.godotengine.org/en/stable/contributing/development/compiling/index.html)
* [Build the C++ bindings](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html) for GDExtensions
* [Build GDExtensions](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)

### Building the extension

Get godot source and patch from the patches directory.

For the extension godot_cpp will need to be built with the extension api json file dumps from the custom godot build.

Scons should be run from an environment that has the Microsoft x64 development tools setup.

> `scons target=[template_debug | template_release]` Build the shared library. Result is in `build\bin`

> `scons example target=[template_debug | template_release]` Copy build products to the `example\addons\tilt-five`

## Basic usage 

- Open the custom version of Godot
- Open the project in the examples directory
- Run the default scene

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Wand tacking
- Input
- Support for multiple glasses
- Better docs and examples

## Acknowledgments

This was written by referring a lot to [GodotVR](https://github.com/GodotVR) code and reading 
[Godot's](https://github.com/godotengine/godot) source code. 
