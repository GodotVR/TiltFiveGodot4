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
##
## The derived node should be persistent.

# Called when the manager is loaded and added to our scene
func _enter_tree():
	T5Interface.t5_manager = self

# Called when the manager is removed
func _exit_tree():
	T5Interface.t5_manager = null

## Invoked by T5Interface when the Tilt Five service has started
func service_started():
	pass
	
## Invoked by T5Interface when the Tilt Five service has stopped
func service_stopped():
	pass
	
## Invoked by T5Interface when the Tilt Five service is not available
## The driver might not be installed
func service_unvailable():
	pass
	
## Invoked by T5Interface when the Tilt Five service installed does
## not meet the minimum version requirements
func service_incorrect_version():
	pass

## Invoked by the T5Interface to find out if the glasses should be used in
## game
func should_use_glasses(glasses_id : String) -> bool:
	return true

## Invoked by the T5Interface to get an T5XRRig derived node
func create_xr_rig(glasses_id : String) -> T5XRRig:
	push_error("create_xr_rig not implemented in T5ManagerBase derived class")
	return null

## Invoked by the T5Interface if the Tilt Five glasses become unavailable
func release_xr_rig(xr_rig : T5XRRig) -> void:
	push_error("release_glasses_scene not implemented in T5ManagerBase derived class")
	
## Invoked by the T5Interface to set the gameboard type the Tilt Fiave glasses detected
func set_gameboard_type(xr_rig : T5XRRig, gameboard_type : T5Def.GameboardType) -> void:
	pass
	

