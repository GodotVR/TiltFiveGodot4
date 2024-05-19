#ifndef _CAMERA_H
#define _CAMERA_H

#include <TiltFiveNative.h>
#include <array>
#include <chrono>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

#include <Glasses.h>

using namespace std::chrono_literals;

namespace T5Integration {

const int camera_buffer_count = 3;
const size_t camera_buffer_size = T5_MIN_CAM_IMAGE_BUFFER_WIDTH * T5_MIN_CAM_IMAGE_BUFFER_HEIGHT;

using CameraBuffer = std::array<uint8_t, camera_buffer_size>;

class Camera {
public:
	void set_camera_idx(uint8_t idx);
	void set_glasses(Glasses::Ptr glasses);

	bool start_capture();
	void stop_capture();
	bool is_capturing() const;

	void release_filled_buffer(int buffer_index);
	int acquire_filled_buffer();
	std::span<uint8_t> get_buffer(int buffer_index);
	int get_image_width(int buffer_index) const;
	int get_image_height(int buffer_index) const;
	int get_image_stride(int buffer_index) const;
	uint8_t get_illumination_mode(int buffer_index) const;
	void get_camera_position(int buffer_index, float& out_pos_x, float& out_pos_y, float& out_pos_z) const;
	void get_camera_orientation(int buffer_index, float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w) const;

private:
	bool configure_camera(bool enable);

	bool _is_captured = false;
	uint8_t _camera_idx = 0;
	std::weak_ptr<Glasses> _glasses;

	std::array<CameraBuffer, camera_buffer_count> _camera_buffers;
	std::array<T5_CamImage, camera_buffer_count> _camera_buffer_info;
};

inline void Camera::set_camera_idx(uint8_t idx) {
	_camera_idx = idx;
}

inline bool Camera::is_capturing() const {
	return _is_captured;
}

inline void Camera::set_glasses(Glasses::Ptr glasses) {
	_glasses = glasses;
}

} //namespace T5Integration

#endif //_CAMERA_H