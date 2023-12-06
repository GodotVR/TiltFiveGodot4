#include <GodotT5Glasses.h>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp> 
#include <Wand.h>
#include <ObjectRegistry.h>

using godot::Vector3;
using godot::Quaternion;
using godot::Projection;
using godot::Variant;
using godot::XRServer;
using T5Integration::WandButtons;

namespace GlassesState = T5Integration::GlassesState;
namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5Glasses::GodotT5Glasses(std::string_view id) 
: Glasses(id) {
	set_swap_chain_size(g_swap_chain_length);
}


Transform3D GodotT5Glasses::get_head_transform(Vector3 eye_offset) {
	Quaternion orientation;
	Vector3 position;

	get_glasses_orientation(orientation.x, orientation.y, orientation.z, orientation.w);
	get_glasses_position(position.x, position.y, position.z);

	// Tiltfive -> Godot axis
	position = Vector3(position.x, position.z, -position.y);
	orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
	orientation = orientation.inverse();

	Transform3D headPose;
	headPose.set_origin(position);
	headPose.set_basis(orientation);

	Transform3D axisAdjust;
	axisAdjust.rotate(Vector3(1.0, 0.0, 0.0), -Math_PI / 2.0f);

	Transform3D eye_pose;
	eye_pose.set_origin(eye_offset);

	return headPose * axisAdjust * eye_pose;
}

Vector3 GodotT5Glasses::get_eye_offset(Glasses::Eye eye) {
	float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
	auto ipd = get_ipd();
	return Vector3(dir * ipd / 2.0f, 0, 0);
}

Transform3D GodotT5Glasses::get_eye_transform(Glasses::Eye eye) {
	return get_head_transform(get_eye_offset(eye));
}

Transform3D GodotT5Glasses::get_wand_transform(int wand_num) {
	Quaternion orientation;
	Vector3 position;
	get_wand_position(wand_num, position.x, position.y, position.z);
	get_wand_orientation(wand_num, orientation.x, orientation.y, orientation.z, orientation.w);

	// Tiltfive -> Godot axis
	position = Vector3(position.x, position.z, -position.y);
	orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
	orientation = orientation.inverse();

	Transform3D wandPose;
	wandPose.set_origin(position);
	wandPose.set_basis(orientation * Quaternion(Vector3(1,0,0), Math_PI / 2.0f));

	return wandPose;
}

PackedFloat64Array GodotT5Glasses::get_projection_for_eye(Glasses::Eye view, double aspect, double z_near, double z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix
	arr.fill(0);

	Projection cm;
	cm.set_perspective(get_fov(), aspect, z_near, z_far);

	real_t *m = (real_t *)cm.columns;
	for (int i = 0; i < 16; i++) {
		arr[i] = m[i];
	}

	return arr;    
}

void GodotT5Glasses::on_glasses_reserved() {
	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

    char buffer[64];
    ERR_FAIL_COND(snprintf(buffer, 64, "/user/%s/head", get_id().c_str()) > 64);

	_head.instantiate();
	_head->set_tracker_type(XRServer::TRACKER_HEAD);
	_head->set_tracker_name(buffer);
	_head->set_tracker_desc("Players head");
	xr_server->add_tracker(_head);    
}

void GodotT5Glasses::on_glasses_released() {
	if(_head.is_valid()) {
		XRServer *xr_server = XRServer::get_singleton();
		ERR_FAIL_NULL(xr_server);

		xr_server->remove_tracker(_head);
	}
}

void GodotT5Glasses::on_glasses_dropped() {
	if(_head.is_valid()) {
		XRServer *xr_server = XRServer::get_singleton();
		ERR_FAIL_NULL(xr_server);

		xr_server->remove_tracker(_head);
	}
}

void GodotT5Glasses::on_tracking_updated() {
	if(_head.is_valid()) {
		if (is_tracking()) {
			_head->set_pose(
				"default", 
				get_head_transform(), 
				Vector3(), 
				Vector3(), 
				godot::XRPose::XR_TRACKING_CONFIDENCE_HIGH);
		} else {
			_head->invalidate_pose("default");
		}
	}

	auto num_wands = get_num_wands();
	for(int wand_idx = 0; wand_idx < num_wands; ++wand_idx) {
		if(wand_idx == _wand_trackers.size())
			add_tracker();
		update_wand(wand_idx);
	}   
}

bool GodotT5Glasses::is_in_use() {
	auto current_state = get_current_state();
	return (current_state & GlassesState::SUSTAIN_CONNECTION) || (current_state & GlassesState::UNAVAILABLE);
}

Vector2 GodotT5Glasses::get_render_size() {
	int width;
	int height;
	Glasses::get_display_size(width, height);

	return Vector2(width, height);
}

void GodotT5Glasses::add_tracker() {
	int new_idx = _wand_trackers.size();
	int new_id = new_idx + 1;

	Ref<XRPositionalTracker> positional_tracker;
	positional_tracker.instantiate();

    char buffer[64];
    ERR_FAIL_COND(snprintf(buffer, 64, "/user/%s/wand_%d", get_id().c_str(), new_id) > 64);
 
    positional_tracker->set_tracker_type(XRServer::TRACKER_CONTROLLER);
    positional_tracker->set_tracker_name(buffer);
    positional_tracker->set_tracker_desc("Tracks wand");

	_wand_trackers.push_back(positional_tracker);    
}

void GodotT5Glasses::update_wand(int wand_idx) {

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

		if(trigger_value > _trigger_click_threshold + g_trigger_hysteresis_range) {
			tracker->set_input("trigger_click", Variant(true));
		}
		else if(trigger_value < (_trigger_click_threshold - g_trigger_hysteresis_range)) {
			tracker->set_input("trigger_click", Variant(false));
		}
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

bool GodotT5Glasses::get_tracker_association(StringName tracker_name, int& out_wand_idx) {
    for(out_wand_idx = 0; out_wand_idx < _wand_trackers.size(); ++out_wand_idx) {
        auto b1 = tracker_name.to_utf8_buffer();
        auto b2 = _wand_trackers[out_wand_idx]->get_tracker_name().to_utf8_buffer();

        if(tracker_name == _wand_trackers[out_wand_idx]->get_tracker_name())
            return true;
    }
    out_wand_idx = -1;
    return false;
}


} // GodotT5Integration