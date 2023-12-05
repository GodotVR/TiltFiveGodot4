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
	const uint16_t READY				= 0x00000001; //0000000001
	const uint16_t GRAPHICS_INIT		= 0x00000002; //0000000010
	const uint16_t SUSTAIN_CONNECTION	= 0x00000004; //0000000100

	const uint16_t CREATED				= 0x00000008; //0000001000
	const uint16_t UNAVAILABLE			= 0x00000010; //0000010000
	const uint16_t TRACKING				= 0x00000020; //0000100000
	const uint16_t CONNECTED			= 0x00000040; //0001000000
	const uint16_t TRACKING_WANDS		= 0x00000080; //0010000000
	const uint16_t ERROR				= 0x00000100; //0100000000
	const uint16_t DISPLAY_STARTED		= 0x00000200; //1000000000
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

class Glasses 
{
    friend T5Service;

    protected:
    struct SwapChainFrame {
        SwapChainFrame();
        T5_GlassesPose glasses_pose;
        intptr_t left_eye_handle;
        intptr_t right_eye_handle;
    };

    public:
	using Ptr = std::shared_ptr<Glasses>;

    enum Eye 
    {
        Mono,
        Left,
        Right
    };

    Glasses(const std::string_view id);
    virtual ~Glasses();

    const std::string get_id();    
    const std::string get_name();
    bool is_connected();
    bool is_available();
    bool is_tracking();
    
    bool allocate_handle(T5_Context context);
    void destroy_handle();
    void connect(const std::string_view application_name);
    void disconnect();
	void start_display();
	void stop_display();


    float get_ipd();
    float get_fov();
    void get_display_size(int& out_width, int& out_height);

	void get_pose(T5_Vec3& out_position, T5_Quat& out_orientation);

    void get_glasses_position(float& out_pos_x, float& out_pos_y, float& out_pos_z);
    void get_glasses_orientation(float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w);

    int get_current_frame_idx() { return _current_frame_idx; }
	void send_frame();

    void set_upside_down_texture(bool is_upside_down);

    bool update_connection();
    bool update_tracking();

   	void get_events(int index, std::vector<GlassesEvent>& out_events);
    T5_GameboardType get_gameboard_type();

	int get_num_wands() { return _wand_list.size(); }

    bool is_wand_state_set(int wand_num, uint8_t flags);
    bool is_wand_state_changed(int wand_num, uint8_t flags);
	bool is_wand_pose_valid(int wand_num);
	void get_wand_position(int wand_num, float& out_pos_x, float& out_pos_y, float& out_pos_z);
	void get_wand_orientation(int wand_num, float& out_quat_x, float& out_quat_y, float& out_quat_z, float& out_quat_w);
    void get_wand_trigger(int wand_num, float& out_trigger);
    void get_wand_stick(int wand_num, float& out_stick_x, float& out_stick_y);
    void get_wand_buttons(int wand_num, WandButtons& buttons);

    void trigger_haptic_pulse(int wand_num, float amplitude, uint16_t duration);

    virtual void on_post_draw() {}

protected:

    void set_swap_chain_size(int size);
    void set_swap_chain_texture_pair(int swap_chain_idx, intptr_t left_eye_handle, intptr_t right_eye_handle);
    void set_swap_chain_texture_array(int swap_chain_idx, intptr_t array_handle);

	virtual void on_start_display() {}
	virtual void on_stop_display() {}
    virtual void on_glasses_reserved() {}
    virtual void on_glasses_released() {}
    virtual void on_glasses_dropped() {}
    virtual void on_tracking_updated() {}
    virtual void on_send_frame(int swap_chain_idx) {}

    GlassesFlags::FlagType get_current_state();
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

    void begin_reserved_state();
    void end_reserved_state();

	private:

	Scheduler::Ptr _scheduler; 
	T5Math::Ptr _math;

	std::string _id;
	std::string _application_name;
	std::string _friendly_name;
	T5_Glasses _glasses_handle = nullptr;

    int _current_frame_idx = 0;
    std::vector<SwapChainFrame> _swap_chain_frames;

	float _ipd = 0.059f;

	GlassesFlags _state;
	GlassesFlags _previous_event_state;
	GlassesFlags _previous_update_state;

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

inline bool Glasses::is_connected() 
{ 
    return _state.is_current(GlassesState::CONNECTED); 
}

inline bool Glasses::is_available() 
{ 
    return !(_state.is_current(GlassesState::SUSTAIN_CONNECTION) || _state.is_current(GlassesState::UNAVAILABLE)); 
}

inline bool Glasses::is_tracking() 
{ 
    return is_connected() && _state.is_current(GlassesState::TRACKING); 
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

inline T5_GameboardType Glasses::get_gameboard_type() {
    return _swap_chain_frames[_current_frame_idx].glasses_pose.gameboardType;
}

inline GlassesFlags::FlagType Glasses::get_current_state() {
    return _state.get_current();
}

}