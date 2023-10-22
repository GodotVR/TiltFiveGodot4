extends T5Origin3D

var elapsed = 0.0

func _process(delta):
	elapsed += delta
	position.y = sin(elapsed) * 1
	
