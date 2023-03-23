#include "GodotT5Service.h"
#include <assert.h>
#include <Logging.h>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/classes/texture_layered.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/defs.hpp> 

using godot::Quaternion;
using godot::Image;
using godot::RenderingServer;
using godot::TypedArray;
using godot::Error;
using TextureLayeredType = godot::RenderingServer::TextureLayeredType;
using T5Integration::Glasses;
using T5Integration::WandButtons;

namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5ObjectRegistry g_godot_t5_services;

GodotT5Service::GodotT5Service()
	: T5Service()
{}


bool GodotT5Service::should_glasses_be_connected(Glasses::Ptr glasses) {
    return !_active_glasses;
}

void GodotT5Service::glasses_were_connected(Glasses::Ptr glasses) {
    if(!_active_glasses)
    {
        _active_glasses = glasses;
        create_textures(_active_glasses);
    }
}

void GodotT5Service::glasses_were_disconnected(Glasses::Ptr glasses) {
    if(_active_glasses == glasses)
    {
        _active_glasses.reset();
        release_textures(_active_glasses);
    }
}

bool GodotT5Service::is_connected()
{
	return _active_glasses && _active_glasses->is_connected();
}

bool GodotT5Service::is_tracking()
{
	return is_connected() && _active_glasses->is_tracking();
}

Vector2 GodotT5Service::get_display_size() {
	if (!_active_glasses) return Vector2(1216, 768);
    int width, height;
    _active_glasses->get_display_size(width, height);
    return Vector2(width, height);
}

float GodotT5Service::get_fov() {
	if (!_active_glasses) return 38.0f;
    return _active_glasses->get_fov();
}


Transform3D GodotT5Service::get_eye_offset(Glasses::Eye eye)
{
    assert(_active_glasses);
	float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
	auto ipd = _active_glasses->get_ipd();
    Transform3D eye_pose;
    eye_pose.set_origin(Vector3(dir * ipd / 2.0f, 0, 0));

	return eye_pose;
}

Transform3D GodotT5Service::get_head_transform() {
	if (!_active_glasses) return Transform3D();
	
	Quaternion orientation;
	Vector3 position;
	_active_glasses->get_glasses_orientation(orientation.x, orientation.y, orientation.z, orientation.w);
	_active_glasses->get_glasses_position(position.x, position.y, position.z);

	Transform3D headPose;
    headPose.set_origin(position);
	headPose.set_basis(orientation.inverse());
    headPose.rotate(Vector3(1,0,0), -Math_PI / 2.0f);

	return headPose;
}

Transform3D GodotT5Service::get_eye_transform(Glasses::Eye eye) 
{
	if (!_active_glasses) return Transform3D();

	return get_eye_offset(eye) * get_head_transform();
}

size_t GodotT5Service::get_num_wands() 
{
	return _active_glasses ? _active_glasses->get_num_wands() : 0;
}


bool GodotT5Service::is_wand_pose_valid(size_t wand_num) 
{
	return _active_glasses ? _active_glasses->is_wand_pose_valid(wand_num) : false;
}

Transform3D GodotT5Service::get_wand_transform(size_t wand_num) {
	Quaternion orientation;
	Vector3 position;
	_active_glasses->get_wand_position(wand_num, position.x, position.y, position.z);
	_active_glasses->get_wand_orientation(wand_num, orientation.x, orientation.y, orientation.z, orientation.w);

    position = Vector3(position.x, position.z, -position.y);
    orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
    orientation = orientation.inverse();

	Transform3D wandPose;
    wandPose.set_origin(position);
	wandPose.set_basis(orientation * Quaternion(Vector3(1,0,0), Math_PI / 2.0f));

	return wandPose;
}

void GodotT5Service::connection_updated()
{
}

