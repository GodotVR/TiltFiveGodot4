extends Node3D

var image_capture: T5ImageCapture
var is_capturing := false
var image_size: Vector2i
var camera_image: Image
@onready var camera_view: TextureRect = $ScreenUI/CameraView

# Grab the T5ImageCapture out of the xr rig
func _on_t_5_manager_xr_rig_was_added(xr_rig: T5XRRig):
	var new_image_capture = xr_rig.get_image_capture()
	if not image_capture and new_image_capture:
		image_capture = new_image_capture
	
func _on_t_5_manager_xr_rig_will_be_removed(xr_rig):
	var old_image_capture = xr_rig.get_image_capture()
	if image_capture and image_capture == old_image_capture:
		if is_capturing:
			image_capture.stop_capture()
			is_capturing = false
		camera_view.visible = false
		image_capture = null

# Toggle the camera on and off
func _input(event):
	if image_capture == null:
		return
	if not event.is_action_pressed("toggle_camera"):
		return
	if not is_capturing and image_capture.start_capture():
		is_capturing = true
		camera_view.visible = true
	else:
		image_capture.stop_capture()
		is_capturing = false
		camera_view.visible = false

func _process(_delta):
	if not is_capturing:
		return
	# Test to see if a image buffer is available
	if image_capture.acquire_buffer():
		# get the image data
		var buffer := image_capture.get_image_data()
		# Get the size
		var new_size = image_capture.get_image_size()
		# If is the first time of the size has changed
		if camera_image == null or image_size != new_size:
			image_size = new_size
			camera_image = Image.create_from_data(image_size.x, image_size.y, false, Image.FORMAT_R8, buffer)
			camera_view.texture = ImageTexture.create_from_image(camera_image)
		else:
			camera_image.set_data(image_size.x, image_size.y, false, Image.FORMAT_R8, buffer)
			camera_view.texture.update(camera_image)
		image_capture.release_buffer()
