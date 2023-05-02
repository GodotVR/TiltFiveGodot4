extends Node3D



func _on_node_glasses_available():
	$T5Manager.reserve_glasses($T5GlassesViewport)


func _on_node_glasses_reserved(success):
	if success:
		print("Got glasses")
		$T5GlassesViewport.use_xr = true
