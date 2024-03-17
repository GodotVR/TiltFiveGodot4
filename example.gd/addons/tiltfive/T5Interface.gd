extends Node
## It will instantiate the TiltFive interface and register it with the XRServer.
##
## This script should be configured be automatically added as an autoload script
## when the plugin is enabled. This  

const T5ManagerBase = preload("res://addons/tiltfive/T5ManagerBase.gd")

# State of a set of glasses. 
class XRRigState:
	var available := false
	var attempting_to_reserve := false
	var reserved := false
	var xr_rig : T5XRRig
	func can_attempt_to_reserve():
		return available and (not attempting_to_reserve) and (not reserved)
		
# Dictionary maps glasses_id -> XRRigState
var id_to_state: Dictionary

var tilt_five_xr_interface: TiltFiveXRInterface 

var t5_manager : T5ManagerBase:
	set(value):
		t5_manager = value

func get_tilt_five_xr_interface() -> TiltFiveXRInterface:
	return tilt_five_xr_interface
		
func _enter_tree():
	process_mode = Node.PROCESS_MODE_ALWAYS
	_create_xr_interface()

func _exit_tree():
	_destroy_xr_interface()

func _notification(what):
	match what:
		NOTIFICATION_APPLICATION_PAUSED:
			_destroy_xr_interface()
		NOTIFICATION_APPLICATION_RESUMED:
			_create_xr_interface()
			if !tilt_five_xr_interface.is_initialized():
				tilt_five_xr_interface.initialize()
				
func _create_xr_interface():
	if (tilt_five_xr_interface): return
	tilt_five_xr_interface = TiltFiveXRInterface.new()
	if tilt_five_xr_interface:
		tilt_five_xr_interface.application_id = T5ProjectSettings.application_id
		tilt_five_xr_interface.application_version = T5ProjectSettings.application_version
		tilt_five_xr_interface.trigger_click_threshold = T5ProjectSettings.trigger_click_threshhold
		tilt_five_xr_interface.debug_logging = T5ProjectSettings.is_debug_logging

		XRServer.add_interface(tilt_five_xr_interface)
		tilt_five_xr_interface.glasses_event.connect(_on_glasses_event)
		tilt_five_xr_interface.service_event.connect(_on_service_event)

func _destroy_xr_interface():
	if tilt_five_xr_interface:
		tilt_five_xr_interface.service_event.disconnect(_on_service_event)
		tilt_five_xr_interface.glasses_event.disconnect(_on_glasses_event)
		if tilt_five_xr_interface.is_initialized():
			tilt_five_xr_interface.uninitialize()
		
		XRServer.remove_interface(tilt_five_xr_interface)
		tilt_five_xr_interface = null
		id_to_state.clear()

func _ready():
	if not t5_manager:
		push_error("T5Manager is not set in T5Interface")
		return 
	if !tilt_five_xr_interface.is_initialized():
		tilt_five_xr_interface.initialize()

func _start_display(glasses_id : StringName, xr_rig : T5XRRig):
	tilt_five_xr_interface.start_display(glasses_id, xr_rig, xr_rig.get_origin())
	xr_rig.get_camera().tracker = "/user/%s/head" % glasses_id
	xr_rig.get_wand().tracker = "/user/%s/wand_1" % glasses_id

func _process_glasses():
	for glasses_id in id_to_state:
		var xr_rig_state = id_to_state.get(glasses_id) as XRRigState
		if xr_rig_state.can_attempt_to_reserve() and t5_manager.should_use_glasses(glasses_id):
			xr_rig_state.attempting_to_reserve = true
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
	var xr_rig_state = id_to_state.get(glasses_id) as XRRigState
	if not xr_rig_state:
		xr_rig_state = XRRigState.new()
		id_to_state[glasses_id] = xr_rig_state
	match event_num:
		TiltFiveXRInterface.E_GLASSES_AVAILABLE:
			xr_rig_state.available = true
			_process_glasses()

		TiltFiveXRInterface.E_GLASSES_UNAVAILABLE:
			xr_rig_state.available = false
			if xr_rig_state.attempting_to_reserve:
				xr_rig_state.attempting_to_reserve = false
				_process_glasses()

		TiltFiveXRInterface.E_GLASSES_RESERVED:
			xr_rig_state.reserved = true
			xr_rig_state.attempting_to_reserve = false
			
			# instance our scene
			var xr_rig = t5_manager.create_xr_rig(glasses_id)
			if xr_rig:
				xr_rig_state.xr_rig = xr_rig
				_start_display(glasses_id, xr_rig)
			else:
				tilt_five_xr_interface.release_glasses(glasses_id)
				
		TiltFiveXRInterface.E_GLASSES_DROPPED:
			xr_rig_state.reserved = false

			var xr_rig = xr_rig_state.xr_rig
			if xr_rig:
				tilt_five_xr_interface.stop_display(glasses_id)
				xr_rig_state.xr_rig = null
				t5_manager.release_xr_rig(xr_rig)

		TiltFiveXRInterface.E_GLASSES_TRACKING:
			var gbt = tilt_five_xr_interface.get_gameboard_type(glasses_id)
			var xr_rig = xr_rig_state.xr_rig
			if xr_rig and xr_rig._gameboard_type != gbt:
				xr_rig._gameboard_type = gbt
				xr_rig._gameboard_size = tilt_five_xr_interface.get_gameboard_extents(gbt)
				t5_manager.set_gameboard_type(xr_rig, gbt)
			
		_:
			pass # These are the only events that need to be handled
