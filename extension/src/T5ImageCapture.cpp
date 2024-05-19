#include <T5ImageCapture.h>
#include <godot_cpp/variant/quaternion.hpp>
#include <span>

using godot::ClassDB;
using godot::D_METHOD;
using godot::MethodInfo;
using godot::Quaternion;

void T5ImageCapture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_capture"), &T5ImageCapture::start_capture);
	ClassDB::bind_method(D_METHOD("stop_capture"), &T5ImageCapture::stop_capture);
	ClassDB::bind_method(D_METHOD("acquire_buffer"), &T5ImageCapture::acquire_buffer);
	ClassDB::bind_method(D_METHOD("release_buffer"), &T5ImageCapture::release_buffer);
	ClassDB::bind_method(D_METHOD("get_image_data"), &T5ImageCapture::get_image_data);
	ClassDB::bind_method(D_METHOD("get_camera_transform"), &T5ImageCapture::get_camera_transform);
	ClassDB::bind_method(D_METHOD("get_image_size"), &T5ImageCapture::get_image_size);
	ClassDB::bind_method(D_METHOD("get_image_stride"), &T5ImageCapture::get_image_stride);
	ClassDB::bind_method(D_METHOD("get_frame_illumination_mode"), &T5ImageCapture::get_frame_illumination_mode);
};

bool T5ImageCapture::start_capture() {
	_camera.set_camera_idx(0);
	return _camera.start_capture();
}

void T5ImageCapture::stop_capture() {
	_camera.stop_capture();
}

bool T5ImageCapture::acquire_buffer() {
	ERR_FAIL_COND_V(!_camera.is_capturing(), false);

	_acquired_buffer_idx = _camera.acquire_filled_buffer();
	if (_acquired_buffer_idx >= 0) {
		set_transform(get_camera_transform());
	}

	return _acquired_buffer_idx >= 0 && _acquired_buffer_idx < 3;
}

void T5ImageCapture::release_buffer() {
	if (_acquired_buffer_idx >= 0) {
		_camera.release_filled_buffer(_acquired_buffer_idx);
		_acquired_buffer_idx = -1;
	}
}

PackedByteArray T5ImageCapture::get_image_data() {
	ERR_FAIL_INDEX_V(_acquired_buffer_idx, 3, PackedByteArray());

	auto buffer = _camera.get_buffer(_acquired_buffer_idx);
	PackedByteArray packed_data;
	packed_data.resize(buffer.size());
	memcpy(packed_data.ptrw(), buffer.data(), buffer.size());

	return packed_data;
}

Transform3D T5ImageCapture::get_camera_transform() {
	ERR_FAIL_INDEX_V(_acquired_buffer_idx, 3, Transform3D());

	Quaternion orientation;
	Vector3 position;
	_camera.get_camera_position(_acquired_buffer_idx, position.x, position.y, position.z);
	_camera.get_camera_orientation(_acquired_buffer_idx, orientation.x, orientation.y, orientation.z, orientation.w);

	// Tiltfive -> Godot axis
	position = Vector3(position.x, position.z, -position.y);
	orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
	orientation = orientation.inverse();

	Transform3D local_transform;
	local_transform.set_origin(position);
	local_transform.set_basis(orientation);

	return local_transform;
}

Vector2i T5ImageCapture::get_image_size() const {
	ERR_FAIL_INDEX_V(_acquired_buffer_idx, 3, Vector2i());
	auto width = _camera.get_image_width(_acquired_buffer_idx);
	auto height = _camera.get_image_height(_acquired_buffer_idx);
	return Vector2i(width, height);
}

int T5ImageCapture::get_image_stride() const {
	ERR_FAIL_INDEX_V(_acquired_buffer_idx, 3, 0);
	return _camera.get_image_stride(_acquired_buffer_idx);
}

int T5ImageCapture::get_frame_illumination_mode() const {
	ERR_FAIL_INDEX_V(_acquired_buffer_idx, 3, false);
	return _camera.get_illumination_mode(_acquired_buffer_idx);
}
