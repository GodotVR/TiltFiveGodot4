extends Node3D

func _on_t5_manager_glasses_scene_was_added(glasses):
	print("Scene ", glasses.name, " added")

func _on_t5_manager_glasses_scene_will_be_removed(glasses):
	print("Scene ", glasses.name, " removed")
