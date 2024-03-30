extends Node3D


func _on_t5_manager_xr_rig_was_added(xr_rig):
	print("Scene for glasses ", xr_rig.get_glasses_id(), " added")


func _on_t5_manager_xr_rig_will_be_removed(xr_rig):
	print("Scene for glasses ", xr_rig.get_glasses_id(), " removed")
