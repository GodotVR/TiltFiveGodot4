class_name T5Manager extends Node 

## The T5Manager node should be added to your
## main scene and will manage when glasses
## and wands are detected.
##
## This should be persistent.

signal glasses_available(glasses_id : String)
signal glasses_reserved(glasses_id : String)
signal glasses_dropped(glasses_id : String)

const xr_origin_node := ^"Origin"
const xr_camera_node := ^"Origin/Camera"
const wand_node_list := [^"Origin/Wand_1", ^"Origin/Wand_2", ^"Origin/Wand_3", ^"Origin/Wand_4"]

var tilt_five_xr_interface: TiltFiveXRInterface 

@export var automatically_start : bool = true

var reserved_glasses: Dictionary

# Called when the manager is loaded and added to our scene
func _enter_tree():
	tilt_five_xr_interface = T5Interface.get_tile_five_xr_interface()

	if tilt_five_xr_interface:
		tilt_five_xr_interface.glasses_event.connect(on_glasses_event)

# Called when the manager is removed
func _exit_tree():
	if tilt_five_xr_interface:
		tilt_five_xr_interface.glasses_event.disconnect(on_glasses_event)

		tilt_five_xr_interface = null

# Called when our scene is fully setup
func _ready():
	if not tilt_five_xr_interface:
		return

	if automatically_start and !tilt_five_xr_interface.is_initialized():
		tilt_five_xr_interface.initialize()

func start_service() -> bool:
	if not tilt_five_xr_interface:
		return false

	return tilt_five_xr_interface.initialize()

func has_reserved_glasses() -> bool:
	for glasses in reserved_glasses:
		if reserved_glasses[glasses]:
			return true
	return false

func reserve_glasses(glasses_id : StringName, display_name := "") -> void:
	if not reserved_glasses.has(glasses_id):
		print("Warning: Tilt Five glasses id ", glasses_id, " does not exist")
		return
	if reserved_glasses[glasses_id]:
		print("Warning: Tilt Five glasses ", glasses_id, " already reserved")
		return
	if display_name.length() == 0:
		display_name = ProjectSettings.get_setting_with_override("xr/tilt_five/default_display_name")
	tilt_five_xr_interface.reserve_glasses(glasses_id, display_name)
	
func start_display(glasses_id : StringName, viewport : SubViewport):
	var xr_origin = viewport.get_node(xr_origin_node)
	tilt_five_xr_interface.start_display(glasses_id, viewport, xr_origin)
	var t5_camera := viewport.get_node_or_null(xr_camera_node) as T5Camera3D
	if t5_camera:
		t5_camera.tracker = "/user/%s/head" % glasses_id
	for idx in 4:
		var controller = viewport.get_node_or_null(wand_node_list[idx]) as T5Controller3D
		if controller:
			controller.tracker = "/user/%s/wand_%d" % [glasses_id, idx + 1]
		
func on_glasses_event(glasses_id, event_num):
	print(glasses_id, " ", event_num)
	match  event_num:
		TiltFiveXRInterface.E_AVAILABLE:
			if not reserved_glasses.has(glasses_id):
				reserved_glasses[glasses_id] = false
			glasses_available.emit(glasses_id)
		TiltFiveXRInterface.E_UNAVAILABLE:
			reserved_glasses.erase(glasses_id)
		TiltFiveXRInterface.E_RESERVED:
			reserved_glasses[glasses_id] = true
			glasses_reserved.emit(glasses_id)
		TiltFiveXRInterface.E_DROPPED:
			if reserved_glasses.get(glasses_id, false):
				reserved_glasses[glasses_id] = false
				glasses_dropped.emit(glasses_id)
		TiltFiveXRInterface.E_TRACKING:
			var gbt = tilt_five_xr_interface.get_gameboard_type(glasses_id)
			print("Gameboard size = ", tilt_five_xr_interface.get_gameboard_extents(gbt))
		
