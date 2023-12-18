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
		_define_project_setting("xr/tilt_five/default_display_name", TYPE_STRING)
		_define_project_setting("xr/tilt_five/trigger_click_threshhold", TYPE_FLOAT, PROPERTY_HINT_RANGE, "0,1,0.01", 0.3)
		_define_project_setting("xr/tilt_five/debug_logging", TYPE_BOOL, PROPERTY_HINT_NONE, "", false)
		_initialized = true

static var application_id : String:
	get:
		var app_id := ProjectSettings.get_setting_with_override("application/config/name")
		if not app_id or app_id == "":
			return "tiltfive.godot.game"
		return app_id

static var application_version : String:
	get:
		var version := ProjectSettings.get_setting_with_override("application/config/version")
		if not version or version == "":
			return "unknown"
		return version
		
static var default_display_name : String:
	get:
		setup_properties()
		var disp_name := ProjectSettings.get_setting_with_override("xr/tilt_five/default_display_name")
		if not disp_name or disp_name == "":
			return T5ProjectSettings.application_id
		return disp_name

static var trigger_click_threshhold : float:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/trigger_click_threshhold")

static var is_debug_logging : bool:
	get:
		setup_properties()
		return ProjectSettings.get_setting_with_override("xr/tilt_five/debug_logging")
