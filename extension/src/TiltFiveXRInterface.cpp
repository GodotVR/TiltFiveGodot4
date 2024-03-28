#include "TiltFiveXRInterface.h"
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using GodotT5Integration::GodotT5ObjectRegistry;
using T5Integration::GlassesEvent;
using Eye = GodotT5Integration::Glasses::Eye;

void TiltFiveXRInterface::_bind_methods() {
	// Methods.

	ClassDB::bind_method(D_METHOD("reserve_glasses", "glasses_id", "display_name"), &TiltFiveXRInterface::reserve_glasses);
	ClassDB::bind_method(D_METHOD("start_display", "glasses_id", "viewport", "gameboard"), &TiltFiveXRInterface::start_display);
	ClassDB::bind_method(D_METHOD("stop_display", "glasses_id"), &TiltFiveXRInterface::stop_display);
	ClassDB::bind_method(D_METHOD("release_glasses", "glasses_id"), &TiltFiveXRInterface::release_glasses);
	ClassDB::bind_method(D_METHOD("get_available_glasses_ids"), &TiltFiveXRInterface::get_available_glasses_ids);
	ClassDB::bind_method(D_METHOD("get_reserved_glasses_ids"), &TiltFiveXRInterface::get_reserved_glasses_ids);
	ClassDB::bind_method(D_METHOD("get_glasses_name", "glasses_id"), &TiltFiveXRInterface::get_glasses_name);
	ClassDB::bind_method(D_METHOD("get_gameboard_type", "glasses_id"), &TiltFiveXRInterface::get_gameboard_type);
	ClassDB::bind_method(D_METHOD("get_gameboard_extents", "gameboard_type"), &TiltFiveXRInterface::get_gameboard_extents);

	// Properties.
	ClassDB::bind_method(D_METHOD("set_application_id", "application_id"), &TiltFiveXRInterface::set_application_id);
	ClassDB::bind_method(D_METHOD("get_application_id"), &TiltFiveXRInterface::get_application_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "application_id"), "set_application_id", "get_application_id");

	ClassDB::bind_method(D_METHOD("set_application_version", "application_version"), &TiltFiveXRInterface::set_application_version);
	ClassDB::bind_method(D_METHOD("get_application_version"), &TiltFiveXRInterface::get_application_version);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "application_version"), "set_application_version", "get_application_version");

	ClassDB::bind_method(D_METHOD("set_trigger_click_threshold", "threshold"), &TiltFiveXRInterface::set_trigger_click_threshold);
	ClassDB::bind_method(D_METHOD("get_trigger_click_threshold"), &TiltFiveXRInterface::get_trigger_click_threshold);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "trigger_click_threshold"), "set_trigger_click_threshold", "get_trigger_click_threshold");

	ClassDB::bind_method(D_METHOD("set_debug_logging", "debug_logging"), &TiltFiveXRInterface::set_debug_logging);
	ClassDB::bind_method(D_METHOD("get_debug_logging"), &TiltFiveXRInterface::get_debug_logging);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_logging"), "set_debug_logging", "get_debug_logging");

	// Signals.
	ADD_SIGNAL(MethodInfo("service_event", PropertyInfo(Variant::INT, "event")));
	ADD_SIGNAL(MethodInfo("glasses_event", PropertyInfo(Variant::STRING, "glasses_id"), PropertyInfo(Variant::INT, "event")));

	// Constants.
	BIND_ENUM_CONSTANT(E_SERVICE_STOPPED);
	BIND_ENUM_CONSTANT(E_SERVICE_RUNNING);
	BIND_ENUM_CONSTANT(E_SERVICE_T5_UNAVAILABLE);
	BIND_ENUM_CONSTANT(E_SERVICE_T5_INCOMPATIBLE_VERSION);

	BIND_ENUM_CONSTANT(E_GLASSES_ADDED);
	BIND_ENUM_CONSTANT(E_GLASSES_LOST);
	BIND_ENUM_CONSTANT(E_GLASSES_AVAILABLE);
	BIND_ENUM_CONSTANT(E_GLASSES_UNAVAILABLE);
	BIND_ENUM_CONSTANT(E_GLASSES_RESERVED);
	BIND_ENUM_CONSTANT(E_GLASSES_DROPPED);
	BIND_ENUM_CONSTANT(E_GLASSES_TRACKING);
	BIND_ENUM_CONSTANT(E_GLASSES_NOT_TRACKING);
	BIND_ENUM_CONSTANT(E_GLASSES_STOPPED_ON_ERROR);

	BIND_ENUM_CONSTANT(NO_GAMEBOARD_SET);
	BIND_ENUM_CONSTANT(LE_GAMEBOARD);
	BIND_ENUM_CONSTANT(XE_GAMEBOARD);
	BIND_ENUM_CONSTANT(XE_RAISED_GAMEBOARD);
}

