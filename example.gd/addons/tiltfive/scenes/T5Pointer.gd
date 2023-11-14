extends RayCast3D
class_name T5Pointer

var pointing_at : PhysicsBody3D

func get_pointing_at() -> PhysicsBody3D:
	return pointing_at

func _physics_process(_delta):
	if is_colliding():
		var is_pointing_at : PhysicsBody3D = get_collider()

		if pointing_at != is_pointing_at:
			# out with the old
			if pointing_at and pointing_at.has_method("on_pointer_exited"):
				pointing_at.on_pointer_exited(self)

			# in with the new
			pointing_at = is_pointing_at
			if pointing_at and pointing_at.has_method("on_pointer_entered"):
				pointing_at.on_pointer_entered(self)
	elif pointing_at:
		if pointing_at.has_method("on_pointer_exited"):
			pointing_at.on_pointer_exited(self)

		# unset
		pointing_at = null
