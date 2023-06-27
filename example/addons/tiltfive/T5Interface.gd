@tool
extends Node

## This script should be configured as an autoload script.
## It will instantiate the TileFive interface and register
## it with the XRServer.
##
## It also registers various project settings the user can
## setup.
##
## Note that the TiltFive interface is also registed when
## in editor. This will allow the Godot editor to request
## action information from the interface.

var tilt_five_xr_interface: TiltFiveXRInterface 

func get_tile_five_xr_interface() -> TiltFiveXRInterface:
	return tilt_five_xr_interface

func _define_project_setting(
		p_name : String,
		p_type : int,
		p_hint : int = PROPERTY_HINT_NONE,
		p_hint_string : String = "",
		p_default_val = "") -> void:
	# p_default_val can be any type!!

	if !ProjectSettings.has_setting(p_name):
		ProjectSettings.set_setting(p_name, p_default_val)

	var property_info : Dictionary = {
		"name" : p_name,
		"type" : p_type,
		"hint" : p_hint,
		"hint_string" : p_hint_string
	}

	ProjectSettings.add_property_info(property_info)
	ProjectSettings.set_as_basic(p_name, true)
	ProjectSettings.set_initial_value(p_name, p_default_val)

# Called when the manager is loaded and added to our scene
func _enter_tree():
	_define_project_setting("xr/tilt_five/application_id", TYPE_STRING, PROPERTY_HINT_NONE, "", "my.game.com")
	_define_project_setting("xr/tilt_five/application_version", TYPE_STRING, PROPERTY_HINT_NONE, "", "0.1.0")
	_define_project_setting("xr/tilt_five/default_display_name", TYPE_STRING, PROPERTY_HINT_NONE, "", "Game: Player One")

	tilt_five_xr_interface = TiltFiveXRInterface.new();
	if tilt_five_xr_interface:
		tilt_five_xr_interface.application_id = ProjectSettings.get_setting_with_override("xr/tilt_five/application_id")
		tilt_five_xr_interface.application_version = ProjectSettings.get_setting_with_override("xr/tilt_five/application_version")

		XRServer.add_interface(tilt_five_xr_interface)

func _exit_tree():
	if tilt_five_xr_interface:
		if tilt_five_xr_interface.is_initialized():
			tilt_five_xr_interface.uninitialize()

		XRServer.remove_interface(tilt_five_xr_interface)
		tilt_five_xr_interface = null
