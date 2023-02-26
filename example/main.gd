extends Node


func _ready():
	$TiltFiveManager.add(10)
	$TiltFiveManager.add(20)
	$TiltFiveManager.add(30)
	print($TiltFiveManager.get_total())
	
