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
@export var glasses_scene : PackedScene = preload("res://addons/tiltfive/scenes/T5GlassesBase.tscn")

var reserved_glasses: Dictionary

# We'll add our glasses scenes as children of this node
var glasses_node: Node3D

# Called when the manager is loaded and added to our scene
func _enter_tree():
	tilt_five_xr_interface = T5Interface.get_tilt_five_xr_interface()

	if tilt_five_xr_interface:
		tilt_five_xr_interface.service_event.connect(on_service_event)
		tilt_five_xr_interface.glasses_event.connect(on_glasses_event)

# Called when the manager is removed
func _exit_tree():
	if tilt_five_xr_interface:
		tilt_five_xr_interface.glasses_event.disconnect(on_glasses_event)
		tilt_five_xr_interface.service_event.disconnect(on_service_event)

		tilt_five_xr_interface = null

# Called when our scene is fully setup
func _ready():
	# Create a node on our parent under which we create our glasses scenes
	glasses_node = Node3D.new()
	glasses_node.name = "TiltFiveGlasses"
	get_parent().add_child.call_deferred(glasses_node)

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
		print_verbose("Warning: Tilt Five glasses id ", glasses_id, " does not exist")
		return
	if reserved_glasses[glasses_id]:
		print_verbose("Warning: Tilt Five glasses ", glasses_id, " already reserved")
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

func node_name_from_glasses_id(glasses_id: String) -> String:
	return "Glasses_" + glasses_id.replace("-", "_")

func on_service_event(event_num):
	match  event_num:
		TiltFiveXRInterface.E_SERVICE_T5_UNAVAILABLE:
			print("Tilt Five service is unavailable")
		TiltFiveXRInterface.E_SERVICE_T5_INCOMPATIBLE_VERSION:
			print("Tilt Five service has an incompatible version")
		TiltFiveXRInterface.E_SERVICE_RUNNING:
			print("Tilt Five service is running")
		TiltFiveXRInterface.E_SERVICE_STOPPED:
			print("Tilt Five service has stopped")

func on_glasses_event(glasses_id, event_num):
	match event_num:
		TiltFiveXRInterface.E_GLASSES_AVAILABLE:
			print_verbose(glasses_id, " E_GLASSES_AVAILABLE")
			if not reserved_glasses.has(glasses_id):
				reserved_glasses[glasses_id] = false

			# If we're managing our glasses scene, reserve our glasses
			if glasses_scene:
				reserve_glasses(glasses_id)

			# Let others who are interested know
			glasses_available.emit(glasses_id)

		TiltFiveXRInterface.E_GLASSES_UNAVAILABLE:
			print_verbose(glasses_id, " E_GLASSES_UNAVAILABLE")

			# Let others who are interested know
			reserved_glasses.erase(glasses_id)

		TiltFiveXRInterface.E_GLASSES_RESERVED:
			print_verbose(glasses_id, " E_GLASSES_RESERVED")

			reserved_glasses[glasses_id] = true

			# If we're managing our glasses scene, instance our scene
			if glasses_scene:
				var gview = glasses_scene.instantiate()
				gview.name = node_name_from_glasses_id(glasses_id)
				glasses_node.add_child(gview)
				start_display(glasses_id, gview)

			# Let others who are interested know
			glasses_reserved.emit(glasses_id)

		TiltFiveXRInterface.E_GLASSES_DROPPED:
			print_verbose(glasses_id, " E_GLASSES_DROPPED")

			var node_name = node_name_from_glasses_id(glasses_id)
			var gview = glasses_node.get_node_or_null(node_name)
			if gview:
				tilt_five_xr_interface.stop_display(glasses_id)
				glasses_node.remove_child(gview)
				gview.queue_free()

			if reserved_glasses.get(glasses_id, false):
				reserved_glasses[glasses_id] = false
				glasses_dropped.emit(glasses_id)

		TiltFiveXRInterface.E_GLASSES_TRACKING:
			var gbt = tilt_five_xr_interface.get_gameboard_type(glasses_id)
			print_verbose(glasses_id, " E_GLASSES_TRACKING, Gameboard size = ", tilt_five_xr_interface.get_gameboard_extents(gbt))

		TiltFiveXRInterface.E_GLASSES_NOT_TRACKING:
			print_verbose(glasses_id, " E_GLASSES_NOT_TRACKING")

		_:
			print_verbose(glasses_id, " - unknown event: ", event_num)

