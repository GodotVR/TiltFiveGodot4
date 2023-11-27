@icon("res://addons/tiltfive/assets/glasses.svg")
class_name T5Manager extends "res://addons/tiltfive/T5ManagerBase.gd" 
## Create a instance of an XR rig for each pair of Tilt Five glasses
##
## This node will create an instance of the T5XRRig or derived scene
## for each pair of Tilt Five glasses that are found. The glasses
## scene is an XR rig with an SubViewport, T5Origin, Camera, and wand.
## If the glasses scene is not specified then T5XRRig.tscn is used.
## 
## This should be persistent.

## Signal when the glasses scene is added to the main scene
## This signal is depreciated please use xr_rig_was_added
signal glasses_scene_was_added(glasses : T5XRRig)

## Signal when the glasses scene is removed from the main scene
## This signal is depreciated please use xr_rig_will_be_removed
signal glasses_scene_will_be_removed(glasses : T5XRRig)

## Signal when the T5XRRig scene is added to the main scene
signal xr_rig_was_added(xr_rig : T5XRRig)

## Signal when the T5XRRig scene is removed to the main scene
signal xr_rig_will_be_removed(xr_rig : T5XRRig)

## [PackedScene] inherited from T5XRRig.tcsn. 
@export var glasses_scene : PackedScene = preload("res://addons/tiltfive/scenes/T5XRRig.tscn")

## A [T5Gameboard] node in the scene that will be used to set the location and content scale of the
## [T5Origin] in the glasses scene
@export var start_location : T5Gameboard

# We'll add our glasses scenes as children of this node
var glasses_node: Node3D

func _ready():	
	glasses_node = Node3D.new()
	glasses_node.name = "TiltFiveGlasses"
	get_parent().add_child.call_deferred(glasses_node)

func create_xr_rig(glasses_id : String) -> T5XRRig:
	var xr_rig = glasses_scene.instantiate() as T5XRRig
	xr_rig._glasses_id = glasses_id
	glasses_node.add_child(xr_rig)
	if start_location:
		var origin := xr_rig.get_origin()
		origin.transform = start_location.transform
		origin.gameboard_scale = start_location.content_scale
	glasses_scene_was_added.emit(xr_rig)
	xr_rig_was_added.emit(xr_rig)
	return xr_rig
	
func release_xr_rig(xr_rig : T5XRRig) -> void:
	glasses_scene_will_be_removed.emit(xr_rig)
	xr_rig_will_be_removed.emit(xr_rig)
	glasses_node.remove_child(xr_rig)
	xr_rig.queue_free()
	
