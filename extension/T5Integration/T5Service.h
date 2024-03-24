#pragma once
#include <Glasses.h>
#include <StateFlags.h>
#include <TaskSystem.h>
#include <memory>
#include <string>
#include <vector>

namespace T5Integration {

using TaskSystem::CotaskPtr;
using TaskSystem::run_in_foreground;
using TaskSystem::Scheduler;
using TaskSystem::task_sleep;

extern std::mutex g_t5_exclusivity_group_1;
//extern std::mutex g_t5_exclusivity_group_2;

// Required minimum t5 service version
const int t5_version_major = 1;
const int t5_version_minor = 4;
const int t5_version_revision = 0;

// clang-format off
namespace T5ServiceState {
	const uint16_t RUNNING					= 0x0001; 
	const uint16_t STARTING 				= 0x0002;

	const uint16_t T5_UNAVAILABLE			= 0x0008; 
	const uint16_t T5_INCOMPATIBLE_VERSION	= 0x0010; 
	const uint16_t ERROR					= 0x1000; 
};
// clang-format on

using T5ServiceFlags = StateFlags<uint16_t>;

// clang-format off
struct T5ServiceEvent {
	enum EType
	{
		E_NONE						= 0,
		E_STOPPED					= 1,
		E_RUNNING					= 2,
		E_T5_UNAVAILABLE			= 3,
		E_T5_INCOMPATIBLE_VERSION	= 4
	};
	// clang-format on

	T5ServiceEvent(EType evt) :
			event(evt) {}

	EType event;
};

class T5Service {
public:
	using Ptr = std::shared_ptr<T5Service>;

	T5Service();
	virtual ~T5Service();

	bool start_service(const std::string_view applicationID, std::string_view applicationVersion, uint8_t sdk_type = 0, void* platform_context = nullptr);
	void stop_service();
	bool is_service_started();

	void reserve_glasses(int glasses_num, const std::string_view display_name);
	void release_glasses(int glasses_num);

	void set_upside_down_texture(int glasses_num, bool is_upside_down);

	int get_glasses_count() { return _glasses_list.size(); }
	std::optional<int> find_glasses_idx(const std::string_view glasses_id);

	const std::string get_glasses_id(int glasses_idx) const;
	const std::string get_glasses_name(int glasses_idx) const;

	void update_connection();
	void update_tracking();
	void get_service_events(std::vector<T5ServiceEvent>& out_events);
	void get_glasses_events(std::vector<GlassesEvent>& out_events);

	T5_GraphicsApi get_graphics_api() const;
	void* get_graphics_context_handle();
	bool is_image_texture_array() const;

	void get_gameboard_size(T5_GameboardType gameboard_type, T5_GameboardSize& gameboard_size);

protected:
	CotaskPtr startup_checks();
	CotaskPtr query_t5_service_version();
	CotaskPtr query_glasses_list();

	virtual std::unique_ptr<Glasses> create_glasses(const std::string_view id);

	virtual bool should_glasses_be_reserved(int glasses_idx) { return true; }

	virtual void connection_updated() {}
	virtual void tracking_updated() {}

	void set_graphics_context(const T5_GraphicsContextGL& opengl_context);
	void set_graphics_context(const T5_GraphicsContextVulkan& vulkan_context);

protected:
	std::string _application_id;
	std::string _application_version;

	T5_Context _context = nullptr;
	std::string _t5_service_version;
	std::vector<Glasses::Ptr> _glasses_list;

	T5ServiceFlags _state;
	T5ServiceFlags _previous_event_state;

	std::chrono::milliseconds _poll_rate_for_monitoring = 2s;
	std::chrono::milliseconds _poll_rate_for_retry = 100ms;

	T5_Result _last_error;

	Scheduler::Ptr _scheduler;
	T5_GraphicsApi _graphics_api;
	T5_GraphicsContextGL _opengl_graphics_context;
	T5_GraphicsContextVulkan _vulkan_graphics_context;
};

inline void T5Service::set_graphics_context(const T5_GraphicsContextGL& opengl_context) {
	_graphics_api = T5_GraphicsApi::kT5_GraphicsApi_GL;
	_opengl_graphics_context = opengl_context;
}

inline void T5Service::set_graphics_context(const T5_GraphicsContextVulkan& vulkan_context) {
	_graphics_api = T5_GraphicsApi::kT5_GraphicsApi_Vulkan;
	_vulkan_graphics_context = vulkan_context;
}

inline T5_GraphicsApi T5Service::get_graphics_api() const {
	return _graphics_api;
}

inline bool T5Service::is_image_texture_array() const {
	if (_graphics_api == T5_GraphicsApi::kT5_GraphicsApi_GL &&
			_opengl_graphics_context.textureMode == T5_GraphicsApi_GL_TextureMode::kT5_GraphicsApi_GL_TextureMode_Array)
		return true;
	return false;
}

inline void* T5Service::get_graphics_context_handle() {
	switch (_graphics_api) {
		case T5_GraphicsApi::kT5_GraphicsApi_GL:
			return &_opengl_graphics_context;
		case T5_GraphicsApi::kT5_GraphicsApi_Vulkan:
			return &_vulkan_graphics_context;
		default:
			break;
	};
	return nullptr;
}

inline const std::string T5Service::get_glasses_id(int glasses_idx) const {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_id() : std::string();
}

inline const std::string T5Service::get_glasses_name(int glasses_idx) const {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_name() : std::string();
}

inline void T5Service::set_upside_down_texture(int glasses_idx, bool is_upside_down) {
	if (glasses_idx < _glasses_list.size())
		_glasses_list[glasses_idx]->set_upside_down_texture(is_upside_down);
}

} //namespace T5Integration
