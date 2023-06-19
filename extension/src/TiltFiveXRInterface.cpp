#include "TiltFiveXRInterface.h"
#include "TiltFiveGameboard.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;
using GodotT5Integration::GodotT5ObjectRegistry;
using T5Integration::GlassesEvent;
using Eye = GodotT5Integration::Glasses::Eye;

void TiltFiveXRInterface::_bind_methods() {
	// Methods.

	ClassDB::bind_method(D_METHOD("start_service", "application_id", "application_version"), &TiltFiveXRInterface::start_service);
	ClassDB::bind_method(D_METHOD("stop_service"), &TiltFiveXRInterface::stop_service);
	ClassDB::bind_method(D_METHOD("reserve_glasses", "glasses_id", "display_name"), &TiltFiveXRInterface::reserve_glasses);
	ClassDB::bind_method(D_METHOD("start_display", "glasses_id", "viewport", "xr_origin"), &TiltFiveXRInterface::start_display);
	ClassDB::bind_method(D_METHOD("stop_display", "glasses_id"), &TiltFiveXRInterface::stop_display);
	ClassDB::bind_method(D_METHOD("release_glasses", "glasses_id"), &TiltFiveXRInterface::release_glasses);
	ClassDB::bind_method(D_METHOD("get_available_glasses_ids"), &TiltFiveXRInterface::get_available_glasses_ids);
	ClassDB::bind_method(D_METHOD("get_reserved_glasses_ids"), &TiltFiveXRInterface::get_reserved_glasses_ids);

	// Properties.

	// Signals.
	ADD_SIGNAL(MethodInfo("glasses_event", PropertyInfo(Variant::STRING, "glasses_id"), PropertyInfo(Variant::INT, "event")));
	// ClassDB::bind_method(D_METHOD("emit_custom_signal", "name", "value"), &Example::emit_custom_signal);

	// Constants.
	BIND_ENUM_CONSTANT(E_ADDED);       
	BIND_ENUM_CONSTANT(E_LOST);       
	BIND_ENUM_CONSTANT(E_AVAILABLE); 
	BIND_ENUM_CONSTANT(E_UNAVAILABLE); 
	BIND_ENUM_CONSTANT(E_RESERVED); 
	BIND_ENUM_CONSTANT(E_DROPPED); 
	BIND_ENUM_CONSTANT(E_TRACKING); 
	BIND_ENUM_CONSTANT(E_NOT_TRACKING); 
	BIND_ENUM_CONSTANT(E_STOPPED_ON_ERROR);
}



TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_entry(StringName glasses_id) {
	for(auto& entry : _glasses_index) {
		if(glasses_id == entry.id) {
			return &entry;
		}
	}	
	return nullptr;
}



TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_by_render_target(RID test_render_target) {
	auto render_server = RenderingServer::get_singleton();

	for(auto& entry : _glasses_index) {
		auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
		if(!viewport) continue;
        auto glasses_render_target = render_server->viewport_get_render_target(viewport->get_viewport_rid());   
        if(test_render_target == glasses_render_target) {    
            return &entry;
        }
	}
	return nullptr;
}

TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_by_viewport(RID test_viewport) {
	for(auto& entry : _glasses_index) {
		auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
		if(!viewport) continue;
        if(test_viewport == viewport->get_viewport_rid()) {    
            return &entry;
        }
	}
	return nullptr;
}

bool TiltFiveXRInterface::setup() {
	xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V_MSG(xr_server, false, "XRServer unavailable");

	t5_service = GodotT5ObjectRegistry::service();
	ERR_FAIL_COND_V_MSG(!t5_service, false, "Couldn't obtain GodotT5Service singleton");

    RenderingServer *rendering_server = RenderingServer::get_singleton();
    ERR_FAIL_NULL_V(rendering_server, false);
    RenderingDevice *rendering_device = rendering_server->get_rendering_device();
	if(rendering_device){
		WARN_PRINT("The using vulkan renderer for Tilt Five is not currently functional");
		t5_service->use_vulkan_api();
	} else {
		t5_service->use_opengl_api();
	}

	xr_server->add_interface(this);
	return true;
}

void TiltFiveXRInterface::teardown() {
	
	if(xr_server)
		xr_server->remove_interface(this);

	t5_service.reset();

	xr_server = nullptr;
}

bool TiltFiveXRInterface::start_service(const String application_id, const String application_version)
{
    if(!setup()) return false;

	auto ai = application_id.ascii();
	auto av = application_version.ascii();

    bool is_started = t5_service->start_service(ai.get_data(), av.get_data());
	if(is_started)
		initialize();
	return is_started;
}

void TiltFiveXRInterface::stop_service()
{
	if(t5_service->is_service_started()) {
		set_primary(false);
		uninitialize();
		t5_service->stop_service();
	}

	teardown();
}

