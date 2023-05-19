#include <GodotT5Glasses.h>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp> 
#include <Wand.h>

using godot::RenderingServer;
using godot::Vector3;
using godot::Quaternion;
using godot::Variant;
using godot::XRServer;
using godot::Image;
using godot::TypedArray;
using godot::ImageTexture;
using T5Integration::WandButtons;

namespace GlassesState = T5Integration::GlassesState;
namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5Glasses::GodotT5Glasses(std::string_view id) 
: Glasses(id) {
    set_swap_chain_size(g_swap_chain_length);
    _swap_chain_textures.resize(g_swap_chain_length);
}

void GodotT5Glasses::SwapChainTextures::allocate_textures(int width, int height) {
    auto render_server = RenderingServer::get_singleton();

    deallocate_textures();

    Ref<Image> dummy_image = Image::create(width, height, false, godot::Image::FORMAT_RGBA8);
    godot::Color bg(0,0,0);
    dummy_image->fill(bg);

    left_eye_tex = ImageTexture::create_from_image(dummy_image);
    right_eye_tex = ImageTexture::create_from_image(dummy_image);

    TypedArray<Image> image_arr;
    image_arr.append(dummy_image);
    image_arr.append(dummy_image);

    render_tex.instantiate();
    render_tex->create_from_images(image_arr);
}

void GodotT5Glasses::SwapChainTextures::deallocate_textures() {
    auto render_server = RenderingServer::get_singleton();
    
    render_tex.unref();
    right_eye_tex.unref();
    left_eye_tex.unref();
}

void GodotT5Glasses::allocate_textures() {
    auto render_server = RenderingServer::get_singleton();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].allocate_textures(width, height);
        set_swap_chain_texture_handles(
            i,
            render_server->texture_get_native_handle(_swap_chain_textures[i].left_eye_tex->get_rid()),
            render_server->texture_get_native_handle(_swap_chain_textures[i].right_eye_tex->get_rid()));
    }
}

void GodotT5Glasses::deallocate_textures() {
    auto render_server = RenderingServer::get_singleton();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].deallocate_textures();
        set_swap_chain_texture_handles(i, 0, 0);
    }
}

void GodotT5Glasses::on_glasses_reserved() {
    allocate_textures();
}

void GodotT5Glasses::on_glasses_released() {
    deallocate_textures();
}

void GodotT5Glasses::on_glasses_dropped() {
    deallocate_textures();
}

void GodotT5Glasses::on_tracking_updated() {
    auto num_wands = get_num_wands();
    for(int wand_idx = 0; wand_idx < num_wands; ++wand_idx) {
        if(wand_idx == _wand_trackers.size())
            add_tracker();
        update_wand(wand_idx);
    }   
}

void GodotT5Glasses::on_send_frame(int swap_chain_idx) {
    auto rendering_server = RenderingServer::get_singleton();
    auto& textures = _swap_chain_textures[swap_chain_idx];

    auto rentex = textures.render_tex->get_rid();

    rendering_server->texture_copy(rentex, 0, 0, textures.left_eye_tex->get_rid(), 0, 0);
    rendering_server->texture_copy(rentex, 0, 1, textures.right_eye_tex->get_rid(), 0, 0);
}

bool GodotT5Glasses::is_in_use() {
    auto current_state = get_current_state();
    return (current_state & GlassesState::SUSTAIN_CONNECTION) || (current_state & GlassesState::UNAVAILABLE);
}

Vector2 GodotT5Glasses::get_display_size() {
    int width;
    int height;
    Glasses::get_display_size(width, height);

    return Vector2(width, height);
}

Transform3D GodotT5Glasses::get_head_transform() {
	Quaternion orientation;
	Vector3 position;
	get_glasses_orientation(orientation.x, orientation.y, orientation.z, orientation.w);
	get_glasses_position(position.x, position.y, position.z);

	Transform3D headPose;
    headPose.set_origin(position);
	headPose.set_basis(orientation.inverse());
    headPose.rotate(Vector3(1,0,0), -Math_PI / 2.0f);

	return headPose;
}