String TiltFiveXRInterface::get_application_id() const {
	return application_id;
}

void TiltFiveXRInterface::set_application_id(const String& p_string) {
	application_id = p_string;
}

String TiltFiveXRInterface::get_application_version() const {
	return application_version;
}

void TiltFiveXRInterface::set_application_version(const String& p_string) {
	application_version = p_string;
}

float TiltFiveXRInterface::get_trigger_click_threshold() {
	return _trigger_click_threshold;
}

void TiltFiveXRInterface::set_trigger_click_threshold(float threshold) {
	_trigger_click_threshold = threshold;

	for (auto& entry : _glasses_index) {
		if (!entry.glasses.expired()) {
			entry.glasses.lock()->set_trigger_click_threshold(_trigger_click_threshold);
		}
	}
}

bool TiltFiveXRInterface::get_debug_logging() {
	return GodotT5ObjectRegistry::logger()->get_debug();
}

void TiltFiveXRInterface::set_debug_logging(bool is_debug) {
	GodotT5ObjectRegistry::logger()->set_debug(is_debug);
}

TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_entry(StringName glasses_id) {
	for (auto& entry : _glasses_index) {
		if (glasses_id == entry.id) {
			return &entry;
		}
	}
	return nullptr;
}

TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_by_render_target(RID test_render_target) {
	auto render_server = RenderingServer::get_singleton();

	for (auto& entry : _glasses_index) {
		auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
		if (!viewport)
			continue;
		auto glasses_render_target = render_server->viewport_get_render_target(viewport->get_viewport_rid());
		if (test_render_target == glasses_render_target) {
			return &entry;
		}
	}
	return nullptr;
}

TiltFiveXRInterface::GlassesIndexEntry* TiltFiveXRInterface::lookup_glasses_by_viewport(RID test_viewport) {
	for (auto& entry : _glasses_index) {
		auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
		if (!viewport)
			continue;
		if (test_viewport == viewport->get_viewport_rid()) {
			return &entry;
		}
	}
	return nullptr;
}

bool TiltFiveXRInterface::_is_initialized() const {
	return _initialised;
}

bool TiltFiveXRInterface::_initialize() {
	// Already initialized?
	if (_initialised) {
		return true;
	}

	xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V_MSG(xr_server, false, "XRServer unavailable");

	t5_service = GodotT5ObjectRegistry::service();
	ERR_FAIL_COND_V_MSG(!t5_service, false, "Couldn't obtain GodotT5Service singleton");

	RenderingServer* rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rendering_server, false);
	RenderingDevice* rendering_device = rendering_server->get_rendering_device();
	if (rendering_device) {
		t5_service->use_vulkan_api();
	} else {
		t5_service->use_opengl_api();
	}

	auto ai = application_id.ascii();
	auto av = application_version.ascii();

	bool is_started = t5_service->start_service(ai.get_data(), av.get_data(), kSdkTypeCommunityGodot);
	ERR_FAIL_COND_V_MSG(!is_started, false, "Couldn't start T5 Service");

	_initialised = true;
	return true;
}

void TiltFiveXRInterface::_uninitialize() {
	if (_initialised) {
		if (t5_service->is_service_started()) {
			t5_service->stop_service();
		}
		t5_service.reset();

		if (xr_server->get_primary_interface() == this) {
			xr_server->set_primary_interface(Ref<XRInterface>());
		}

		xr_server = nullptr;
		_initialised = false;
	}
}

void TiltFiveXRInterface::reserve_glasses(const StringName glasses_id, const String display_name) {
	if (!t5_service)
		return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");

	auto dn = display_name.ascii();
	t5_service->reserve_glasses(entry->idx, dn.get_data());
}

void TiltFiveXRInterface::start_display(const StringName glasses_id, Variant vobj, Variant oobj) {
	if (!t5_service)
		return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");

	auto viewport = Object::cast_to<SubViewport>(vobj);
	auto gameboard = Object::cast_to<T5Origin3D>(oobj);
	ERR_FAIL_NULL_MSG(viewport, "Parameter 2 is not a SubViewport");
	ERR_FAIL_NULL_MSG(gameboard, "Parameter 3 is not a T5Origin3D");

	_start_display(*entry, viewport, gameboard);
}