void TiltFiveXRInterface::reserve_glasses(const StringName glasses_id, const String display_name) {
    if(!t5_service) return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");

	auto dn = display_name.ascii();
	t5_service->reserve_glasses(entry->idx, dn.get_data());
}

void TiltFiveXRInterface::start_display(const StringName glasses_id, Variant vobj, Variant oobj) {
    if(!t5_service) return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");

	auto viewport = Object::cast_to<SubViewport>(vobj);
	auto xr_origin = Object::cast_to<XROrigin3D>(oobj);
	ERR_FAIL_NULL_MSG(viewport, "Parameter 2 is not a SubViewport");
	ERR_FAIL_NULL_MSG(xr_origin, "Parameter 3 is not a XROrigin3D");

	_start_display(*entry, viewport, xr_origin);
}

void TiltFiveXRInterface::_start_display(TiltFiveXRInterface::GlassesIndexEntry& entry, SubViewport* viewport, XROrigin3D* xr_origin) {
	auto glasses = entry.glasses.lock();
	if(!glasses->is_reserved()) {
		WARN_PRINT("Glasses need to be reserved to display viewport");
		return;
	}
	entry.viewport_id = viewport->get_instance_id();
	entry.xr_origin_id = xr_origin->get_instance_id();

	viewport->set_use_xr(true);
	viewport->set_update_mode(godot::SubViewport::UpdateMode::UPDATE_ALWAYS);
}

void TiltFiveXRInterface::stop_display(const StringName glasses_id) {
	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");
	_stop_display(*entry);
}


void TiltFiveXRInterface::_stop_display(GlassesIndexEntry& entry) {
	auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
	if(viewport) {
		viewport->set_use_xr(false);
		viewport->set_update_mode(godot::SubViewport::UpdateMode::UPDATE_DISABLED);
	}
	entry.viewport_id = ObjectID();
	entry.xr_origin_id = ObjectID();
}

void TiltFiveXRInterface::release_glasses(const StringName glasses_id) {
    if(!t5_service) return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");
	_stop_display(*entry);
	t5_service->release_glasses(entry->idx);
}

PackedStringArray TiltFiveXRInterface::get_available_glasses_ids() {
    if(!t5_service) return PackedStringArray();
	
	PackedStringArray available_list;
	for(int i = 0; i < t5_service->get_glasses_count(); i++) {
		auto glasses = t5_service->get_glasses(i);
		if(glasses->is_available())
			available_list.append(glasses->get_id().c_str());
	}
	return available_list;
}

PackedStringArray TiltFiveXRInterface::get_reserved_glasses_ids() {
    if(!t5_service) return PackedStringArray();
	
	PackedStringArray reserved_list;
	for(int i = 0; i < t5_service->get_glasses_count(); i++) {
		auto glasses = t5_service->get_glasses(i);
		if(glasses->is_in_use())
			reserved_list.append(glasses->get_id().c_str());
	}
	return reserved_list;
}

StringName TiltFiveXRInterface::_get_name() const {
	StringName name("TiltFive");
	return name;
}

uint32_t TiltFiveXRInterface::_get_capabilities() const {
	return XR_STEREO | XR_AR;
}

bool TiltFiveXRInterface::_is_initialized() const {
	return _initialised;
}

bool TiltFiveXRInterface::_initialize() {
	if (!_initialised) {
		_initialised = ((bool)t5_service) && xr_server;
	}
	return _initialised;
}

void TiltFiveXRInterface::_uninitialize() {
	if (_initialised) {
		_initialised = false;
	}
}

