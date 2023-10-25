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

const T5ManagerBase = preload("res://addons/tiltfive/T5ManagerBase.gd")

enum GameboardType {
	LE = TiltFiveXRInterface.LE_GAMEBOARD,
	XE = TiltFiveXRInterface.XE_GAMEBOARD,
	XE_Raised = TiltFiveXRInterface.XE_RAISED_GAMEBOARD,
	Unknown = TiltFiveXRInterface.NO_GAMEBOARD_SET
}

# State of a set of glasses. 
class GlassesState:
	var available := false
	var attempting_to_reserve := false
	var reserved := false
	var glasses_scene : Node
	var gameboard_type := GameboardType.Unknown
	func can_attempt_to_reserve():
		return available and (not attempting_to_reserve) and (not reserved)
		
# Dictionary maps glasses_id -> GlassesState
var glasses_dict: Dictionary

var tilt_five_xr_interface: TiltFiveXRInterface 

var t5_manager : T5ManagerBase:
	set(value):
		t5_manager = value

func get_tilt_five_xr_interface() -> TiltFiveXRInterface:
	return tilt_five_xr_interface
	
func get_setting_or_default(name : String, default):
	var val = ProjectSettings.get_setting_with_override(name)
	if not val:
		val = default
	return val

# Called when the manager is loaded and added to our scene
func _enter_tree():
	tilt_five_xr_interface = TiltFiveXRInterface.new();
	if tilt_five_xr_interface:
		tilt_five_xr_interface.application_id = T5ProjectSettings.application_id
		tilt_five_xr_interface.application_version = T5ProjectSettings.application_version

		XRServer.add_interface(tilt_five_xr_interface)
		tilt_five_xr_interface.glasses_event.connect(_on_glasses_event)
		tilt_five_xr_interface.service_event.connect(_on_service_event)

func _exit_tree():
	if tilt_five_xr_interface:
		tilt_five_xr_interface.service_event.disconnect(_on_service_event)
		tilt_five_xr_interface.glasses_event.disconnect(_on_glasses_event)
		if tilt_five_xr_interface.is_initialized():
			tilt_five_xr_interface.uninitialize()
		
		XRServer.remove_interface(tilt_five_xr_interface)
		tilt_five_xr_interface = null

func _ready():
	if not t5_manager:
		push_error("T5Manager is not set in T5Interface")
		return 
	if !tilt_five_xr_interface.is_initialized():
		tilt_five_xr_interface.initialize()

func _start_display(glasses_id : StringName, glasses_scene : Node):
	var viewport := t5_manager.get_glasses_scene_viewport(glasses_scene)
	var xr_origin := t5_manager.get_glasses_scene_origin(glasses_scene)
	tilt_five_xr_interface.start_display(glasses_id, viewport, xr_origin)
	var t5_camera := t5_manager.get_glasses_scene_camera(glasses_scene)
	if t5_camera:
		t5_camera.tracker = "/user/%s/head" % glasses_id
	for idx in range(4):
		var controller = t5_manager.get_glasses_scene_wand(glasses_scene, idx)
		if not controller: break
		controller.tracker = "/user/%s/wand_%d" % [glasses_id, idx + 1]

func _process_glasses():
	for glasses_id in glasses_dict:
		var glasses_state = glasses_dict.get(glasses_id) as GlassesState
		if glasses_state.can_attempt_to_reserve() and t5_manager.should_use_glasses(glasses_id):
			glasses_state.attempting_to_reserve = true
			tilt_five_xr_interface.reserve_glasses(glasses_id, t5_manager.get_glasses_display_name(glasses_id))

func _on_service_event(event_num):
	match event_num:
		TiltFiveXRInterface.E_SERVICE_RUNNING:
			t5_manager.service_started()
		TiltFiveXRInterface.E_SERVICE_STOPPED:
			t5_manager.service_stopped()
		TiltFiveXRInterface.E_SERVICE_T5_UNAVAILABLE:
			t5_manager.service_unvailable()
		TiltFiveXRInterface.E_SERVICE_T5_INCOMPATIBLE_VERSION:
			t5_manager.service_incorrect_version()
	
func _on_glasses_event(glasses_id, event_num):
	var glasses_state = glasses_dict.get(glasses_id) as GlassesState
	if not glasses_state:
		glasses_state = GlassesState.new()
		glasses_dict[glasses_id] = glasses_state
	match event_num:
		TiltFiveXRInterface.E_GLASSES_AVAILABLE:
			print_verbose(glasses_id, " E_AVAILABLE")
			glasses_state.available = true
			_process_glasses()

		TiltFiveXRInterface.E_GLASSES_UNAVAILABLE:
			print_verbose(glasses_id, " E_UNAVAILABLE")
			glasses_state.available = false
			if glasses_state.attempting_to_reserve:
				glasses_state.attempting_to_reserve = false
				_process_glasses()

		TiltFiveXRInterface.E_GLASSES_RESERVED:
			print_verbose(glasses_id, " E_RESERVED")
			glasses_state.reserved = true
			glasses_state.attempting_to_reserve = false
			
			var glasses_scene = t5_manager.create_glasses_scene(glasses_id)

			# instance our scene
			if glasses_scene:
				glasses_state.glasses_scene = glasses_scene
				_start_display(glasses_id, glasses_scene)
			else:
				tilt_five_xr_interface.release_glasses(glasses_id)
				
		TiltFiveXRInterface.E_GLASSES_DROPPED:
			print_verbose(glasses_id, " E_DROPPED")
			glasses_state.reserved = false

			var glasses_scene = glasses_state.glasses_scene
			if glasses_scene:
				tilt_five_xr_interface.stop_display(glasses_id)
				glasses_state.glasses_scene = null
				t5_manager.release_glasses_scene(glasses_scene)

		TiltFiveXRInterface.E_GLASSES_TRACKING:
			var gbt = tilt_five_xr_interface.get_gameboard_type(glasses_id)
			if glasses_state.gameboard_type != gbt:
				glasses_state.gameboard_type = gbt
				if glasses_state.glasses_scene:
					t5_manager.set_glasses_scene_gameboard_type(glasses_state.glasses_scene, glasses_state.gameboard_type)
			print_verbose(glasses_id, " E_TRACKING, Gameboard size = ", tilt_five_xr_interface.get_gameboard_extents(gbt))

		TiltFiveXRInterface.E_GLASSES_NOT_TRACKING:
			print_verbose(glasses_id, " E_NOT_TRACKING")

		_:
			print_verbose(glasses_id, " - unknown event: ", event_num)
