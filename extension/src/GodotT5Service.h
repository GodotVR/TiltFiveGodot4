#pragma once
#include <ObjectRegistry.h>
#include <Glasses.h>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/classes/ref.hpp>   
#include <godot_cpp/classes/global_constants.hpp> 

namespace GodotT5Integration {

using godot::Transform3D;
using godot::Vector2;
using godot::Vector3;
using godot::Ref;
using godot::RID;

using T5Integration::Glasses;

/*
enum ControllerMap {
	// buttons
	WAND_BUTTON_A		= GlobalConstants::JOY_BUTTON_0,
	WAND_BUTTON_B		= GlobalConstants::JOY_BUTTON_1,
	WAND_BUTTON_X		= GlobalConstants::JOY_BUTTON_2,
	WAND_BUTTON_Y		= GlobalConstants::JOY_BUTTON_3,
	WAND_BUTTON_1		= GlobalConstants::JOY_BUTTON_4,
	WAND_BUTTON_2		= GlobalConstants::JOY_BUTTON_5,
	WAND_BUTTON_STICK	= GlobalConstants::JOY_BUTTON_6,
	WAND_BUTTON_T5		= GlobalConstants::JOY_BUTTON_7,

	// AXIS
	WAND_ANALOG_X = GlobalConstants::JOY_AXIS_0,
	WAND_ANALOG_Y = GlobalConstants::JOY_AXIS_1,
	WAND_ANALOG_TRIGGER = GlobalConstants::JOY_AXIS_2,
};
*/
class GodotT5Service : public T5Integration::T5Service {

protected:

	bool should_glasses_be_connected(Glasses::Ptr glasses) override;
	void glasses_were_connected(Glasses::Ptr glasses) override;
	void glasses_were_disconnected(Glasses::Ptr glasses) override;

	virtual void connection_updated() override;
	virtual void tracking_updated() override;

    void update_wand(size_t wand_idx);

public:
	using Ptr = std::shared_ptr<GodotT5Service>;

	GodotT5Service();

	bool is_connected();
	bool is_tracking();

    Vector2 get_display_size();
    float get_fov();
	Transform3D get_head_transform();
	Transform3D get_eye_offset(Glasses::Eye eye);
	Transform3D get_eye_transform(Glasses::Eye eye);

	size_t get_num_wands();
	bool is_wand_pose_valid(size_t wand_num);
	Transform3D get_wand_transform(size_t wand_num);

	void send_frame();

    RID get_color_texture();

private:

	Glasses::Ptr _active_glasses;

    void create_textures(Glasses::Ptr glasses);
    void release_textures(Glasses::Ptr glasses);

	RID _render_texture;
	RID _left_eye_texture;
    RID _right_eye_texture;

    std::vector<uint32_t> _wand_controller_id;
    std::vector<std::string> _wand_name;
};

class GodotT5Math : public T5Integration::T5Math {
	public:
	using Ptr = std::shared_ptr<GodotT5Math>;
	void rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z) override;
};

class GodotT5Logger : public T5Integration::Logger {
public:
	using Ptr = std::shared_ptr<GodotT5Logger>;
	void log_error(const char* message, const char* func_name, const char* file_name, int line_num) override;
	void log_warning(const char* message, const char* func_name, const char* file_name, int line_num) override;
	void log_string(const char* message) override;
};


class GodotT5ObjectRegistry : public T5Integration::ObjectRegistry {
	public:
	GodotT5ObjectRegistry() = default;
	~GodotT5ObjectRegistry() = default;

	static GodotT5Service::Ptr service();

	T5Integration::T5Service::Ptr get_service() override;
	T5Integration::T5Math::Ptr get_math() override;
	T5Integration::Logger::Ptr get_logger() override;

	protected:
	GodotT5Service::Ptr::weak_type _service;
	GodotT5Math::Ptr::weak_type _math;
};


}
