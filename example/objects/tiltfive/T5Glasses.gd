extends "res://addons/tiltfive/scenes/T5GlassesBase.gd"

# Some of these may end up being renamed
@onready var gameboard = $Origin
@onready var pointer = $"$Origin/Wand_1/T5-pointer"
@onready var attactor = $Origin/Wand_1/Attractor

var trigger_pressed : bool = false

func _on_wand_button_pressed(name):
	pass # Replace with function body.


func _on_wand_button_released(name):
	pass # Replace with function body.


func _on_wand_input_float_changed(name, value):
	if name == "trigger":
		# Handle trigger as a button
		if trigger_pressed and value < 0.4:
			attactor.enabled = false
			trigger_pressed = false
		elif !trigger_pressed and value > 0.6:
			var pointing_at = pointer.get_pointing_at()
			if pointing_at and pointing_at is RigidBody3D:
				attactor.attract = pointing_at
				attactor.enabled = true
			
			trigger_pressed = true

func _physics_process(delta):
	if wand and wand.visible:
		var stick = wand.get_vector2("stick")
		var orientation = wand.global_transform.basis
		var forward = Vector3(orientation.z.x, 0.0, orientation.z.z).normalized()
		var right = Vector3(orientation.x.x, 0.0, orientation.x.z).normalized()
		
		gameboard.global_transform.origin += (-stick.y * forward * delta) + (stick.x * right * delta)
