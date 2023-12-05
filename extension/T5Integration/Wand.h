#pragma once
#include <thread>
#include <chrono>
#include <vector>
#include <TiltFiveNative.h>


using namespace std::chrono_literals;

namespace T5Integration {

namespace WandState {
const uint8_t SYNCED = 0x01;
const uint8_t CONNECTED = 0x02;
const uint8_t BUTTONS_VALID = 0x04;
const uint8_t ANALOG_VALID = 0x08;
const uint8_t POSE_VALID = 0x10;
const uint8_t BATTERY_VALID = 0x20;
};

struct WandButtons {
	bool t5:1;
	bool one:1;
	bool two:1;
	bool three:1;
	bool a:1;
	bool b:1;
	bool x:1;
	bool y:1;
};

struct WandAnalog {
	float trigger;
	T5_Vec2 stick;
};

struct WandPose {
	T5_Quat rotToWND_GBD;
	T5_Vec3 posAim_GBD;
	T5_Vec3 posFingertips_GBD;
	T5_Vec3 posGrip_GBD;
};

struct Wand {
	T5_WandHandle _handle;
	uint8_t _state;
	WandButtons _buttons;
	WandAnalog _analog;
	WandPose _pose;
	uint8_t _battery;

	void update_from_stream_event(T5_WandStreamEvent& event);
	void update_from_wand(const Wand& other_wand);
};

using WandList = std::vector<Wand>;

class WandService {
public:
	bool start(T5_Glasses handle);
	void stop();
	bool is_running();

	void get_wand_data(WandList& list);

	T5_Result get_last_error();

private:
	bool configure_wand_tracking(bool enable);
	void monitor_wands(std::stop_token s_token);

	T5_Glasses _glasses_handle;
	WandList _wand_list;

	std::jthread _thread;
	std::mutex _list_access;
	std::atomic_bool _running;

	std::chrono::milliseconds _poll_rate_for_retry = 20ms;
	uint32_t _wait_time_for_wand_IO = 100;

	T5_Result _last_wand_error;
};

inline Wand* find_wand(WandList& list, T5_WandHandle handle) {
	auto it = std::find_if(list.begin(), list.end(),
						   [handle](auto& test_wand) {
							   return test_wand._handle == handle;
						   });

	if(it != list.end())
		return std::addressof(*it);
	return nullptr;
}

} // T5Integration