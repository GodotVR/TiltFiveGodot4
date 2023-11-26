class_name T5XRRig extends SubViewport
## represents a scene with all the components needed for Tilt Five tracked glasses and wand

## An ID attached to a pair of Tilt Five glasses
var glasses_id : StringName

## Type of gameboard that is set up 
var gameboard_type := T5Def.GameboardType.Unknown

## size of the gameboard in meters. Raised XE gameboards can have a height
var gameboard_size := AABB()

## the node that relates the center of the gameboard to world coordinates
var origin : T5Origin3D

## the tracked camera
var camera : T5Camera3D

## the tracked wand controller
var wand : T5Controller3D

func _enter_tree():
	origin = $Origin
	camera = $Origin/Camera
	wand = $Origin/Wand_1

func _process(_delta):
	if wand: wand.visible = wand.get_has_tracking_data()