Transform3D GodotT5Glasses::get_eye_offset(Glasses::Eye eye) {
	float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
	auto ipd = get_ipd();
    Transform3D eye_pose;
    eye_pose.set_origin(Vector3(dir * ipd / 2.0f, 0, 0));

	return eye_pose;
}

Transform3D GodotT5Glasses::get_eye_transform(Glasses::Eye eye) {
	return get_eye_offset(eye) * get_head_transform();
}

Transform3D GodotT5Glasses::get_wand_transform(size_t wand_num) {
	Quaternion orientation;
	Vector3 position;
	get_wand_position(wand_num, position.x, position.y, position.z);
	get_wand_orientation(wand_num, orientation.x, orientation.y, orientation.z, orientation.w);

    position = Vector3(position.x, position.z, -position.y);
    orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
    orientation = orientation.inverse();

	Transform3D wandPose;
    wandPose.set_origin(position);
	wandPose.set_basis(orientation * Quaternion(Vector3(1,0,0), Math_PI / 2.0f));

	return wandPose;
}
 
RID GodotT5Glasses::get_color_texture() 
{ 
    auto rendering_server = RenderingServer::get_singleton();

    int current_frame = get_current_frame_idx();
    return _swap_chain_textures[current_frame].render_tex->get_rid();
}

void GodotT5Glasses::add_tracker() {
    int new_idx = _wand_trackers.size();
    int new_id = new_idx + 1;

    Ref<XRPositionalTracker> positional_tracker;
    positional_tracker.instantiate();

    auto tracker_name = std::format("{}/tilt_five_wand_{}", get_id(), new_id);
 
    positional_tracker->set_tracker_type(XRServer::TRACKER_CONTROLLER);
    positional_tracker->set_tracker_name(tracker_name.c_str());
    positional_tracker->set_tracker_desc(tracker_name.c_str());

    _wand_trackers.push_back(positional_tracker);    
}

void GodotT5Glasses::update_wand(size_t wand_idx) {

    auto xr_server = XRServer::get_singleton();

    auto tracker = _wand_trackers[wand_idx];

    if(is_wand_state_changed(wand_idx, WandState::CONNECTED)) {
        if(is_wand_state_set(wand_idx, WandState::CONNECTED)) {
            xr_server->add_tracker(tracker);
        } else  {
            xr_server->remove_tracker(tracker);
            return;
        }
    }
    if(is_wand_state_set(wand_idx, WandState::POSE_VALID)) {
        auto wand_transform = get_wand_transform(wand_idx);
        tracker->set_pose("default", wand_transform, Vector3(), Vector3(), godot::XRPose::XR_TRACKING_CONFIDENCE_HIGH);
    } else  {
        tracker->invalidate_pose("default");
    }
    if(is_wand_state_set(wand_idx, WandState::ANALOG_VALID)) {
        float trigger_value;
        get_wand_trigger(wand_idx, trigger_value);
        tracker->set_input("trigger", Variant(trigger_value));
        Vector2 stick;
        get_wand_stick(wand_idx, stick.x, stick.y);
        tracker->set_input("stick", Variant(stick));
    }
    if(is_wand_state_set(wand_idx, WandState::BUTTONS_VALID)) {
        WandButtons buttons;
        get_wand_buttons(wand_idx, buttons);

        tracker->set_input("button_a", Variant(buttons.a));
        tracker->set_input("button_b", Variant(buttons.b));
        tracker->set_input("button_x", Variant(buttons.x));
        tracker->set_input("button_y", Variant(buttons.y));
        tracker->set_input("button_1", Variant(buttons.one));
        tracker->set_input("button_2", Variant(buttons.two));
        tracker->set_input("button_3", Variant(buttons.three));
        tracker->set_input("button_t5", Variant(buttons.t5)); 
    }
}
} // GodotT5Integration