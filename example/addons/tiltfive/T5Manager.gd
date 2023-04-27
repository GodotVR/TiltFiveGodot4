class_name T5Manager extends Node 

signal glasses_available
signal glasses_reserved(success)
signal glasses_dropped

enum GlassesEvent {
	E_ADDED         = 1,
	E_LOST          = 2,
	E_AVAILABLE     = 3,
	E_UNAVAILABLE   = 4,
	E_RESERVED      = 5,
	E_DROPPED  		= 6,
	E_TRACKING      = 7,
	E_NOT_TRACKING  = 8,
	E_STOPPED_ON_ERROR = 9
}

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

func reserve_glasses(viewport : Viewport, display_name := "") -> void:
	if has_reserved_glasses():
		print("Warning: Tilt Five glasses already reserved")
		return
	if display_name.length() == 0:
		display_name = default_display_name
	for try_glasses_id in reserved_glasses:
		tilt_five_xr_interface.reserve_glasses(try_glasses_id, display_name, viewport.get_viewport_rid())
		while true:
			var result = await tilt_five_xr_interface.glasses_event
			if result[0] != try_glasses_id: 
				continue
			elif result[1] == GlassesEvent.E_RESERVED:
				reserved_glasses[try_glasses_id] = true
				glasses_reserved.emit(true)
				return
			else: 
				break
	glasses_reserved.emit(false)
			
	
func on_glasses_event(glasses_id, event_num):
	print(glasses_id, " ", event_num)
	match  event_num:
		GlassesEvent.E_AVAILABLE:
			if not reserved_glasses.has(glasses_id):
				reserved_glasses[glasses_id] = false
			glasses_available.emit()
		GlassesEvent.E_UNAVAILABLE:
			reserved_glasses.erase(glasses_id)
		GlassesEvent.E_DROPPED:
			if reserved_glasses.get(glasses_id, false):
				reserved_glasses[glasses_id] = false
				glasses_dropped.emit()
	
		
