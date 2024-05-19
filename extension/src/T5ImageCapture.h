#ifndef T5_IMAGE_CAPTURE_H
#define T5_IMAGE_CAPTURE_H

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_data_container.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/xr_pose.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <Camera.h>
#include <GodotT5Glasses.h>

using godot::Image;
using godot::Node3D;
using godot::PackedByteArray;
using godot::Ref;
using godot::Texture2D;
using godot::Vector2i;
using godot::XRPose;
using GodotT5Integration::GodotT5Glasses;
using T5Integration::Camera;

class T5ImageCapture : public Node3D {
	GDCLASS(T5ImageCapture, Node3D);

public:
	void set_glasses(GodotT5Glasses::Ptr glasses);

	bool start_capture();
	void stop_capture();

	bool acquire_buffer();
	void release_buffer();

	PackedByteArray get_image_data();
	Transform3D get_camera_transform();
	Vector2i get_image_size() const;
	int get_image_stride() const;
	int get_frame_illumination_mode() const;

protected:
	static void _bind_methods();

	int _acquired_buffer_idx = -1;
	Camera _camera;
};

inline void T5ImageCapture::set_glasses(GodotT5Glasses::Ptr glasses) {
	_camera.set_glasses(glasses);
}

#endif // T5_IMAGE_CAPTURE_H