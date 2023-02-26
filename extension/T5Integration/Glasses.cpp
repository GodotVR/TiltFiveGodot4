#include <ObjectRegistry.h>
#include <Glasses.h>
#include <Logging.h>
#include <Wand.h>

using TaskSystem::task_sleep;
using TaskSystem::run_in_foreground;
using TaskSystem::run_now;

namespace T5Integration {


	Glasses::Glasses( const std::string& id)
	:	_id(id) {

		_scheduler = ObjectRegistry::scheduler();
		_math = ObjectRegistry::math();

		_glasses_pose.posGLS_GBD.x = 0;
		_glasses_pose.posGLS_GBD.y = 0;
		_glasses_pose.posGLS_GBD.z = 0;
		_glasses_pose.rotToGLS_GBD.x = 0;
		_glasses_pose.rotToGLS_GBD.y = 0;
		_glasses_pose.rotToGLS_GBD.z = 0;
		_glasses_pose.rotToGLS_GBD.w = 1;

		_state.reset(GlassesState::UNAVAILABLE, true);

	}

	Glasses::~Glasses() {
		destroy_handle();
	}

	void Glasses::get_pose(T5_Vec3& out_position, T5_Quat& out_orientation) {
		out_position = _glasses_pose.posGLS_GBD;
		out_orientation = _glasses_pose.rotToGLS_GBD;
	}


	void Glasses::get_glasses_position(float& out_pos_x, float& out_pos_y, float& out_pos_z) {
		out_pos_x = _glasses_pose.posGLS_GBD.x;
		out_pos_y = _glasses_pose.posGLS_GBD.y;
		out_pos_z = _glasses_pose.posGLS_GBD.z;
	}

	void Glasses::get_glasses_orientation(float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w) {
		out_quat_x = _glasses_pose.rotToGLS_GBD.x;
		out_quat_y = _glasses_pose.rotToGLS_GBD.y;
		out_quat_z = _glasses_pose.rotToGLS_GBD.z;
		out_quat_w = _glasses_pose.rotToGLS_GBD.w;
	}    
	
	bool Glasses::is_wand_state_set(size_t wand_num, uint8_t flags) {
		return wand_num < _wand_list.size() && (_wand_list[wand_num]._state & flags) == flags;
	}

    bool Glasses::is_wand_state_changed(size_t wand_num, uint8_t flags) {
		if(wand_num >= _wand_list.size()) return false;

		auto current_state = _wand_list[wand_num]._state;
		auto previous_state = _previous_wand_state[wand_num];
		auto changed = current_state ^ previous_state;
		_previous_wand_state[wand_num] = previous_state & ~changed | current_state & changed;
		return (changed & flags) != 0;
	}

	bool Glasses::is_wand_pose_valid(size_t wand_num) {
		return wand_num < _wand_list.size() && (_wand_list[wand_num]._state & WandState::POSE_VALID) != 0;
	}

	void Glasses::get_wand_position(size_t wand_num, float& out_pos_x, float& out_pos_y, float& out_pos_z) {
		if (wand_num < _wand_list.size()) {
			out_pos_x = _wand_list[wand_num]._pose.posAim_GBD.x;
			out_pos_y = _wand_list[wand_num]._pose.posAim_GBD.y;
			out_pos_z = _wand_list[wand_num]._pose.posAim_GBD.z;
		}
		else {
			out_pos_x = out_pos_y = out_pos_z = 0;
		}
	}

	void Glasses::get_wand_orientation(size_t wand_num, float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w) {
		if (wand_num < _wand_list.size()) {
			out_quat_x = _wand_list[wand_num]._pose.rotToWND_GBD.x;
			out_quat_y = _wand_list[wand_num]._pose.rotToWND_GBD.y;
			out_quat_z = _wand_list[wand_num]._pose.rotToWND_GBD.z;
			out_quat_w = _wand_list[wand_num]._pose.rotToWND_GBD.w;
		}
		else {
			out_quat_x = out_quat_y = out_quat_z = 0;
			out_quat_w = 1;
		}
	}

    void Glasses::get_wand_trigger(size_t wand_num, float& out_trigger) {
		out_trigger = 0;
		if (wand_num < _wand_list.size()) {
			out_trigger = _wand_list[wand_num]._analog.trigger;
		}
	}
	
    void Glasses::get_wand_stick(size_t wand_num, float& out_stick_x, float& out_stick_y) {
		out_stick_x = 0;
		out_stick_y = 0;
		if (wand_num < _wand_list.size()) {
			out_stick_x = _wand_list[wand_num]._analog.stick.x;
			out_stick_y = _wand_list[wand_num]._analog.stick.y;
		}
	}

	void Glasses::get_wand_buttons(size_t wand_num, WandButtons& buttons) {
		if (wand_num < _wand_list.size()) {
			buttons = _wand_list[wand_num]._buttons;
		}
	}

	bool Glasses::allocate_handle(T5_Context context) {
		T5_Result result; 
		{ 
			std::lock_guard lock(g_t5_exclusivity_group_1);
			result = t5CreateGlasses(context, _id.c_str(), &_glasses_handle);
		}
		if (result != T5_SUCCESS) {
			LOG_T5_ERROR(result);
			return false;
		}
		_state.set(GlassesState::CREATED);
		_state.clear(GlassesState::UNAVAILABLE);
		_scheduler->add_task(monitor_parameters());

		return true;
	}

	void Glasses::destroy_handle() {
		_state.clear_all();
		{
			std::lock_guard lock(g_t5_exclusivity_group_1);
			t5DestroyGlasses(&_glasses_handle);
		}
		_glasses_handle = nullptr;
	}

	CotaskPtr Glasses::monitor_connection() {
		T5_Result result;

		while (_glasses_handle && _state.is_current(GlassesState::SUSTAIN_CONNECTION)) {
			T5_ConnectionState connectionState;

			{
				std::lock_guard lock(g_t5_exclusivity_group_1);
				result = t5GetGlassesConnectionState(_glasses_handle, &connectionState);
			}
			if (result != T5_SUCCESS) {
				// Doesn't seem to be anything recoverable here
				co_await run_in_foreground;
				LOG_T5_ERROR(result);
				_state.reset(GlassesState::ERROR);
				co_return;
			}

			switch (connectionState) {
			case kT5_ConnectionState_NotExclusivelyConnected:
			{
				_state.clear(GlassesState::READY);
				{
					std::lock_guard lock(g_t5_exclusivity_group_1);
					result = t5ReserveGlasses(_glasses_handle, _application_name.c_str());
				}
				if (result == T5_SUCCESS || result == T5_ERROR_ALREADY_CONNECTED)
					continue;
				else if (result == T5_ERROR_UNAVAILABLE) {
					// Some else has the glasses so stop 
					// trying to connect
					_state.clear(GlassesState::SUSTAIN_CONNECTION);
					_state.set(GlassesState::UNAVAILABLE);
					co_return;
				}
				else if (result == T5_ERROR_DEVICE_LOST) {
					_state.clear(GlassesState::SUSTAIN_CONNECTION);
					co_await run_in_foreground;
					LOG_T5_ERROR(result);
					destroy_handle();
					co_return;
				}
				_state.reset(GlassesState::ERROR);
				co_await run_in_foreground;
				LOG_T5_ERROR(result);
				co_return;
			}
			case kT5_ConnectionState_ExclusiveReservation:
			case kT5_ConnectionState_Disconnected:
			{
				_state.clear(GlassesState::READY);

				{
					std::lock_guard lock(g_t5_exclusivity_group_1);
					result = t5EnsureGlassesReady(_glasses_handle);
				}
				if (result == T5_SUCCESS)
					continue;
				else if (result == T5_ERROR_TRY_AGAIN) {
					break;
				}
				if (result == T5_ERROR_UNAVAILABLE) {
					// Some else has the glasses so stop 
					// trying to connect
					_state.clear(GlassesState::SUSTAIN_CONNECTION);
					_state.set(GlassesState::UNAVAILABLE);
				}
				else if (result == T5_ERROR_DEVICE_LOST) {
					_state.clear(GlassesState::SUSTAIN_CONNECTION);
					co_await run_in_foreground;
					LOG_T5_ERROR(result);
					destroy_handle();
				}
				else {
					_state.reset(GlassesState::ERROR);
					co_await run_in_foreground;
					LOG_T5_ERROR(result);
				}
				co_return;
			}
			case kT5_ConnectionState_ExclusiveConnection:
			{
				_state.set(GlassesState::READY);
				if (!_state.is_current(GlassesState::GRAPHICS_INIT)) {
					co_await run_in_foreground;
					initialize_graphics();
				}
				break;
			}
			}

			if (_state.any_changed(GlassesState::READY | GlassesState::GRAPHICS_INIT)) {
				if (_state.is_current(GlassesState::READY | GlassesState::GRAPHICS_INIT))
					_state.set(GlassesState::CONNECTED);
				else
					_state.clear(GlassesState::CONNECTED);
			}
			if (_state.is_current(GlassesState::READY) && !_state.is_current(GlassesState::TRACKING_WANDS)) {
				_state.set(GlassesState::TRACKING_WANDS);
				_scheduler->add_task(monitor_wands());
			}

			co_await task_sleep(
				_state.is_current(GlassesState::CONNECTED) ?
				_poll_rate_for_monitoring :
				_poll_rate_for_connecting);
		}
	}

	CotaskPtr Glasses::monitor_parameters() {
		co_await query_ipd();
		co_await query_friendly_name();

		T5_Result result;
		std::vector<T5_ParamGlasses> _changed_params;

		while (_glasses_handle && _state.is_current(GlassesState::CREATED)) {
			co_await task_sleep(_poll_rate_for_monitoring);

			uint16_t buffer_size = 16;
			_changed_params.resize(buffer_size);
			{
				std::lock_guard lock(g_t5_exclusivity_group_1);
				result = t5GetChangedGlassesParams(_glasses_handle, _changed_params.data(), &buffer_size);
			}
			if (result != T5_SUCCESS) {
				co_await run_in_foreground;
				LOG_T5_ERROR(result);
				co_return;
			}

			if (buffer_size == 0)
				continue;

			_changed_params.resize(buffer_size);
			for (auto param : _changed_params) {
				switch (param) {
				case kT5_ParamGlasses_Float_IPD:
				{
					co_await query_ipd();
				}
				case kT5_ParamGlasses_UTF8_FriendlyName:
				{
					co_await query_friendly_name();
				}
				default:
				break;
				}
			}
		}
	}

	CotaskPtr Glasses::monitor_wands() {
		WandService wand_service;

		if(!wand_service.start(_glasses_handle))
			co_return;

		co_await run_in_foreground;

		if(!wand_service.is_running())
		{ 
			_state.clear(GlassesState::TRACKING_WANDS);
			LOG_T5_ERROR(wand_service.get_last_error());
			co_return;
		}
		
		while (_glasses_handle && _state.is_current(GlassesState::SUSTAIN_CONNECTION) && wand_service.is_running()) {
			
			auto err = wand_service.get_last_error();
			if (err != T5_SUCCESS) {
				LOG_T5_ERROR(err);
			}

			wand_service.get_wand_data(_wand_list);
			while(_wand_list.size() > _previous_wand_state.size())
				_previous_wand_state.push_back(0);
			co_await run_in_foreground;
		}

		wand_service.stop();
		_state.clear(GlassesState::TRACKING_WANDS); 
		auto err = wand_service.get_last_error();
		if (err != T5_SUCCESS) {
			LOG_T5_ERROR(wand_service.get_last_error());
		}
	}

	CotaskPtr Glasses::query_ipd() {
		double ipd;
		T5_Result result;
		for (int tries = 0; tries < 10; ++tries) {
			{
				std::lock_guard lock(g_t5_exclusivity_group_1);
				result = t5GetGlassesFloatParam(_glasses_handle, 0, kT5_ParamGlasses_Float_IPD, &ipd);
			}
			if (result != T5_ERROR_NO_SERVICE && result != T5_ERROR_IO_FAILURE) {
				break;
			}
			co_await task_sleep(_poll_rate_for_connecting);
		}
		co_await run_in_foreground;
		if (result != T5_SUCCESS) {
			LOG_T5_ERROR(result);
		}
		else {
			_ipd = static_cast<float>(ipd);
		}
	}

	CotaskPtr Glasses::query_friendly_name() {
		std::vector<char> buffer;
		size_t buffer_size = 64;
		buffer.resize(buffer_size);

		T5_Result result;
		for (int tries = 0; tries < 10; ++tries) {
			buffer_size = buffer.size();
			{
				std::lock_guard lock(g_t5_exclusivity_group_1);
				result = t5GetGlassesUtf8Param(_glasses_handle, 0, kT5_ParamGlasses_UTF8_FriendlyName, buffer.data(), &buffer_size);
			}
			if (result == T5_ERROR_OVERFLOW) {
				buffer.resize(buffer_size);
				continue;
			}
			else if (result != T5_ERROR_NO_SERVICE && result != T5_ERROR_IO_FAILURE) {
				break;
			}
			co_await task_sleep(_poll_rate_for_connecting);
		}
		co_await run_in_foreground;
		if (result == T5_SUCCESS) {
			buffer.resize(buffer_size);
			_friendly_name = buffer.data();
		}
		else if(result == T5_ERROR_SETTING_UNKNOWN) {
			_friendly_name = _id;
		}
		else
			LOG_T5_ERROR(result);
	}


	void Glasses::connect(std::string application_name) {
		if (_glasses_handle) {
			_application_name = application_name;

			_state.set(GlassesState::SUSTAIN_CONNECTION);
			_scheduler->add_task(monitor_connection());
		}
	}

	void Glasses::disconnect() {

		if (_state.is_current(GlassesState::READY)) {
			T5_Result result;
			{
				std::lock_guard lock(g_t5_exclusivity_group_1);
				result = t5ReleaseGlasses(_glasses_handle);
			}
			if (result != T5_SUCCESS) {
				LOG_T5_ERROR(result);
			}
		}
		_state.clear(GlassesState::READY);
	}

	bool Glasses::initialize_graphics() {

		// t5 exclusivity group 3 - serialized in main thread
		auto result = t5InitGlassesGraphicsContext(_glasses_handle, kT5_GraphicsApi_GL, nullptr);
		// T5_ERROR_INVALID_STATE seems to mean previously initialized
		bool is_graphics_initialized = (result == T5_SUCCESS || result == T5_ERROR_INVALID_STATE);
		if (!is_graphics_initialized) {
			LOG_T5_ERROR(result);
			return false;
		}

		_state.set(GlassesState::GRAPHICS_INIT);

		return true;
	}


	void Glasses::update_pose() {
		if (!_state.is_current(GlassesState::CONNECTED))
			return;

		T5_Result result;
		{
			std::lock_guard lock(g_t5_exclusivity_group_1);
			result = t5GetGlassesPose(_glasses_handle, kT5_GlassesPoseUsage_GlassesPresentation,  &_glasses_pose);
		}
		bool isTracking = (result == T5_SUCCESS);

		if (isTracking) {
			_state.set(GlassesState::TRACKING);
		}
		else {
			_state.clear(GlassesState::TRACKING);

			if (result == T5_ERROR_TRY_AGAIN)
				return;
			else if (result == T5_ERROR_NOT_CONNECTED) {
				_state.clear(GlassesState::CONNECTED);
				LOG_T5_ERROR(result);
			}
			else {
				LOG_T5_ERROR(result);
				_state.reset(GlassesState::ERROR);
			}
		}
		LOG_TOGGLE(false, isTracking, "Tracking started", "Tracking ended");
	}

	void Glasses::get_eye_position(Eye eye, T5_Vec3& pos) {
		
		float dir = (eye == Left ? -1.0f : 1.0f);
		auto ipd = get_ipd();
		pos.x = dir * ipd / 2.0f;
		pos.y = 0.0f;
		pos.z = 0.0f;

		_math->rotate_vector(
			_glasses_pose.rotToGLS_GBD.x, 
			_glasses_pose.rotToGLS_GBD.y, 
			_glasses_pose.rotToGLS_GBD.z, 
			_glasses_pose.rotToGLS_GBD.w,
			pos.x,
			pos.y,
			pos.z);

		pos.x += _glasses_pose.posGLS_GBD.x;
		pos.y += _glasses_pose.posGLS_GBD.y;
		pos.z += _glasses_pose.posGLS_GBD.z;
	}

	void Glasses::send_frame(intptr_t leftEyeTexture, intptr_t rightEyeTexture) {
		if (_state.is_current(GlassesState::TRACKING | GlassesState::CONNECTED)) {
			T5_FrameInfo frameInfo;

			int width;
			int height;
			get_display_size(width, height);

			frameInfo.vci.startY_VCI = static_cast<float>(-tan((get_fov() * 3.1415926535 / 180.0) * 0.5f));
			frameInfo.vci.startX_VCI = frameInfo.vci.startY_VCI * (float)width / (float)height;
			frameInfo.vci.width_VCI = -2.0f * frameInfo.vci.startX_VCI;
			frameInfo.vci.height_VCI = -2.0f * frameInfo.vci.startY_VCI;

			frameInfo.texWidth_PIX = width;
			frameInfo.texHeight_PIX = height;

			frameInfo.leftTexHandle = (void*)leftEyeTexture;
			frameInfo.rightTexHandle = (void*)rightEyeTexture;

			get_eye_position(Left, frameInfo.posLVC_GBD);
			frameInfo.rotToLVC_GBD = _glasses_pose.rotToGLS_GBD;

			get_eye_position(Right, frameInfo.posRVC_GBD);
			frameInfo.rotToRVC_GBD = _glasses_pose.rotToGLS_GBD;

			frameInfo.isUpsideDown = _is_upside_down_texture;
			frameInfo.isSrgb = true;

			// t5 exclusivity group 3 - serialized in main thread
			T5_Result result = t5SendFrameToGlasses(_glasses_handle, &frameInfo);

			LOG_TOGGLE(false, result == T5_SUCCESS, "Started sending frames", "Stoped sending frames");
			if (result == T5_SUCCESS)
				return;
			LOG_T5_ERROR(result);
			if (result == T5_ERROR_NOT_CONNECTED) {
				_state.clear(GlassesState::CONNECTED);
			}
			// not sure how we might get here
			else if (result == T5_ERROR_GFX_CONTEXT_INIT_FAIL || result == T5_ERROR_INVALID_GFX_CONTEXT) {
				_state.clear(GlassesState::GRAPHICS_INIT);
			}
			else {
				_state.reset(GlassesState::ERROR);
			}
		}
	}

	bool Glasses::update_connection() {

		return true;
	}

	bool Glasses::update_tracking() {
		update_pose();

		return true;
	}

}