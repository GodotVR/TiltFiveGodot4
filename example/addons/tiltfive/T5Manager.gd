class_name T5Manager extends Node 

signal glasses_available(glasses_id : String)
signal glasses_reserved(glasses_id : String)
signal glasses_dropped(glasses_id : String)

const xr_origin_node := ^"Gameboard"

const wand_node_list := [^"Gameboard/Wand_1", ^"Gameboard/Wand_2", ^"Gameboard/Wand_3", ^"Gameboard/Wand_4"]

var tilt_five_xr_interface: TiltFiveXRInterface 

@export var automatically_start : bool = true
@export var application_id : String = "my.game.com"
@export var application_version : String = "0.1.0"
@export var default_display_name : String = "Game: Player One"

var reserved_glasses: Dictionary

# Called when the node enters the scene tree for the first time.
func _ready():
	tilt_five_xr_interface = TiltFiveXRInterface.new();
	if not tilt_five_xr_interface:
		return
	tilt_five_xr_interface.glasses_event.connect(on_glasses_event)
	if automatically_start:
		start_service()
		
func _exit_tree():
	pass
	
func start_service() -> bool:
	return tilt_five_xr_interface.start_service(application_id, application_version)

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
		display_name = default_display_name
	tilt_five_xr_interface.reserve_glasses(glasses_id, display_name)
	
func start_display(glasses_id : StringName, viewport : SubViewport):
	var xr_origin = viewport.get_node(xr_origin_node)
	tilt_five_xr_interface.start_display(glasses_id, viewport, xr_origin)
	for wand_node in wand_node_list:
		var controller = viewport.get_node(wand_node) as XRController3D
		if controller:
			var tracker_name = controller.tracker
			controller.tracker = tracker_name.replace("glasses", glasses_id)
		
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
		GlassesEvent.E_TRACKING:
			var gbt = tilt_five_xr_interface.get_gameboard_type(glasses_id)
			print("Gameboard size = ", tilt_five_xr_interface.get_gameboard_extents(gbt))
		
