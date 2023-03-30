#include "TiltFiveXRInterface.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;
using GodotT5Integration::GodotT5ObjectRegistry;
using T5Integration::GlassesEvent;
using Eye = GodotT5Integration::Glasses::Eye;

void TiltFiveXRInterface::_bind_methods() {
	// Methods.
	// ClassDB::bind_method(D_METHOD("simple_func"), &Example::simple_func);

	// Properties.

	ClassDB::bind_method(D_METHOD("start_service", "application_id", "application_version"), &TiltFiveXRInterface::start_service);
	ClassDB::bind_method(D_METHOD("reserve_glasses", "glasses_id", "display_name"), &TiltFiveXRInterface::reserve_glasses);
	ClassDB::bind_method(D_METHOD("release_glasses", "glasses_id"), &TiltFiveXRInterface::release_glasses);


	// Signals.
	ADD_SIGNAL(MethodInfo("glasses_event", PropertyInfo(Variant::STRING, "glasses_id"), PropertyInfo(Variant::INT, "event")));
	// ClassDB::bind_method(D_METHOD("emit_custom_signal", "name", "value"), &Example::emit_custom_signal);

	// Constants.
	// BIND_ENUM_CONSTANT(FIRST);
}


bool TiltFiveXRInterface::setup() {
	xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V_MSG(xr_server, false, "XRServer unavailable");

	t5_service = GodotT5ObjectRegistry::service();
	ERR_FAIL_COND_V_MSG(!t5_service, false, "Couldn't obtain GodotT5Service singleton");

	xr_server->add_interface(this);
	return true;
}

void TiltFiveXRInterface::teardown() {
	
	if(xr_server)
		xr_server->remove_interface(this);

	t5_service.reset();
}

bool TiltFiveXRInterface::start_service(const String application_id, const String application_version)
{
    if(!setup()) return false;
	
    bool is_started = t5_service->start_service(application_id.ascii().get_data(), application_version.ascii().get_data());
	if(is_started)
		initialize();
	return is_started;
}

void TiltFiveXRInterface::reserve_glasses(const String glasses_id, const String display_name) 
{
    if(!t5_service) return;

    int glasses_idx;
    if(try_find_glasses_idx(glasses_id, glasses_idx)) {
        t5_service->connect_glasses(glasses_idx, display_name.ascii().get_data());
    }
    else {
        auto msg = "Unknown glasses id " + glasses_id;
        LOG_ERROR(msg.ascii().get_data());
    }
}

void TiltFiveXRInterface::release_glasses(const String glasses_id)
{
    if(!t5_service) return;

    int glasses_idx;
    if(try_find_glasses_idx(glasses_id, glasses_idx)) {
        t5_service->disconnect_glasses(glasses_idx);
    }
    else {
        auto msg = "Unknown glasses id " + glasses_id;
        LOG_ERROR(msg.ascii().get_data());
    }
}

void TiltFiveXRInterface::add_glasses(int glasses_idx) {
    if(glasses_ids.size() <= glasses_idx)
        glasses_ids.resize(glasses_idx + 1);
    glasses_ids[glasses_idx] = t5_service->get_glasses_id(glasses_idx);
}

bool TiltFiveXRInterface::try_find_glasses_idx(const String& glasses_id, int& out_glasses_idx) {
    for(out_glasses_idx = 0; out_glasses_idx < glasses_ids.size(); ++out_glasses_idx)
    {
        if(glasses_id == glasses_ids[out_glasses_idx].c_str())
            return true;
    }
    return false;
}

StringName TiltFiveXRInterface::_get_name() const {
	// this currently fails to return because we loose our data before it ends up in the callers hands...
	StringName name("TiltFive");
	return name;
}

uint32_t TiltFiveXRInterface::_get_capabilities() const {
	return XR_STEREO | XR_AR;
}

bool TiltFiveXRInterface::_is_initialized() const {
	return initialised;
}

bool TiltFiveXRInterface::_initialize() {
	if (!initialised) {
		initialised = ((bool)t5_service) && xr_server;
	}
	return initialised;
}

void TiltFiveXRInterface::_uninitialize() {
	if (initialised) {

		initialised = false;
		xr_server = nullptr;
        t5_service.reset();
	}
}

