@icon("res://addons/tiltfive/assets/glasses.svg")
class_name T5Manager extends "res://addons/tiltfive/T5ManagerBase.gd" 
## Create a instance of an XR rig for each pair of Tilt Five glasses
##
## This node will create an instance of the T5GlassesBase or derived scene
## for each pair of Tilt Five glasses that are found. The glasses
## scene is an XR rig with an SubViewport, T5Origin, Camera, and wand.
## If the glasses scene is not specified then T5GlassesBase.tscn is used.
## 
## This should be persistent.

## Signal when the glasses scene is added to the main scene
signal glasses_scene_was_added(glasses : T5GlassesBase)

## Signal when the glasses scene is removed from the main scene
signal glasses_scene_will_be_removed(glasses : T5GlassesBase)

const xr_origin_node := ^"Origin"
const xr_camera_node := ^"Origin/Camera"
const wand_node_list := [^"Origin/Wand_1", ^"Origin/Wand_2"]

## [PackedScene] that will instanced for a pair of Tilt Five glasses. Defaults to T5GlassesBase.tscn
@export var glasses_scene : PackedScene = preload("res://addons/tiltfive/scenes/T5GlassesBase.tscn")

## A [T5Gameboard] node in the scene that will be used to set the location and content scale of the
## [T5Origin] in the glasses scene
@export var start_location : T5Gameboard

# We'll add our glasses scenes as children of this node
var glasses_node: Node3D

func _ready():	
	glasses_node = Node3D.new()
	glasses_node.name = "TiltFiveGlasses"
	get_parent().add_child.call_deferred(glasses_node)

func create_glasses_scene(glasses_id : String) -> Node:
	var gview = glasses_scene.instantiate()
	glasses_node.add_child(gview)
	if start_location:
		var origin := get_glasses_scene_origin(gview)
		origin.transform = start_location.transform
		origin.gameboard_scale = start_location.content_scale
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
	
