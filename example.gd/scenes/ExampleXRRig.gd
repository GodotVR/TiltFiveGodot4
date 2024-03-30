extends T5XRRig


func _ready():
	var timer = Timer.new()
	timer.autostart = true
	timer.wait_time = 1
	add_child(timer)
	timer.timeout.connect(func():
		$Origin/Pivot/GlassesName.text = get_glasses_name()
	)

func _process(delta):
	var pos : Vector3 = $Origin/Camera.global_position
	if pos.distance_to($Origin/Pivot.global_position) < 0.001: return
	$Origin/Pivot.look_at(Vector3(pos.x, $Origin/Pivot.global_position.y, pos.z), Vector3.UP, true)
