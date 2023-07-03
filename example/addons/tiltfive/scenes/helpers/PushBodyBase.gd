extends Node3D

@export var push_strength : float = 0.01

var was_parent_visible : bool = false

func _physics_process(delta):
	var is_parent_visible : bool = get_parent().visible
	if !was_parent_visible and is_parent_visible:
		# move it to our starting position
		$Body.global_transform = global_transform
	elif is_parent_visible:
		# Always rotate
		$Body.global_transform.basis = global_transform.basis

		# Then we do our move and collide
		var motion = global_position - $Body.global_position
		var collision = $Body.move_and_collide(motion, false, 0.001, false, 1)
		if collision:
			var with : PhysicsBody3D = collision.get_collider()

			if with and with is RigidBody3D:
				var rb : RigidBody3D = with

				var pos = collision.get_position() - rb.global_position
				var impulse = -collision.get_normal() * (push_strength * motion.length() / delta)

				rb.apply_impulse(impulse, pos)

	was_parent_visible = is_parent_visible