bool TiltFiveXRInterface::_supports_play_area_mode(XRInterface::PlayAreaMode mode) const {
    return mode == XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::PlayAreaMode TiltFiveXRInterface::_get_play_area_mode() const {
    return XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::TrackingStatus TiltFiveXRInterface::_get_tracking_status() const {
	return t5_service->is_tracking() ? XRInterface::TrackingStatus::XR_NORMAL_TRACKING : XRInterface::TrackingStatus::XR_NOT_TRACKING;
}

Vector2 TiltFiveXRInterface::_get_render_target_size() {
    return t5_service->get_display_size();
}

uint32_t TiltFiveXRInterface::_get_view_count() {
	return 2; // stereo
}

Transform3D TiltFiveXRInterface::_get_camera_transform() {
	if (!initialised) {
		return Transform3D();
	}

	auto hmd_transform = t5_service->get_head_transform();

	return xr_server->get_reference_frame() * hmd_transform;
}

Transform3D TiltFiveXRInterface::_get_transform_for_view(uint32_t view, const Transform3D &origin_transform) {
	if (!initialised) {
		return Transform3D();
	}
	auto world_scale = xr_server->get_world_scale();

	auto eye_transform = t5_service->get_eye_transform(view == 0 ? Eye::Left : Eye::Right);
	eye_transform.scale(Vector3(world_scale, world_scale, world_scale));

	return eye_transform * origin_transform;
}

PackedFloat64Array TiltFiveXRInterface::_get_projection_for_view(uint32_t p_view, double aspect, double z_near, double z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix

    Projection cm;
    cm.set_perspective(t5_service->get_fov(), aspect, z_near, z_far);
	//auto offset = t5_service->get_eye_offset(p_view == 0 ? Eye::Left : Eye::Right);

	//cm = cm * offset;

    real_t *m = (real_t *)cm.columns;
	for (int i = 0; i < 16; i++) {
		arr[i] = m[i];
	}

    return arr;
}

void TiltFiveXRInterface::_post_draw_viewport(const RID &render_target, const Rect2 &screen_rect) {
	Rect2 src_rect(0.0f, 0.0f, 1.0f, 1.0f);
	Rect2 dst_rect = screen_rect;

	// halve our width
	Vector2 size = dst_rect.get_size();
	size.x = size.x * 0.5;
	dst_rect.size = size;

	Vector2 eye_center(0.0, 0.0);

	add_blit(render_target, src_rect, screen_rect, true, 0, false, eye_center, 0, 0, 0, 0);

}

void TiltFiveXRInterface::_end_frame() {
	t5_service->send_frame();
}

PackedStringArray TiltFiveXRInterface::_get_suggested_tracker_names() const {
	PackedStringArray tracker_names;
	
	tracker_names.append("tilt_five_wand_1");
	tracker_names.append("tilt_five_wand_2");
	tracker_names.append("tilt_five_wand_3");
	tracker_names.append("tilt_five_wand_4");

	return tracker_names;
}

PackedStringArray TiltFiveXRInterface::_get_suggested_pose_names(const StringName &tracker_name) const {
	PackedStringArray tracker_names;
	
	tracker_names.append("aim");
	tracker_names.append("grip");
	tracker_names.append("finger");

	return tracker_names;
}

void TiltFiveXRInterface::_process() {

    if(!t5_service) return;

    t5_service->update_connection();
    t5_service->update_tracking();

    auto& events = t5_service->get_events();
    for(int i = 0; i < events.size(); i++) 
    {
        if(events[i].event == GlassesEvent::E_CONNECTED) {
            ++reserved_glasses_count;
			if(reserved_glasses_count > 0 && !is_primary()) {
				set_primary(true);
			}
		}
        else if(events[i].event == GlassesEvent::E_DISCONNECTED) {
            --reserved_glasses_count;
			if(reserved_glasses_count == 0 && is_primary()) {
				set_primary(false);
    		}
		}
        else if(events[i].event == GlassesEvent::E_ADDED)
            add_glasses(events[i].glasses_num);
        emit_signal("glasses_event", glasses_ids[events[i].glasses_num].c_str(), (int)events[i].event);
    }
}

RID TiltFiveXRInterface::_get_color_texture() {
	RID color_texture;
	if(t5_service)
		color_texture = t5_service->get_color_texture();
	return color_texture;		
}

	
bool TiltFiveXRInterface::_get_anchor_detection_is_enabled() const {
	return false;
}

void TiltFiveXRInterface::_set_anchor_detection_is_enabled(bool enabled) {

}

int32_t TiltFiveXRInterface::_get_camera_feed_id() const {
	return 0;
}

TiltFiveXRInterface::TiltFiveXRInterface() {
}

TiltFiveXRInterface::~TiltFiveXRInterface() {

}