void GodotT5Service::tracking_updated()
{
	if (!_active_glasses) return;

    auto num_wands = _active_glasses->get_num_wands();
    while(_wand_controller_id.size() < num_wands) {
        auto new_idx = _wand_controller_id.size();
        _wand_controller_id.push_back(-1);
        _wand_name.push_back(_active_glasses->get_id() + ":" + std::to_string(new_idx) + '\0');
    }

    for(int wand_idx = 0; wand_idx < num_wands; ++wand_idx) {
        update_wand(wand_idx);
    }
}

void GodotT5Service::update_wand(size_t wand_idx) {
    /*
    auto controller_id = _wand_controller_id[wand_idx];
    int hand = wand_idx < 2 ? wand_idx + 1 : 0;
    if(_active_glasses->is_wand_state_changed(wand_idx, WandState::CONNECTED)) {
        if(_active_glasses->is_wand_state_set(wand_idx, WandState::CONNECTED)) {
            controller_id = godot::arvr_api->godot_arvr_add_controller(&(_wand_name[wand_idx][0]), hand, true, true);
            _wand_controller_id[wand_idx] = controller_id;
        }
        else if(controller_id > 0) {
            godot::arvr_api->godot_arvr_remove_controller(controller_id);
            return;
        }
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::POSE_VALID)) {
        auto wand_transform = as_c_struct(get_wand_transform(wand_idx));
        godot::arvr_api->godot_arvr_set_controller_transform(controller_id, &wand_transform, true, true);
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::ANALOG_VALID)) {
        float trigger_value;
        _active_glasses->get_wand_trigger(wand_idx, trigger_value);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_TRIGGER, trigger_value, true);
        Vector2 stick;
        _active_glasses->get_wand_stick(wand_idx, stick.x, stick.y);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_X, stick.x, true);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_Y, stick.y, true);
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::BUTTONS_VALID)) {
        WandButtons buttons;
        _active_glasses->get_wand_buttons(wand_idx, buttons);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_A,     buttons.a);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_B,	    buttons.b);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_X,	    buttons.x);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_Y,	    buttons.y);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_1,	    buttons.one);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_2,	    buttons.two);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_STICK, buttons.three);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_T5,	buttons.t5);
    }
    */
}

void GodotT5Service::send_frame()
{
	if (!_active_glasses) return;

    auto rendering_server = RenderingServer::get_singleton();

    rendering_server->texture_copy(_render_texture, 0, 0, _left_eye_texture, 0, 0);
    rendering_server->texture_copy(_render_texture, 0, 1, _right_eye_texture, 0, 0);

	_active_glasses->send_frame(
        rendering_server->texture_get_native_handle(_left_eye_texture), 
        rendering_server->texture_get_native_handle(_right_eye_texture));
}

RID GodotT5Service::get_color_texture() {
    //return RID();
    return _render_texture;
}

void GodotT5Service::create_textures(Glasses::Ptr glasses) {

    int width;
    int height;
    glasses->get_display_size(width, height);

    //godot::Color bg(0,0,0,1);

    //Ref<Image> image = Image::create(width, height, false, Image::FORMAT_RGBA8);

    //TypedArray<Image> image_layers;
    //image_layers.append(image);
    //image_layers.append(image);
    
    auto render_server = RenderingServer::get_singleton();

    _render_texture = render_server->texture_create_render_texture(width, height, 2);
    //_render_texture = render_server->texture_3d_create(Image::Format::FORMAT_RGBA8, width, height, 2, false, image_layers);
    //_render_texture = render_server->texture_2d_layered_create(image_layers, TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY);
    _left_eye_texture = render_server->texture_create_render_texture(width, height, 1);
    _right_eye_texture = render_server->texture_create_render_texture(width, height, 1);
}

void GodotT5Service::release_textures(Glasses::Ptr glasses) {
    
    auto rendering_server = RenderingServer::get_singleton();
    
    rendering_server->free_rid(_right_eye_texture);
    rendering_server->free_rid(_left_eye_texture);
    rendering_server->free_rid(_render_texture);
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