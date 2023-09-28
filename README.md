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

## Starting with a new project

To use this plugin in your own project:
- Copy the `addons/tiltfive` folder into your project
- Open Project->Project Settings
  - Click on the Plugins tab 
  - Make sure the Tilt Five plugin is enabled
  - You may need to restart Godot
- In the main scene add a T5Manager node
- In the main scene add a T5Gameboard node
- On the T5Manager node set the start location to the T5Gameboard node
- Add lights and other items to your scene

Running should now show your scene on the Tilt Five system

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK


