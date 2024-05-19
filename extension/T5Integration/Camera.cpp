#include <Camera.h>
#include <Logging.h>
#include <iostream>

namespace T5Integration {

extern std::mutex g_t5_exclusivity_group_1;

bool Camera::start_capture() {
	LOG_FAIL_COND_V(_glasses.expired(), false);
	_is_captured = configure_camera(true);

	// Put the buffers into the queue for the first time.a
	if (_is_captured) {
		for (int i = 0; i < camera_buffer_count; ++i) {
			release_filled_buffer(i);
		}
	}
	return _is_captured;
}

void Camera::stop_capture() {
	LOG_FAIL_COND(_glasses.expired());
	configure_camera(false);
}

void Camera::release_filled_buffer(int buffer_index) {
	LOG_FAIL_COND(_glasses.expired());
	LOG_FAIL_COND(!_is_captured);
	LOG_FAIL_COND(buffer_index < 0 || buffer_index >= camera_buffer_count);

	auto& buffer = _camera_buffers[buffer_index];
	auto& camImage = _camera_buffer_info[buffer_index];

	camImage.cameraIndex = 0;
	camImage.imageWidth = 0;
	camImage.imageHeight = 0;
	camImage.cameraIndex = 0;
	camImage.imageStride = 0;
	camImage.illuminationMode = 0;
	camImage.bufferSize = buffer.size();
	camImage.pixelData = buffer.data();

	T5_Result result;
	{
		std::lock_guard lock(g_t5_exclusivity_group_1);
		result = t5SubmitEmptyCamImageBuffer(_glasses.lock()->_glasses_handle, &camImage);
	}
	if (result != T5_SUCCESS) {
		LOG_T5_ERROR(result);
	}
}

int Camera::acquire_filled_buffer() {
	LOG_FAIL_COND_V(_glasses.expired(), -1);
	T5_CamImage camImage;
	T5_Result result;
	{
		std::lock_guard lock(g_t5_exclusivity_group_1);
		result = t5GetFilledCamImageBuffer(_glasses.lock()->_glasses_handle, &camImage);
	}
	if (result == T5_ERROR_TRY_AGAIN) {
		return -1;
	} else if (result != T5_SUCCESS) {
		LOG_T5_ERROR(result);
		return -1;
	}
	for (int i = 0; i < _camera_buffers.size(); ++i) {
		if (_camera_buffers[i].data() == camImage.pixelData) {
			_camera_buffer_info[i] = camImage;
			return i;
		}
	}
	LOG_ERROR("Failed to find buffer.");
	return -1;
}

std::span<uint8_t> Camera::get_buffer(int buffer_index) {
	LOG_FAIL_COND_V(buffer_index < 0 || buffer_index >= _camera_buffers.size(), std::span<uint8_t>());

	return std::span(_camera_buffers[buffer_index]);
}

int Camera::get_image_width(int buffer_index) const {
	LOG_FAIL_COND_V(buffer_index < 0 || buffer_index >= _camera_buffer_info.size(), 0);

	return _camera_buffer_info[buffer_index].imageWidth;
}

int Camera::get_image_height(int buffer_index) const {
	LOG_FAIL_COND_V(buffer_index < 0 || buffer_index >= _camera_buffer_info.size(), 0);

	return _camera_buffer_info[buffer_index].imageHeight;
}

int Camera::get_image_stride(int buffer_index) const {
	LOG_FAIL_COND_V(buffer_index < 0 || buffer_index >= _camera_buffer_info.size(), 0);

	return _camera_buffer_info[buffer_index].imageStride;
}

uint8_t Camera::get_illumination_mode(int buffer_index) const {
	LOG_FAIL_COND_V(buffer_index < 0 || buffer_index >= _camera_buffer_info.size(), false);

	return _camera_buffer_info[buffer_index].illuminationMode;
}

void Camera::get_camera_position(int buffer_index, float& out_pos_x, float& out_pos_y, float& out_pos_z) const {
	LOG_FAIL_COND(buffer_index < 0 || buffer_index >= _camera_buffer_info.size());

	out_pos_x = _camera_buffer_info[buffer_index].posCAM_GBD.x;
	out_pos_y = _camera_buffer_info[buffer_index].posCAM_GBD.y;
	out_pos_z = _camera_buffer_info[buffer_index].posCAM_GBD.z;
}

void Camera::get_camera_orientation(int buffer_index, float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w) const {
	LOG_FAIL_COND(buffer_index < 0 || buffer_index >= _camera_buffer_info.size());

	out_quat_x = _camera_buffer_info[buffer_index].rotToCAM_GBD.x;
	out_quat_y = _camera_buffer_info[buffer_index].rotToCAM_GBD.y;
	out_quat_z = _camera_buffer_info[buffer_index].rotToCAM_GBD.z;
	out_quat_w = _camera_buffer_info[buffer_index].rotToCAM_GBD.w;
}

bool Camera::configure_camera(bool enable) {
	LOG_FAIL_COND_V(_glasses.expired(), false);
	LOG_FAIL_COND_V(_camera_buffers.size() <= 0, false);

	T5_CameraStreamConfig config{ _camera_idx, enable };
	T5_Result result = T5_SUCCESS;
	{
		std::lock_guard lock(g_t5_exclusivity_group_1);
		result = t5ConfigureCameraStreamForGlasses(_glasses.lock()->_glasses_handle, config);
	}

	if (result != T5_SUCCESS) {
		LOG_T5_ERROR(result);
		return false;
	}
	return true;
}

} //namespace T5Integration
