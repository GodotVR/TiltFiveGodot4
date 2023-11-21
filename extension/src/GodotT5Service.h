#pragma once
#include <GodotT5Glasses.h>
#include <ObjectRegistry.h>
#include <Glasses.h>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/classes/ref.hpp>   
#include <godot_cpp/classes/xr_positional_tracker.hpp>  
#include <godot_cpp/classes/global_constants.hpp> 

namespace GodotT5Integration {

namespace GD = godot;

using GD::Transform3D;
using GD::Vector2;
using GD::Vector3;
using GD::Ref;
using GD::RID;
using GD::StringName;
using GD::XRPositionalTracker;

using T5Integration::Glasses;


class GodotT5Service : public T5Integration::T5Service {

protected:

	std::unique_ptr<Glasses> create_glasses(const std::string_view id) override;
	

public:
	using Ptr = std::shared_ptr<GodotT5Service>;

	GodotT5Service();

	GodotT5Glasses::Ptr get_glasses(int glasses_idx) {
		// Safe to down cast 
		return std::static_pointer_cast<GodotT5Glasses>(_glasses_list[glasses_idx]);
	}

	void use_opengl_api();
	void use_vulkan_api();



};

class GodotT5Math : public T5Integration::T5Math {
	public:
	using Ptr = std::shared_ptr<GodotT5Math>;
	void rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z, bool inverse = false) override;
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