void TiltFiveXRInterface::_start_display(TiltFiveXRInterface::GlassesIndexEntry& entry, SubViewport* viewport, T5Origin3D* gameboard) {
	auto glasses = entry.glasses.lock();
	if (!glasses->is_reserved()) {
		WARN_PRINT("Glasses need to be reserved to display viewport");
		return;
	}
	glasses->start_display();
	entry.viewport_id = viewport->get_instance_id();
	entry.gameboard_id = gameboard->get_instance_id();

	viewport->set_use_xr(true);
	viewport->set_update_mode(godot::SubViewport::UpdateMode::UPDATE_ALWAYS);
}

void TiltFiveXRInterface::stop_display(const StringName glasses_id) {
	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");
	_stop_display(*entry);
}

void TiltFiveXRInterface::_stop_display(GlassesIndexEntry& entry) {
	auto glasses = entry.glasses.lock();
	auto viewport = Object::cast_to<SubViewport>(ObjectDB::get_instance(entry.viewport_id));
	if (viewport) {
		viewport->set_use_xr(false);
		viewport->set_update_mode(godot::SubViewport::UpdateMode::UPDATE_DISABLED);
	}
	glasses->stop_display();
	entry.viewport_id = ObjectID();
	entry.gameboard_id = ObjectID();
}

void TiltFiveXRInterface::release_glasses(const StringName glasses_id) {
	if (!t5_service)
		return;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_MSG(!entry, "Glasses id was not found");
	_stop_display(*entry);
	t5_service->release_glasses(entry->idx);
}

PackedStringArray TiltFiveXRInterface::get_available_glasses_ids() {
	if (!t5_service)
		return PackedStringArray();

	PackedStringArray available_list;
	for (int i = 0; i < t5_service->get_glasses_count(); i++) {
		auto glasses = t5_service->get_glasses(i);
		if (glasses->is_available())
			available_list.append(glasses->get_id().c_str());
	}
	return available_list;
}

PackedStringArray TiltFiveXRInterface::get_reserved_glasses_ids() {
	if (!t5_service)
		return PackedStringArray();

	PackedStringArray reserved_list;
	for (int i = 0; i < t5_service->get_glasses_count(); i++) {
		auto glasses = t5_service->get_glasses(i);
		if (glasses->is_in_use())
			reserved_list.append(glasses->get_id().c_str());
	}
	return reserved_list;
}

String TiltFiveXRInterface::get_glasses_name(const StringName glasses_id) {
	if(!t5_service)
		return String("");

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_V_MSG(!entry, String(""), "Glasses id was not found");

	std::string glasses_name = t5_service->get_glasses_name(entry->idx);
	return String(glasses_name.c_str());
}

TiltFiveXRInterface::GameBoardType TiltFiveXRInterface::get_gameboard_type(const StringName glasses_id) {
	if (!t5_service)
		return NO_GAMEBOARD_SET;

	auto entry = lookup_glasses_entry(glasses_id);
	ERR_FAIL_COND_V_MSG(!entry, NO_GAMEBOARD_SET, "Glasses id was not found");

	return static_cast<GameBoardType>(entry->glasses.lock()->get_gameboard_type());
}

AABB TiltFiveXRInterface::get_gameboard_extents(GameBoardType gameboard_type) {
	AABB result;
	if (!t5_service)
		return result;

	T5_GameboardSize size;
	t5_service->get_gameboard_size(static_cast<T5_GameboardType>(gameboard_type), size);

	result.set_position(Vector3(-size.viewableExtentNegativeX, -size.viewableExtentNegativeY, 0));
	result.set_end(Vector3(size.viewableExtentPositiveX, size.viewableExtentPositiveY, size.viewableExtentPositiveZ));

	return result;
}

StringName TiltFiveXRInterface::_get_name() const {
	StringName name("TiltFive");
	return name;
}

uint32_t TiltFiveXRInterface::_get_capabilities() const {
	return XR_STEREO | XR_AR;
}

