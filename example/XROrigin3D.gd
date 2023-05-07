extends XROrigin3D

var elapsed = 0.0

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	elapsed += delta
	position.x = sin(elapsed) * 1
	
