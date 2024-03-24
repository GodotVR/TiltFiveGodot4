@tool
extends EditorPlugin

static var _initialized := false

var export_plugin: AndroidExportPlugin

func _setup():
	if not _initialized:
		add_autoload_singleton("T5Interface", "res://addons/tiltfive/T5Interface.gd")
		_initialized = true
	
func _enter_tree():
	export_plugin = AndroidExportPlugin.new()
	add_export_plugin(export_plugin)
	T5ProjectSettings.setup_properties()
	_setup()

func _exit_tree():
	remove_export_plugin(export_plugin)
	export_plugin = null

func _enable_plugin():
	_setup()

func _disable_plugin():
	if _initialized:
		remove_autoload_singleton("T5Interface")

class AndroidExportPlugin extends EditorExportPlugin:
	var _plugin_name = "gdtiltfive"

	func _supports_platform(platform):
		if platform is EditorExportPlatformAndroid:
			return true
		return false

	func _get_android_libraries(platform, debug):
		if debug:
			return PackedStringArray(["tiltfive/bin/android/" + _plugin_name + "-debug.aar"])
		else:
			return PackedStringArray(["tiltfive/bin/android/" + _plugin_name + "-release.aar"])

	func _get_name():
		return _plugin_name
