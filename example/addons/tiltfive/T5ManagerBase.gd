extends Node 
## Base class for all T5Managers. Should not be used directly.
##
## Classes derived from T5ManagerBase implement these functions
## to customize the process of connecting the XR rigs in the scene
## to the Tilt Five glasses hardware that is found.
##
## These functions must be overridden 
##
## create_glasses_scene
## release_glasses_scene
## get_glasses_scene_viewport
## get_glasses_scene_origin
## get_glasses_scene_camera
## get_glasses_scene_wand
##
## The derived node should be persistent.

# Called when the manager is loaded and added to our scene
func _enter_tree():
	T5Interface.t5_manager = self

# Called when the manager is removed
func _exit_tree():
	T5Interface.t5_manager = null

## Invoked by the T5Interface to find out if the glasses should be used in
## game
func should_use_glasses(glasses_id : String) -> bool:
	return true
	
## Invoked by the T5Interface to get the display name to be assigned to 
## the glasses. This is the name that shows up in the Tilt Five control
## panel
func get_glasses_display_name(glasses_id : String) -> String:
	return T5ProjectSettings.default_display_name

## Invoked by the T5Interface to get the XR rig scene to be associated with 
## tilt five glasses. This scene should contain a SubViewport -> T5Origin -> Camera3D and T5Controller3D(s)
func create_glasses_scene(glasses_id : String) -> Node:
	push_error("create_glasses_scene not implemented in T5ManagerBase derived class")
	return null

## Invoked by the T5Interface if the Tilt Five glasses become unavailable
func release_glasses_scene(glasses_scene : Node) -> void:
	push_error("release_glasses_scene not implemented in T5ManagerBase derived class")
	
## Invoked by the T5Interface to get the SubViewport of the XR rig
func get_glasses_scene_viewport(glasses_scene : Node) -> SubViewport:
	push_error("get_glasses_scene_viewport not implemented in T5ManagerBase derived class")
	return null

## Invoked by the T5Interface to get the T5Origin3D of the XR rig
func get_glasses_scene_origin(glasses_scene : Node) -> T5Origin3D:
	push_error("get_glasses_scene_origin not implemented in T5ManagerBase derived class")
	return null

## Invoked by the T5Interface to get the Camera3D of the XR rig
func get_glasses_scene_camera(glasses_scene : Node) -> Camera3D:
	push_error("get_glasses_scene_camera not implemented in T5ManagerBase derived class")
	return null
	
## Invoked by the T5Interface to get a T5Controller3D from the XR rig. Although the default rig
## has only one wand two may be paired to a headset.
func get_glasses_scene_wand(glasses_scene : Node, wand_num : int) -> T5Controller3D:
	push_error("get_glasses_scene_wand not implemented in T5ManagerBase derived class")
	return null
	
## Invoked by the T5Interface to set the gameboard type the Tilt Fiave glasses detected
func set_glasses_scene_gameboard_type(glasses_scene : Node, gameboard_type : T5Interface.GameboardType) -> void:
	pass
	

