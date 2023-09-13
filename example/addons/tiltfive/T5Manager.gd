class_name T5Manager extends T5ManagerBase 

## The T5Manager node should be added to your
## main scene and will manage when glasses
## and wands are detected.
##
## This should be persistent.

signal glasses_scene_was_added(glasses : T5GlassesBase)
signal glasses_scene_will_be_removed(glasses : T5GlassesBase)

const xr_origin_node := ^"Origin"
const xr_camera_node := ^"Origin/Camera"
const wand_node_list := [^"Origin/Wand_1", ^"Origin/Wand_2"]

@export var glasses_scene : PackedScene = preload("res://addons/tiltfive/scenes/T5GlassesBase.tscn")

# We'll add our glasses scenes as children of this node
var glasses_node: Node3D

func _ready():	
	glasses_node = Node3D.new()
	glasses_node.name = "TiltFiveGlasses"
	get_parent().add_child.call_deferred(glasses_node)

func create_glasses_scene(glasses_id : String) -> Node:
	var gview = glasses_scene.instantiate()
	glasses_node.add_child(gview)
	glasses_scene_was_added.emit(gview)
	return gview
	
func release_glasses_scene(glasses_scene : Node) -> void:
	glasses_scene_will_be_removed.emit(glasses_scene)
	glasses_node.remove_child(glasses_scene)
	glasses_scene.queue_free()

func get_glasses_scene_viewport(glasses_scene : Node) -> SubViewport:
	return glasses_scene as SubViewport

func get_glasses_scene_origin(glasses_scene : Node) -> T5Origin3D:
	return glasses_scene.get_node(xr_origin_node)

func get_glasses_scene_camera(glasses_scene : Node) -> Camera3D:
	return glasses_scene.get_node(xr_camera_node)
	
func get_glasses_scene_wand(glasses_scene : Node, wand_num : int) -> T5Controller3D:
	if wand_num < wand_node_list.size():
		return glasses_scene.get_node_or_null(wand_node_list[wand_num]) as T5Controller3D
	return null
	
