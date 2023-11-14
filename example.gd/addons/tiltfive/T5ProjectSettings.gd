@tool
class_name T5ProjectSettings extends Object

static var _initialized := false

static func _define_project_setting(
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

static func setup_properties():
	if not _initialized:
		_define_project_setting("xr/tilt_five/application_id", TYPE_STRING, PROPERTY_HINT_NONE, "", "my.game.com")
		_define_project_setting("xr/tilt_five/application_version", TYPE_STRING, PROPERTY_HINT_NONE, "", "0.1.0")
		_define_project_setting("xr/tilt_five/default_display_name", TYPE_STRING, PROPERTY_HINT_NONE, "", "Game: Player One")
		_define_project_setting("xr/tilt_five/trigger_click_threshhold", TYPE_FLOAT, PROPERTY_HINT_RANGE, "0,1,0.01", 0.3)
		_initialized = true

static var application_id : String:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/application_id")

static var application_version : String:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/application_version")

static var default_display_name : String:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/default_display_name")

static var trigger_click_threshhold : float:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/trigger_click_threshhold")
