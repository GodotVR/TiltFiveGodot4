extends T5Controller3D

@export
var selected_mat : Material

@export
var unselected_mat : Material

var stick_pos = Vector3()
var trigger_pos = Vector3()

var button_dict = {
	T5Def.WAND_BUTTON_A : ^"Controls/A",
	T5Def.WAND_BUTTON_B : ^"Controls/B",
	T5Def.WAND_BUTTON_X : ^"Controls/X",
	T5Def.WAND_BUTTON_Y : ^"Controls/Y",
	T5Def.WAND_BUTTON_1 : ^"Controls/One",
	T5Def.WAND_BUTTON_2 : ^"Controls/Two",
	T5Def.WAND_BUTTON_STICK : ^"Controls/Three",
	T5Def.WAND_BUTTON_T5 : ^"Controls/T5",
	T5Def.WAND_BUTTON_TRIGGER : ^"Controls/TriggerClick"
}

# Called when the node enters the scene tree for the first time.
func _ready():
	stick_pos = $Controls/Three.transform.origin
	trigger_pos = $Controls/Trigger.transform.origin
	$Controls/Trigger.material_override = unselected_mat
	for key in button_dict.keys():
		var ctrl = get_node_or_null(button_dict[key])
		button_dict[key] = ctrl
		if ctrl:
			ctrl.material_override = unselected_mat

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	var trigger = get_float(T5Def.WAND_ANALOG_TRIGGER) * 0.03
	$Controls/Trigger.transform.origin = trigger_pos + Vector3(0, 0, trigger)
	var axis = get_vector2(T5Def.WAND_ANALOG_STICK) * 0.03
	$Controls/Three.transform.origin = stick_pos + Vector3(axis.x, 0, -axis.y)

func _on_button_pressed(button):
	var ctrl = button_dict.get(button)
	if ctrl:
		ctrl.material_override = selected_mat
	if button == T5Def.WAND_BUTTON_1:
		trigger_haptic_pulse(1, 100)
	elif button == T5Def.WAND_BUTTON_2:
		trigger_haptic_pulse(1, 50)

func _on_button_released(button):
	var ctrl = button_dict.get(button)
	if ctrl:
		ctrl.material_override = unselected_mat

