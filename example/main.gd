extends Node3D



func _on_node_glasses_available():
	$T5Manager.reserve_glasses()


func _on_node_glasses_reserved(success):
	if success:
		print("Got glasses")
		get_viewport().use_xr = true
