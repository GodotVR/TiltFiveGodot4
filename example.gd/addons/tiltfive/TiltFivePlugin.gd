@tool
extends EditorPlugin

static var _initialized := false

func _setup():
	if not _initialized:
		#add_custom_type("T5Manager", "Node", preload("res://addons/tiltfive/T5Manager.gd"), preload("res://addons/tiltfive/assets/glasses.svg"))
		add_autoload_singleton("T5Interface", "res://addons/tiltfive/T5Interface.gd")
		_initialized = true
	
func _enter_tree():
	T5ProjectSettings.setup_properties()
	_setup()

func _enable_plugin():
	_setup()

func _disable_plugin():
	if _initialized:
		#remove_custom_type("T5Manager")
		remove_autoload_singleton("T5Interface")

