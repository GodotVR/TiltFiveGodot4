# TiltFiveGodot4

TiltFiveGodot4 is **GDExtension** for the Godot 4 engine to connect to the [Tilt Five](https://www.tiltfive.com/) system. It extends Godot's **XRInterface** and adds T5 Nodes to handle the creation of the XR rigs in the scene.

## Platforms

| | Renderer</br> Forward+ | Renderer</br> Mobile | Renderer</br> Compatibility |
| - | - | - | - |
| Windows | ✅ | ✅ | ✅ |
| Linux<sup>1</sup> | ✅ |✅ | ✅ |
| Android<sup>1</sup> |&#10060;| &#10060; | ✅ |

 1. Experimental Support

## Usage

TiltFiveGodot4 is available in the Godot Asset Library. 

Installable [Releases can also be found here](https://github.com/GodotVR/TiltFiveGodot4/releases). 
- Use `gdtiltfive_gdscript.zip` for gdscript projects
- Use `gdtiltfive_csharp.zip` for C# projects

Please refer to this [documentation](https://patrickdown.github.io/godot/tilt-five-godot-4.html) for installation.

## Build

### Prerequisites

This extension requires a C++20 capable compiler. To build for Android you will need Java 17 and the Android SDK along with the r26c NDK.

Things you will need to know how to do.
* Use [scons](https://scons.org/) 
* [Build GDExtensions](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)
* Install the Android SDK and NDK (For android builds)

### Building the extension

To build the plugin invoke `scons` from the root directory of the project. The build product will in `build\bin`.  Invoking `scons --assemble` will build the product and copy the binaries to the `example.gd\addons\tilt-five\bin` and `example.csharp\addons\tilt-five\bin` directories. 

## Using the build products

When built with the `example` option the `addons\tilt-five` directory can be copied from the `example.gd` or `example.csharp` directories into the root directory of a new Godot project.



