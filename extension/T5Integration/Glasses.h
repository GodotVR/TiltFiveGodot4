#pragma once

#include <optional>
#include <TaskSystem.h>
#include <StateFlags.h>
#include <T5Math.h>
#include <Wand.h>

namespace T5Integration {

using namespace std::chrono_literals;
using GlassesFlags = StateFlags<uint16_t>;
class T5Service;
using TaskSystem::CotaskPtr;
using TaskSystem::Scheduler;

float const g_default_fov = 48.0f;

namespace GlassesState {
	const uint16_t READY				= 0x00000001;
	const uint16_t GRAPHICS_INIT		= 0x00000002;
	const uint16_t SUSTAIN_CONNECTION	= 0x00000004;

	const uint16_t CREATED				= 0x00000008;
	const uint16_t UNAVAILABLE			= 0x00000010;
	const uint16_t TRACKING				= 0x00000020;
	const uint16_t CONNECTED			= 0x00000040;
	const uint16_t TRACKING_WANDS		= 0x00000080;
	const uint16_t ERROR				= 0x00000100;
}

class Glasses 
{
    friend T5Service;

    public:
	using Ptr = std::shared_ptr<Glasses>;

    enum Eye 
    {
        Mono,
        Left,
        Right
    };

    Glasses(const std::string& id);

    ~Glasses();

    const std::string get_id();    
    const std::string get_name();
    bool is_tracking();
    bool is_connected();
    
    uint32_t get_current_state();
    uint32_t get_changed_state();

    bool allocate_handle(T5_Context context);
    void destroy_handle();
    void connect(std::string application_name);
    void disconnect();

    float get_ipd();
    float get_fov();
    void get_display_size(int& out_width, int& out_height);

	void get_pose(T5_Vec3& out_position, T5_Quat& out_orientation);

    void get_glasses_position(float& out_pos_x, float& out_pos_y, float& out_pos_z);
    void get_glasses_orientation(float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w);
	void send_frame(intptr_t leftEyeTexture, intptr_t rightEyeTexture);

    void set_upside_down_texture(bool is_upside_down);

    bool update_connection();
    bool update_tracking();

	size_t get_num_wands() { return _wand_list.size(); }

    bool is_wand_state_set(size_t wand_num, uint8_t flags);
    bool is_wand_state_changed(size_t wand_num, uint8_t flags);
	bool is_wand_pose_valid(size_t wand_num);
	void get_wand_position(size_t wand_num, float& out_pos_x, float& out_pos_y, float& out_pos_z);
	void get_wand_orientation(size_t wand_num, float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w);
    void get_wand_trigger(size_t wand_num, float& out_trigger);
    void get_wand_stick(size_t wand_num, float& out_stick_x, float& out_stick_y);
    void get_wand_buttons(size_t wand_num, WandButtons& buttons);



private:

	CotaskPtr monitor_connection();
	CotaskPtr monitor_parameters();
	CotaskPtr monitor_wands();
	CotaskPtr query_ipd();
	CotaskPtr query_friendly_name();

	bool reserve();
	bool make_ready();
	bool initialize_graphics();

	void configure_wand_tracking();

	void update_pose();

	void get_eye_position(Eye eye, T5_Vec3& pos);

	private:

	Scheduler::Ptr _scheduler; 
	T5Math::Ptr _math;

	std::string _id;
	std::string _application_name;
	std::string _friendly_name;
	T5_Glasses _glasses_handle = nullptr;

	T5_GlassesPose _glasses_pose;

	float _ipd = 0.059f;

	GlassesFlags _state;

    bool _is_upside_down_texture = false;

	WandList _wand_list;
    std::vector<uint8_t> _previous_wand_state;

	std::chrono::milliseconds _poll_rate_for_connecting = 100ms;
	std::chrono::milliseconds _poll_rate_for_monitoring = 2s;
	std::chrono::milliseconds _wait_time_for_wand_IO = 100s;

};

inline const std::string Glasses::get_id()
{ 
    return _id; 
}
   
inline const std::string Glasses::get_name() {
    return _friendly_name;
}

inline bool Glasses::is_tracking() 
{ 
    return _state.is_current(GlassesState::TRACKING); 
}

inline bool Glasses::is_connected() 
{ 
    return _state.is_current(GlassesState::CONNECTED); 
}

inline uint32_t Glasses::get_current_state() 
{ 
    return _state.get_current(); 
}

inline uint32_t Glasses::get_changed_state() 
{ 
    return _state.get_then_update_changes(); 
}

inline float Glasses::get_ipd()
{
    return static_cast<float>(_ipd);
}

inline float Glasses::get_fov()
{
    return 48.0f;
}

inline void Glasses::get_display_size(int& width, int& height) 
{ 
    width = 1216; 
    height = 768; 
}

inline void Glasses::set_upside_down_texture(bool is_upside_down) {
    _is_upside_down_texture = is_upside_down;
}

struct GlassesEvent {
    enum EType
    {
        E_NONE          = 0,
        E_ADDED         = 1,
        E_LOST          = 2,
        E_AVAILABLE     = 3,
        E_UNAVAILABLE   = 4,
        E_CONNECTED     = 5,
        E_DISCONNECTED  = 6,
        E_TRACKING      = 7,
        E_NOT_TRACKING  = 8,
        E_STOPPED_ON_ERROR = 9
    };
    GlassesEvent(int num, EType evt) 
    : glasses_num(num), event(evt)
    {}

    int glasses_num;
    EType event;
};


}