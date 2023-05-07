extends Node3D


var glasses_rig = preload("res://T5Glasses.tscn")


func _on_t5_manager_glasses_available(glasses_id):
	$T5Manager.reserve_glasses(glasses_id)

func _on_t5_manager_glasses_reserved(glasses_id):
	var gview = glasses_rig.instantiate()
	add_child(gview)
	$T5Manager.start_display(glasses_id, gview)