bool TiltFiveXRInterface::_supports_play_area_mode(XRInterface::PlayAreaMode mode) const {
	return mode == XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::PlayAreaMode TiltFiveXRInterface::_get_play_area_mode() const {
	return XRInterface::PlayAreaMode::XR_PLAY_AREA_STAGE;
}

XRInterface::TrackingStatus TiltFiveXRInterface::_get_tracking_status() const {
	if (!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return XRInterface::TrackingStatus::XR_NOT_TRACKING;
	}
	return _render_glasses->is_tracking() ? XRInterface::TrackingStatus::XR_NORMAL_TRACKING : XRInterface::TrackingStatus::XR_NOT_TRACKING;
}

Vector2 TiltFiveXRInterface::_get_render_target_size() {
	if (!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Vector2(1216, 768);
	}
	return _render_glasses->get_render_size();
}

uint32_t TiltFiveXRInterface::_get_view_count() {
	return 2; // stereo
}

Transform3D TiltFiveXRInterface::_get_camera_transform() {
	ERR_FAIL_NULL_V_MSG(xr_server, Transform3D(), "XRServer unavailable");

	if (!_initialised) {
		return Transform3D();
	}
	if (!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Transform3D();
	}

	auto hmd_transform = _render_glasses->get_head_transform();

	// Should be the gameboard scale set in _pre_draw_viewport.
	auto world_scale = xr_server->get_world_scale();
	hmd_transform.origin *= world_scale;

	// We want the transform not adjusted by the reference frame.
	return hmd_transform;
}

Transform3D TiltFiveXRInterface::_get_transform_for_view(uint32_t view, const Transform3D& origin_transform) {
	ERR_FAIL_NULL_V_MSG(xr_server, Transform3D(), "XRServer unavailable");

	if (!_initialised) {
		return Transform3D();
	}
	if (!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return Transform3D();
	}

	auto eye_transform = _render_glasses->get_eye_transform(view == 0 ? Eye::Left : Eye::Right);

	// Should be the gameboard scale set in _pre_draw_viewport.
	auto world_scale = xr_server->get_world_scale();
	eye_transform.origin *= world_scale;

	// Apply origin and reference frame.
	return origin_transform * xr_server->get_reference_frame() * eye_transform;
}

PackedFloat64Array TiltFiveXRInterface::_get_projection_for_view(uint32_t p_view, double aspect, double z_near, double z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix

	if (!_initialised) {
		return arr;
	}
	if (!_render_glasses) {
		WARN_PRINT_ONCE("Glasses not set");
		return arr;
	}

	auto world_scale = xr_server->get_world_scale();

	return _render_glasses->get_projection_for_eye(p_view == 0 ? Eye::Left : Eye::Right, aspect, z_near * world_scale, z_far * world_scale);
}

bool TiltFiveXRInterface::_pre_draw_viewport(const RID& render_target) {
	ERR_FAIL_NULL_V_MSG(xr_server, false, "XRServer unavailable");
	ERR_FAIL_COND_V_MSG(_render_glasses, false, "Rendering viewport already set");
	auto entry = lookup_glasses_by_render_target(render_target);
	ERR_FAIL_COND_V_MSG(!entry, false, "Viewport does not have associated glasses");

	_render_glasses = entry->glasses.lock();

	if (!_render_glasses->is_reserved())
		return false;

	auto gameboard = Object::cast_to<T5Origin3D>(ObjectDB::get_instance(entry->gameboard_id));
	if (!gameboard)
		return false;

	xr_server->set_world_origin(gameboard->get_global_transform());
	xr_server->set_world_scale(gameboard->get_gameboard_scale());

	entry->rendering = true;
	return true;
}

void TiltFiveXRInterface::_post_draw_viewport(const RID& render_target, const Rect2& screen_rect) {
	_render_glasses->on_post_draw();
	_render_glasses.reset();
}

void TiltFiveXRInterface::_end_frame() {
	for (auto& entry : _glasses_index) {
		if (entry.rendering) {
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

PackedStringArray TiltFiveXRInterface::_get_suggested_pose_names(const StringName& tracker_name) const {
	PackedStringArray tracker_names;

	tracker_names.append("aim");
	tracker_names.append("grip");
	tracker_names.append("finger");

	return tracker_names;
}

void TiltFiveXRInterface::log_service_events() {
	if (get_debug_logging() || OS::get_singleton()->is_stdout_verbose()) {
		for (auto& event : _service_events) {
			switch (event.event) {
				case T5ServiceEvent::E_RUNNING:
					LOG_MESSAGE("Tilt Five Running");
					break;
				case T5ServiceEvent::E_STOPPED:
					LOG_MESSAGE("Tilt Five Stopped");
					break;
				case T5ServiceEvent::E_T5_UNAVAILABLE:
					LOG_MESSAGE("Tilt Five Unavailable");
					break;
				case T5ServiceEvent::E_T5_INCOMPATIBLE_VERSION:
					LOG_MESSAGE("Tilt Five Incompatible Version");
					break;
			}
		}
	}
}

void TiltFiveXRInterface::log_glasses_events() {
	if (get_debug_logging() || OS::get_singleton()->is_stdout_verbose()) {
		for (auto& event : _glasses_events) {
			auto glasses = t5_service->get_glasses(event.glasses_num);
			if (!glasses)
				continue;
			switch (event.event) {
				case GlassesEventType::E_GLASSES_ADDED:
					LOG_MESSAGE(glasses->get_id(), " Added");
					break;
				case GlassesEventType::E_GLASSES_LOST:
					LOG_MESSAGE(glasses->get_id(), " Lost");
					break;
				case GlassesEventType::E_GLASSES_AVAILABLE:
					LOG_MESSAGE(glasses->get_id(), " Available to use");
					break;
				case GlassesEventType::E_GLASSES_UNAVAILABLE:
					LOG_MESSAGE(glasses->get_id(), " Unavailable to use");
					break;
				case GlassesEventType::E_GLASSES_RESERVED:
					LOG_MESSAGE(glasses->get_id(), " Reserved for application");
					break;
				case GlassesEventType::E_GLASSES_DROPPED:
					LOG_MESSAGE(glasses->get_id(), " Reservation dropped");
					break;
				case GlassesEventType::E_GLASSES_TRACKING:
					LOG_MESSAGE(glasses->get_id(), " Tracking pose");
					break;
				case GlassesEventType::E_GLASSES_NOT_TRACKING:
					LOG_MESSAGE(glasses->get_id(), " Not tracking pose");
					break;
				case GlassesEventType::E_GLASSES_STOPPED_ON_ERROR:
					LOG_MESSAGE(glasses->get_id(), " Stopped with unknown error");
					break;
			}
		}
	}
}

void TiltFiveXRInterface::_process() {
	if (!t5_service)
		return;

	t5_service->update_connection();
	t5_service->update_tracking();

	_service_events.clear();
	t5_service->get_service_events(_service_events);
	log_service_events();
	for (int i = 0; i < _service_events.size(); i++) {
		emit_signal("service_event", _service_events[i].event);
	}

	_glasses_events.clear();
	t5_service->get_glasses_events(_glasses_events);
	log_glasses_events();
	for (int i = 0; i < _glasses_events.size(); i++) {
		auto glasses_idx = _glasses_events[i].glasses_num;
		switch (_glasses_events[i].event) {
			case GlassesEvent::E_ADDED: {
				if (_glasses_index.size() != glasses_idx) {
					WARN_PRINT("Glasses index");
				}
				_glasses_index.resize(glasses_idx + 1);
				auto glasses = t5_service->get_glasses(glasses_idx);
				glasses->set_trigger_click_threshold(_trigger_click_threshold);

				_glasses_index[glasses_idx].glasses = glasses;
				_glasses_index[glasses_idx].id = glasses->get_id().c_str();
				_glasses_index[glasses_idx].idx = glasses_idx;
				_glasses_index[glasses_idx].rendering = false;

			} break;
			case GlassesEvent::E_CONNECTED: {
				++reserved_glasses_count;
				if (reserved_glasses_count > 0 && !is_primary()) {
					set_primary(true);
				}
			} break;
			case GlassesEvent::E_DISCONNECTED: {
				_stop_display(_glasses_index[glasses_idx]);
				--reserved_glasses_count;
				if (reserved_glasses_count == 0 && is_primary()) {
					set_primary(false);
				}
			} break;
			case GlassesEvent::E_LOST:
			case GlassesEvent::E_UNAVAILABLE: {
				_stop_display(_glasses_index[glasses_idx]);
			} break;

			default:
				break;
		}
		emit_signal("glasses_event", _glasses_index[_glasses_events[i].glasses_num].id, (int)_glasses_events[i].event);
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
#ifdef DEV_ENABLED
	UtilityFunctions::print("Tilt Five DEV_BUILD");
#endif
}

TiltFiveXRInterface::~TiltFiveXRInterface() {
}