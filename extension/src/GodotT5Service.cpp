#include <GodotT5Service.h>
#include <Logging.h>
#include <OpenGLGlasses.h>
#include <VulkanGlasses.h>
#include <assert.h>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/texture_layered.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

using godot::Error;
using godot::Image;
using godot::Quaternion;
using godot::RenderingDevice;
using godot::RenderingServer;
using godot::TypedArray;
using godot::UtilityFunctions;
using godot::Variant;
using godot::XRServer;
using TextureLayeredType = godot::RenderingServer::TextureLayeredType;
using T5Integration::Glasses;
using T5Integration::log_message;
using T5Integration::WandButtons;

namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5ObjectRegistry g_godot_t5_services;

GodotT5Service::GodotT5Service() :
		T5Service() {}

std::unique_ptr<Glasses> GodotT5Service::create_glasses(const std::string_view id) {
	if (get_graphics_api() == kT5_GraphicsApi_GL)
		return std::unique_ptr<Glasses>(new OpenGLGlasses(id));
	else if (get_graphics_api() == kT5_GraphicsApi_Vulkan)
		return std::unique_ptr<Glasses>(new VulkanGlasses(id));

	ERR_FAIL_V_MSG(std::unique_ptr<Glasses>(), "Unknown graphics API");
}

void GodotT5Service::use_opengl_api() {
	T5_GraphicsContextGL graphics_context;
	graphics_context.textureMode = T5_GraphicsApi_GL_TextureMode::kT5_GraphicsApi_GL_TextureMode_Array;
	graphics_context.leftEyeArrayIndex = 0;
	graphics_context.rightEyeArrayIndex = 1;
	set_graphics_context(graphics_context);
}

void GodotT5Service::use_vulkan_api() {
	RenderingServer* rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice* rendering_device = rendering_server->get_rendering_device();
	ERR_FAIL_NULL(rendering_device);

	T5_GraphicsContextVulkan graphics_context;
	graphics_context.instance = (void*)rendering_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_INSTANCE, RID(), 0);
	graphics_context.physicalDevice = (void*)rendering_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_PHYSICAL_DEVICE, RID(), 0);
	graphics_context.device = (void*)rendering_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_DEVICE, RID(), 0);
	graphics_context.queue = (void*)rendering_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_QUEUE, RID(), 0);
	graphics_context.queueFamilyIndex = (uint32_t)rendering_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_QUEUE_FAMILY_INDEX, RID(), 0);
	graphics_context.textureMode = kT5_GraphicsApi_Vulkan_TextureMode_ImageView;
	set_graphics_context(graphics_context);
}

bool GodotT5Service::get_tracker_association(StringName tracker_name, int& out_glasses_idx, int& out_wand_idx) {
	for (out_glasses_idx = 0; out_glasses_idx < _glasses_list.size(); ++out_glasses_idx) {
		auto godot_glasses = std::static_pointer_cast<GodotT5Glasses>(_glasses_list[out_glasses_idx]);
		if (godot_glasses->get_tracker_association(tracker_name, out_wand_idx)) {
			return true;
		}
	}
	out_glasses_idx = -1;
	out_wand_idx = -1;
	return false;
}

void GodotT5Math::rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z, bool inverse) {
	godot::Quaternion orient(quat_x, quat_y, quat_z, quat_w);
	godot::Vector3 vec(vec_x, vec_y, vec_z);

	if (inverse) {
		orient = orient.inverse();
	}

	vec = orient.xform(vec);

	vec_x = vec.x;
	vec_y = vec.y;
	vec_z = vec.z;
}

void GodotT5Logger::log_error(const char* message, const char* func_name, const char* file_name, int line_num) {
	godot::_err_print_error(func_name, file_name, line_num, "TiltFiveXRInterface", message, true, false);
}

void GodotT5Logger::log_warning(const char* message, const char* func_name, const char* file_name, int line_num) {
	godot::_err_print_error(func_name, file_name, line_num, "TiltFiveXRInterface", message, true, false);
}

void GodotT5Logger::log_string(const char* message) {
	Variant v_msg = message;
	UtilityFunctions::print_verbose(v_msg);
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
	} else {
		service = _service.lock();
	}
	return service;
}

T5Integration::T5Math::Ptr GodotT5ObjectRegistry::get_math() {
	GodotT5Math::Ptr math;
	if (_math.expired()) {
		math = std::make_shared<GodotT5Math>();
		_math = math;
	} else {
		math = _math.lock();
	}
	return math;
}

T5Integration::Logger::Ptr GodotT5ObjectRegistry::get_logger() {
	GodotT5Logger::Ptr logger;
	if (_logger.expired()) {
		logger = std::make_shared<GodotT5Logger>();
		_logger = logger;
	} else {
		logger = std::static_pointer_cast<GodotT5Logger>(_logger.lock());
	}
	return logger;
}

} //namespace GodotT5Integration