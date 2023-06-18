extends Node3D

# Add this script onto a physics body to make it interact with a T5Pointer
# You also can inherit it and further implement functions

@onready var highlight_material : StandardMaterial3D = preload("res://addons/tiltfive/materials/highlight_overlay_material.tres")

var pointers_entered : Array

func on_pointer_entered(pointer):
	if !pointers_entered.has(pointer):
		pointers_entered.push_back(pointer)

	_update_highlighted()

func on_pointer_exited(pointer):
	if pointers_entered.has(pointer):
		pointers_entered.erase(pointer)

	_update_highlighted()

func _update_highlighted():
	var overlay_material : StandardMaterial3D
	if pointers_entered.size() > 0:
		overlay_material = highlight_material

	for child in get_children():
		if child is MeshInstance3D:
			var mi : MeshInstance3D = child
			mi.material_overlay = overlay_material

