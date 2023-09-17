@tool
class_name T5Start extends Node3D

@onready var boards = [ $T5_border, $T5_border_XE, $T5_border_XE_raised]

@export var content_scale : float = 1.0:
	set(value):
		content_scale = value
		if boards:
			for node in boards:
				node.scale = Vector3(value, value, value)
		
@export_enum("LE Gameboard", "XE Gameboard", "Raised XE Gameboard") var gameboard_type: int = 0:
	set(value):
		gameboard_type = value
		if boards:
			for idx in boards.size():
				boards[idx].visible = (idx == value)

@export var show_at_runtime : bool = false
		
func _ready():
	var show_board :=  Engine.is_editor_hint() or show_at_runtime
	for idx in boards.size():
		boards[idx].scale = Vector3(content_scale, content_scale, content_scale)
		boards[idx].visible = (idx == gameboard_type) and show_board
		
