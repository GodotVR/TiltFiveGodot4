extends SubViewport

var glasses_id : String

func attach_glasses(glasses_id : String, display_name : String):
	self.glasses_id = glasses_id
	var tilt_five = XRServer.find_interface("TiltFive") as TiltFiveXRInterface
	if tilt_five:
		tilt_five.reserve_glasses(glasses_id, display_name, self)
		while true:
			var result = await tilt_five.glasses_event
			if result[0] != glasses_id: 
				continue
			elif result[1] == 5:
				return
			else: 
				break
