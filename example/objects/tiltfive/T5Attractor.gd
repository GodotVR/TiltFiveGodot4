extends Node3D

@export var enabled : bool = false
@export var attract : RigidBody3D

var strength : float = 5.0
var gravity : float = 0.98

func _physics_process(delta):
	if enabled and attract:
		var direction = ((global_position - attract.global_position) * strength) - attract.linear_velocity
		if direction.length() > 0:
			var impulse = direction * delta
			impulse += Vector3.UP * delta * gravity
			attract.apply_impulse(impulse)
