#include <Logging.h>
#include <Wand.h>
#include <iostream>

namespace T5Integration {

extern std::mutex g_t5_exclusivity_group_1;

void Wand::update_from_stream_event(T5_WandStreamEvent& event) {
	switch (event.type) {
		case kT5_WandStreamEventType_Connect: {
			if (_handle == 0)
				_handle = event.wandId;
			_state = WandState::SYNCED | WandState::CONNECTED;
			_buttons = WandButtons{};
			_analog = WandAnalog{};
			_pose = WandPose{};
			_battery = 0;
		} break;
		case kT5_WandStreamEventType_Disconnect: {
			_state = WandState::SYNCED;
		} break;
		case kT5_WandStreamEventType_Report: {
			_state = WandState::SYNCED | WandState::CONNECTED;
			if (event.report.buttonsValid) {
				_state |= WandState::BUTTONS_VALID;
				_buttons.t5 = event.report.buttons.t5;
				_buttons.one = event.report.buttons.one;
				_buttons.two = event.report.buttons.two;
				_buttons.three = event.report.buttons.three;
				_buttons.a = event.report.buttons.a;
				_buttons.b = event.report.buttons.b;
				_buttons.x = event.report.buttons.x;
				_buttons.y = event.report.buttons.y;
			} else
				_state &= ~WandState::BUTTONS_VALID;
			if (event.report.analogValid) {
				_state |= WandState::ANALOG_VALID;
				_analog.trigger = event.report.trigger;
				_analog.stick = event.report.stick;
			} else
				_state &= ~WandState::ANALOG_VALID;

			if (event.report.poseValid) {
				_state |= WandState::POSE_VALID;
				_pose.rotToWND_GBD = event.report.rotToWND_GBD;
				_pose.posAim_GBD = event.report.posAim_GBD;
				_pose.posFingertips_GBD = event.report.posFingertips_GBD;
				_pose.posGrip_GBD = event.report.posGrip_GBD;
			} else
				_state &= ~WandState::POSE_VALID;
			if (event.report.batteryValid) {
				_state |= WandState::BATTERY_VALID;
				_battery = event.report.battery;
			} else
				_state &= ~WandState::BATTERY_VALID;

		} break;
	}
}

void Wand::update_from_wand(const Wand& other_wand) {
	_state = other_wand._state;
	if (_state & WandState::BUTTONS_VALID)
		_buttons = other_wand._buttons;
	if (_state & WandState::ANALOG_VALID)
		_analog = other_wand._analog;
	if (_state & WandState::POSE_VALID)
		_pose = other_wand._pose;
	if (_state & WandState::BATTERY_VALID)
		_battery = other_wand._battery;
}

bool WandService::start(T5_Glasses handle) {
	_glasses_handle = handle;
	_last_wand_error = T5_SUCCESS;
	_running = true;
	_thread = jthrd::jthread([this](jthrd::stop_token s_token) { monitor_wands(s_token); });
	return _running;
}

void WandService::stop() {
	_thread.get_stop_source().request_stop();
	if (_thread.joinable())
		_thread.join();
}
bool WandService::is_running() {
	return _running;
}

void WandService::get_wand_data(WandList& list) {
	std::lock_guard lock(_list_access);
	list = _wand_list;
}

bool WandService::configure_wand_tracking(bool enable) {
	T5_Result result = T5_SUCCESS;

	for (int tries = 0; tries < 10; ++tries) {
		T5_WandStreamConfig config{ enable };
		T5_Result result;
		{
			std::lock_guard lock(g_t5_exclusivity_group_1);
			result = t5ConfigureWandStreamForGlasses(_glasses_handle, &config);
		}
		if (result != T5_ERROR_NO_SERVICE && result != T5_ERROR_IO_FAILURE) {
			break;
		}
		std::this_thread::sleep_for(_poll_rate_for_retry);
	}

	if (result != T5_SUCCESS) {
		_last_wand_error = result;
		return false;
	}
	return true;
}

T5_Result WandService::get_last_error() {
	auto tmp = _last_wand_error;
	_last_wand_error = T5_SUCCESS;
	return tmp;
}

void WandService::monitor_wands(jthrd::stop_token s_token) {
	if (!configure_wand_tracking(true))
		return;

	while (!s_token.stop_requested()) {
		T5_WandStreamEvent event;
		// g_t5_exclusivity_group_2 but can't conflict with anything currently
		auto result = t5ReadWandStreamForGlasses(_glasses_handle, &event, _wait_time_for_wand_IO);
		if (result == T5_TIMEOUT)
			continue;
		else if (result != T5_SUCCESS) {
			_last_wand_error = result;
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}

		if (event.type != kT5_WandStreamEventType_Desync) {
			std::lock_guard lock(_list_access);
			auto wand_ptr = find_wand(_wand_list, event.wandId);
			auto wand = wand_ptr ? wand_ptr : &_wand_list.emplace_back();
			wand->update_from_stream_event(event);
		} else {
			std::lock_guard lock(_list_access);
			for (auto& wand : _wand_list) {
				wand._state = 0;
			}
		}
	}

	configure_wand_tracking(false);
	_running = false;
}
} //namespace T5Integration
