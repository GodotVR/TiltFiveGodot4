extends T5Controller3D

@export
var selected_mat : Material

@export
var unselected_mat : Material

var stick_pos = Vector3()
var trigger_pos = Vector3()

const WAND_BUTTON_A		:= "button_a"
const WAND_BUTTON_B		:= "button_b"
const WAND_BUTTON_X		:= "button_x"
const WAND_BUTTON_Y		:= "button_y"
const WAND_BUTTON_1		:= "button_1"
const WAND_BUTTON_2		:= "button_2"
const WAND_BUTTON_STICK	:= "button_3"
const WAND_BUTTON_T5	:= "button_t5"
	# Axis
const WAND_ANALOG_STICK	:= "stick"
const WAND_ANALOG_TRIGGER := "trigger"

# Called when the node enters the scene tree for the first time.
func _ready():
	stick_pos = $Controls/Three.transform.origin
	trigger_pos = $Controls/Trigger.transform.origin
	$Controls/A.material_override = unselected_mat
	$Controls/B.material_override = unselected_mat
	$Controls/X.material_override = unselected_mat
	$Controls/Y.material_override = unselected_mat
	$Controls/One.material_override = unselected_mat
	$Controls/Two.material_override = unselected_mat
	$Controls/Three.material_override = unselected_mat
	$Controls/T5.material_override = unselected_mat
	$Controls/Trigger.material_override = unselected_mat


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	var trigger = get_float(WAND_ANALOG_TRIGGER) * 0.03
	$Controls/Trigger.transform.origin = trigger_pos + Vector3(0, 0, trigger)
	var axis = get_vector2(WAND_ANALOG_STICK) * 0.03
	$Controls/Three.transform.origin = stick_pos + Vector3(axis.x, 0, -axis.y)


func _on_button_pressed(button):
	match button:
		WAND_BUTTON_A:
			$Controls/A.material_override = selected_mat
		WAND_BUTTON_B:
			$Controls/B.material_override = selected_mat
		WAND_BUTTON_X:
			$Controls/X.material_override = selected_mat
		WAND_BUTTON_Y:
			$Controls/Y.material_override = selected_mat
		WAND_BUTTON_1:
			$Controls/One.material_override = selected_mat
		WAND_BUTTON_2:
			$Controls/Two.material_override = selected_mat
		WAND_BUTTON_STICK:
			$Controls/Three.material_override = selected_mat
		WAND_BUTTON_T5:
			$Controls/T5.material_override = selected_mat


func _on_button_released(button):
	match button:
		WAND_BUTTON_A:
			$Controls/A.material_override = unselected_mat
		WAND_BUTTON_B:
			$Controls/B.material_override = unselected_mat
		WAND_BUTTON_X:
			$Controls/X.material_override = unselected_mat
		WAND_BUTTON_Y:
			$Controls/Y.material_override = unselected_mat
		WAND_BUTTON_1:
			$Controls/One.material_override = unselected_mat
		WAND_BUTTON_2:
			$Controls/Two.material_override = unselected_mat
		WAND_BUTTON_STICK:
			$Controls/Three.material_override = unselected_mat
		WAND_BUTTON_T5:
			$Controls/T5.material_override = unselected_mat
