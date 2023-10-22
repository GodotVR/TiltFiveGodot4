class_name T5GlassesBase extends SubViewport

@onready var wand = $Origin/Wand_1

func _process(_delta):
	if wand:
		var wand_pose : XRPose = wand.get_pose()
		if wand_pose:
			wand.visible = wand_pose.tracking_confidence != XRPose.XR_TRACKING_CONFIDENCE_NONE
		else:
			wand.visible = false