bool TiltFiveXRInterface::_supports_play_area_mode(XRInterface::PlayAreaMode mode) const {
    return mode == XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::PlayAreaMode TiltFiveXRInterface::_get_play_area_mode() const {
    return XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::TrackingStatus TiltFiveXRInterface::_get_tracking_status() const {
	if(!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return XRInterface::TrackingStatus::XR_NOT_TRACKING;
	}
	return _render_glasses->is_tracking() ? XRInterface::TrackingStatus::XR_NORMAL_TRACKING : XRInterface::TrackingStatus::XR_NOT_TRACKING;
}

Vector2 TiltFiveXRInterface::_get_render_target_size() {
	if(!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Vector2(1216, 768);
	}
    return _render_glasses->get_render_size();
}

uint32_t TiltFiveXRInterface::_get_view_count() {
	return 2; // stereo
}

Transform3D TiltFiveXRInterface::_get_camera_transform() {
	if (!_initialised) {
		return Transform3D();
	}
	if(!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Transform3D();
	}

	auto hmd_transform = _render_glasses->get_head_transform();

	return xr_server->get_reference_frame() * hmd_transform;
}

Transform3D TiltFiveXRInterface::_get_transform_for_view(uint32_t view, const Transform3D &origin_transform) {
	if (!_initialised) {
		return Transform3D();
	}
	if(!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Transform3D();
	}
	auto world_scale = xr_server->get_world_scale();

	auto eye_transform = _render_glasses->get_eye_transform(view == 0 ? Eye::Left : Eye::Right);
	eye_transform.scale(Vector3(world_scale, world_scale, world_scale));

	return origin_transform * eye_transform;
}

PackedFloat64Array TiltFiveXRInterface::_get_projection_for_view(uint32_t p_view, double aspect, double z_near, double z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix

	if (!_initialised) {
		return arr;
	}
	if(!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return arr;
	}

	return _render_glasses->get_projection_for_eye(p_view == 0 ? Eye::Left : Eye::Right, aspect, z_near, z_far);
}

bool TiltFiveXRInterface::_pre_draw_viewport(const RID &render_target) {
	ERR_FAIL_COND_V_MSG(_render_glasses, "Rendering viewport already set", false);
	auto entry = lookup_glasses_by_render_target(render_target);
	ERR_FAIL_COND_V_MSG(!entry, "Viewport does not have associated glasses", false);

	_render_glasses = entry->glasses.lock();

    if(!_render_glasses->is_reserved()) 
		return false;

	auto xr_origin = Object::cast_to<TiltFiveGameboard>(ObjectDB::get_instance(entry->xr_origin_id));
	if(!xr_origin)
		return false;

	xr_server->set_world_origin(xr_origin->get_global_transform());
	xr_server->set_world_scale(xr_origin->get_gameboard_scale());
	 
	entry->rendering = true;
	return true;
}

void TiltFiveXRInterface::_post_draw_viewport(const RID &render_target, const Rect2 &screen_rect) {
	Rect2 src_rect(0.0f, 0.0f, 1.0f, 1.0f);
	Rect2 dst_rect = screen_rect;

	// halve our width
	Vector2 size = dst_rect.get_size();
	size.x = size.x * 0.5;
	dst_rect.size = size;

	Vector2 eye_center(0.0, 0.0);

	//add_blit(render_target, src_rect, screen_rect, true, 0, false, eye_center, 0, 0, 0, 0);
	_render_glasses.reset();
}

void TiltFiveXRInterface::_end_frame() {

	for(auto& entry : _glasses_index) {
		if(entry.rendering) {
			entry.glasses.lock()->send_frame();
			entry.rendering = false;
		}
	}	
}

PackedStringArray TiltFiveXRInterface::_get_suggested_tracker_names() const {
	PackedStringArray tracker_names;
	
	tracker_names.append("glasses/tilt_five_wand_1");
	tracker_names.append("glasses/tilt_five_wand_2");
	tracker_names.append("glasses/tilt_five_wand_3");
	tracker_names.append("glasses/tilt_five_wand_4");

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

	_events.clear();
    t5_service->get_events(_events);
    for(int i = 0; i < _events.size(); i++) 
    {
		auto glasses_idx = _events[i].glasses_num;
		switch (_events[i].event)
		{
			case GlassesEvent::E_ADDED: {
				if(_glasses_index.size() != glasses_idx) {
					WARN_PRINT("Glasses index");
				}
				_glasses_index.resize(glasses_idx + 1);
				auto glasses = t5_service->get_glasses(glasses_idx);
				_glasses_index[glasses_idx].glasses = glasses;
				_glasses_index[glasses_idx].id = glasses->get_id().c_str();
				_glasses_index[glasses_idx].idx = glasses_idx;
				_glasses_index[glasses_idx].rendering = false;

			} break;
			case GlassesEvent::E_CONNECTED: {
				++reserved_glasses_count;
				if(reserved_glasses_count > 0 && !is_primary()) {
					set_primary(true);
				}
			} break;
			case GlassesEvent::E_DISCONNECTED: {
				_stop_display(_glasses_index[glasses_idx]);
				--reserved_glasses_count;
				if(reserved_glasses_count == 0 && is_primary()) {
					set_primary(false);
				}
			} break;
			case GlassesEvent::E_LOST:
			case GlassesEvent::E_UNAVAILABLE: {
				_stop_display(_glasses_index[glasses_idx]);
			} break;
		
			default: break;
		}
	    emit_signal("glasses_event", _glasses_index[_events[i].glasses_num].id, (int)_events[i].event);
    }
}

RID TiltFiveXRInterface::_get_color_texture() {
	ERR_FAIL_COND_V_MSG(!_render_glasses, RID(), "Glasses not set");
	return _render_glasses->get_color_texture();	
}

	
bool TiltFiveXRInterface::_get_anchor_detection_is_enabled() const {
	return false;
}

void TiltFiveXRInterface::_set_anchor_detection_is_enabled(bool enabled) {
	ERR_FAIL_COND_MSG(enabled, "Tilt Five does not support anchors");
}

int32_t TiltFiveXRInterface::_get_camera_feed_id() const {
	return 0;
}

TiltFiveXRInterface::TiltFiveXRInterface() {
}

TiltFiveXRInterface::~TiltFiveXRInterface() {
}