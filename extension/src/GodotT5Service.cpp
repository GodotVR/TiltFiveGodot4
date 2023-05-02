#include "GodotT5Service.h"
#include <assert.h>
#include <Logging.h>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/classes/texture_layered.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/defs.hpp> 

using godot::Variant;
using godot::Quaternion;
using godot::Image;
using godot::RenderingServer;
using godot::TypedArray;
using godot::Error;
using godot::XRServer;
using TextureLayeredType = godot::RenderingServer::TextureLayeredType;
using T5Integration::Glasses;
using T5Integration::WandButtons;
using T5Integration::log_message;


namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5ObjectRegistry g_godot_t5_services;



GodotT5Service::GodotT5Service()
	: T5Service()
{}


void GodotT5Service::reserve_glasses(GD::String glasses_id, GD::String display_name, RID viewport) {
    auto glasses_idx = find_glasses_idx(glasses_id.ascii().get_data());
    ERR_FAIL_COND_MSG(!glasses_idx, "Glasses id not found");

    get_glasses(glasses_idx.value())->set_viewport(viewport);

    T5Service::reserve_glasses(glasses_idx.value(), display_name.ascii().get_data());
}

void GodotT5Service::release_glasses(GD::String glasses_id) {
    auto glasses_idx = find_glasses_idx(glasses_id.ascii().get_data());
    ERR_FAIL_COND_MSG(!glasses_idx, "Glasses id not found");

    get_glasses(glasses_idx.value())->set_viewport(RID());

    T5Service::release_glasses(glasses_idx.value());
}

GodotT5Glasses::Ptr GodotT5Service::find_godot_glasses(int glasses_idx) {
    ERR_FAIL_INDEX_V(glasses_idx, _glasses_list.size(), GodotT5Glasses::Ptr());

    return get_glasses(glasses_idx);
}

GodotT5Glasses::Ptr GodotT5Service::find_godot_glasses(GD::String glasses_id) {
    auto glasses_idx = find_glasses_idx(glasses_id.ascii().get_data());
    ERR_FAIL_COND_V_MSG(!glasses_idx, GodotT5Glasses::Ptr(), "Glasses id not found");

    return get_glasses(glasses_idx.value());
}

GodotT5Glasses::Ptr GodotT5Service::find_glasses_by_render_target(RID test_render_target) {
	auto render_server = RenderingServer::get_singleton();

    for(size_t glasses_idx = 0; glasses_idx < get_glasses_count(); glasses_idx++) {
        auto godot_glasses = get_glasses(glasses_idx);
        auto glasses_render_target = render_server->viewport_get_render_target(godot_glasses->get_viewport());   
        if(test_render_target == glasses_render_target) {    
            return godot_glasses;
        }
    }

    return GodotT5Glasses::Ptr();
}

GodotT5Glasses::Ptr GodotT5Service::find_glasses_by_viewport(RID test_viewport) {
	auto render_server = RenderingServer::get_singleton();

    for(size_t glasses_idx = 0; glasses_idx < get_glasses_count(); glasses_idx++) {
        auto godot_glasses = get_glasses(glasses_idx);
        auto glasses_viewport = godot_glasses->get_viewport();   
        if(test_viewport == glasses_viewport) {    
            return godot_glasses;
        }
    }

    return GodotT5Glasses::Ptr();
}

std::unique_ptr<Glasses> GodotT5Service::create_glasses(const std::string_view id) {
    
    return std::unique_ptr<Glasses>(new GodotT5Glasses(id));
}


void GodotT5Math::rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z)  {

    godot::Quaternion orient(quat_x, quat_y, quat_z, quat_w);
    godot::Vector3 vec(vec_x, vec_y, vec_z);

	vec = orient.xform(vec);

	vec_x = vec.x;
	vec_y = vec.y;
	vec_z = vec.z;
}

void GodotT5Logger::log_error(const char* message, const char* func_name, const char* file_name, int line_num) { 
    godot::_err_print_error(func_name, file_name, line_num, godot::String(message), true, false);
}

void GodotT5Logger::log_warning(const char* message, const char* func_name, const char* file_name, int line_num) {
    godot::_err_print_error(func_name, file_name, line_num, godot::String(message), true, false);
}

void GodotT5Logger::log_string(const char* message) {
	std::cout << message << std::endl;
}


GodotT5Service::Ptr GodotT5ObjectRegistry::service() {

		assert(_instance);
		return std::static_pointer_cast<GodotT5Service>(ObjectRegistry::service());    
}


T5Integration::T5Service::Ptr GodotT5ObjectRegistry::get_service() {
    GodotT5Service::Ptr service;
	if (_service.expired()) {
		service = std::make_shared<GodotT5Service>();
        _service = service;
    }
    else {
        service = _service.lock();
    }
    return service;
}

T5Integration::T5Math::Ptr GodotT5ObjectRegistry::get_math() {
    GodotT5Math::Ptr math;
	if (_math.expired()) {
		math = std::make_shared<GodotT5Math>();
        _math = math;
    }
    else {
        math = _math.lock();
    }
	return math;
}

T5Integration::Logger::Ptr GodotT5ObjectRegistry::get_logger() {
		GodotT5Logger::Ptr logger;
		if (_logger.expired()) {
			logger = std::make_shared<GodotT5Logger>();
			_logger = logger;
		}
		else {
			logger = std::static_pointer_cast<GodotT5Logger>(_logger.lock());		
        }
		return logger;
}

}