class_name T5XRRig extends SubViewport
## represents a scene with all the components needed for Tilt Five tracked glasses and wand

var _glasses_id : StringName
var _gameboard_type := T5Def.GameboardType.Unknown
var _gameboard_size := AABB()
var _origin : T5Origin3D
var _camera : T5Camera3D
var _wand : T5Controller3D

## Get the ID attached to a pair of Tilt Five glasses
func get_glasses_id() -> StringName:
	return _glasses_id

## Get the friendly name of the glasses defined in the Tilt Five control panel
func get_glasses_name() -> String:
	return T5Interface.get_glasses_name(_glasses_id)

## Type of gameboard that is set up 
func get_gameboard_type() -> T5Def.GameboardType:
	return _gameboard_type

## size of the gameboard in meters. Raised XE gameboards can have a height
func get_gameboard_size() -> AABB:
	return _gameboard_size

## the node that relates the center of the gameboard to world coordinates
func get_origin() -> T5Origin3D:
	return _origin

## the tracked camera
func get_camera() -> T5Camera3D:
	return _camera

## the tracked wand controller
func get_wand() -> T5Controller3D:
	return _wand

func _enter_tree():
	_origin = $Origin
	_camera = $Origin/Camera
	_wand = $Origin/Wand_1

func _process(_delta):
	if _wand: _wand.visible = _wand.get_has_tracking_data()
