class_name T5ManagerBase extends Node 

## The T5ManagerBase derived node should be added to your
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

func should_use_glasses(glasses_id : String) -> bool:
	return true
	
func get_glasses_display_name(glasses_id : String) -> String:
	return ProjectSettings.get_setting_with_override("xr/tilt_five/default_display_name")

func create_glasses_scene(glasses_id : String) -> Node:
	push_error("create_glasses_scene not implemented in T5ManagerBase derived class")
	return null
	
func release_glasses_scene(glasses_scene : Node) -> void:
	push_error("destroy_glasses_scene not implemented in T5ManagerBase derived class")
	
func get_glasses_scene_viewport(glasses_scene : Node) -> SubViewport:
	push_error("get_glasses_scene_viewport not implemented in T5ManagerBase derived class")
	return null

func get_glasses_scene_origin(glasses_scene : Node) -> T5Origin3D:
	push_error("get_glasses_scene_origin not implemented in T5ManagerBase derived class")
	return null

func get_glasses_scene_camera(glasses_scene : Node) -> Camera3D:
	push_error("get_glasses_scene_camera not implemented in T5ManagerBase derived class")
	return null
	
func get_glasses_scene_wand(glasses_scene : Node, wand_num : int) -> T5Controller3D:
	push_error("get_glasses_scene_wand not implemented in T5ManagerBase derived class")
	return null
	
func set_glasses_scene_gameboard_type(glasses_scene : Node, gameboard_type : T5Interface.GameboardType) -> void:
	pass
	

