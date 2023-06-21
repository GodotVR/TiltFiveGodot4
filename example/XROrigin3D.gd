extends T5Origin3D

var elapsed = 0.0

func _init():
	print(gameboard_scale)

func _process(delta):
	elapsed += delta
	position.x = sin(elapsed) * 1
	
