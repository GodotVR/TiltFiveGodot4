# Patches for Godot

These patches need to be applied to godot/master for TiltFiveGodot4 to work.

`d43f2c.patch` Adds a function to create multiview textures for stereo rendering
`26ff6a.patch` Adds functions to copy texture layers into textures suitable for sending to Tilt Five glasses

This folder also contains a copy of the `extension_api.json` that includes these functions.
This is included for CI purposes.
You should create your own `extension_api.json` after building Godot from source.
