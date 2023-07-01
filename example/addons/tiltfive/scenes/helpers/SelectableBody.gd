extends Node3D

# Add this script onto a physics body to make it interact with a T5Pointer
# You also can inherit it and further implement functions

@export_range(0.0, 1.0, 0.0001) var grow_amount : float = 0.0002 :
	set(value):
		grow_amount = value
		if highlight_material:
			highlight_material.grow_amount = grow_amount

var highlight_material : StandardMaterial3D
var pointers_entered : Array

func _ready():
	var material : StandardMaterial3D = preload("res://addons/tiltfive/materials/highlight_overlay_material.tres")
	highlight_material = material.duplicate(true)
	highlight_material.grow_amount = grow_amount

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

