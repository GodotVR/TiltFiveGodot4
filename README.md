# TiltFiveGodot4

TiltFiveGodot4 is **GDExtension** for the Godot 4 engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It extends Godot's **XRInterface** and has functions to connect glasses and signals for connection events.

This extension is in an Alpha state and needs that latest Godot 4.1 to run. It currently needs the OpenGL renderer
Vulkan support is coming. 

## Platforms

Currently only Windows 10/11 is supported. Tilt Five Linux and Android support are recent additions and and support 
for those platforms will investigated. 

## Build

### Prerequisites

Things you will need to know how to do.
* Use [scons](https://scons.org/) 
* [Build GDExtensions](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)

### Building the extension

Scons should be run from an environment that has the Microsoft x64 development tools setup.

> `scons target=[template_debug | template_release]` Build the shared library. Result is in `build\bin`

> `scons example target=[template_debug | template_release]` Copy build products to the `example\addons\tilt-five\bin`

## Basic usage 

- Open the project in the examples directory
- Run the default scene

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Vulkan support
- API for tangible camera on the glasses
- Better docs
- Examples


## Acknowledgments

This was written by referring a lot to [GodotVR](https://github.com/GodotVR) code and reading 
[Godot's](https://github.com/godotengine/godot) source code. 
