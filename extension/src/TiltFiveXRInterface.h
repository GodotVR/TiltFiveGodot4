#ifndef TILT_FIVE_XR_INTERFACE_H
#define TILT_FIVE_XR_INTERFACE_H

#include <godot_cpp/classes/xr_interface_extension.hpp>

#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <GodotT5Glasses.h>
#include <GodotT5Service.h>
#include <T5Origin3D.h>

using godot::AABB;
using godot::ObjectID;
using godot::PackedFloat64Array;
using godot::PackedStringArray;
using godot::Rect2;
using godot::RID;
using godot::String;
using godot::StringName;
using godot::SubViewport;
using godot::Transform3D;
using godot::Variant;
using godot::Vector2;
using godot::XRInterfaceExtension;
using godot::XRServer;
using GodotT5Integration::GodotT5Glasses;
using GodotT5Integration::GodotT5Service;
using T5Integration::GlassesEvent;
using T5Integration::T5ServiceEvent;

// ID assigned to this Godot plugin
static constexpr uint8_t kSdkTypeCommunityGodot = 0x70;

class TiltFiveXRInterface : public XRInterfaceExtension {
	GDCLASS(TiltFiveXRInterface, XRInterfaceExtension);

	// clang-format off
	enum GameBoardType {
		NO_GAMEBOARD_SET 	= kT5_GameboardType_None,
		LE_GAMEBOARD 		= kT5_GameboardType_LE,
		XE_GAMEBOARD 		= kT5_GameboardType_XE,
		XE_RAISED_GAMEBOARD = kT5_GameboardType_XE_Raised
	};
	// clang-format on

	struct GlassesIndexEntry {
		StringName id;
		int idx;
		std::weak_ptr<GodotT5Glasses> glasses;
		ObjectID viewport_id;
		ObjectID gameboard_id;
		bool rendering;
	};

public:
	// Constants.

	// clang-format off
    enum ServiceEventType
    {
		E_SERVICE_STOPPED					= T5ServiceEvent::E_STOPPED,
		E_SERVICE_RUNNING					= T5ServiceEvent::E_RUNNING,
		E_SERVICE_T5_UNAVAILABLE			= T5ServiceEvent::E_T5_UNAVAILABLE,
		E_SERVICE_T5_INCOMPATIBLE_VERSION	= T5ServiceEvent::E_T5_INCOMPATIBLE_VERSION
    };

	enum GlassesEventType
	{
		E_GLASSES_ADDED				= GlassesEvent::E_ADDED,
		E_GLASSES_LOST				= GlassesEvent::E_LOST,
		E_GLASSES_AVAILABLE			= GlassesEvent::E_AVAILABLE,
		E_GLASSES_UNAVAILABLE		= GlassesEvent::E_UNAVAILABLE,
		E_GLASSES_RESERVED			= GlassesEvent::E_CONNECTED,
		E_GLASSES_DROPPED			= GlassesEvent::E_DISCONNECTED,
		E_GLASSES_TRACKING			= GlassesEvent::E_TRACKING,
		E_GLASSES_NOT_TRACKING		= GlassesEvent::E_NOT_TRACKING,
		E_GLASSES_STOPPED_ON_ERROR 	= GlassesEvent::E_STOPPED_ON_ERROR
	};
	// clang-format on

	// Property setters and getters.

	String get_application_id() const;
	void set_application_id(const String &p_string);

	String get_application_version() const;
	void set_application_version(const String &p_string);

	float get_trigger_click_threshold();
	void set_trigger_click_threshold(float threshold);

	bool get_debug_logging();
	void set_debug_logging(bool is_debug);

	// Functions.

	void reserve_glasses(const StringName glasses_id, const String display_name);
	void start_display(const StringName glasses_id, Variant viewport, Variant xr_origin);
	void stop_display(const StringName glasses_id);
	void release_glasses(const StringName glasses_id);

	GameBoardType get_gameboard_type(const StringName glasses_id);
	AABB get_gameboard_extents(GameBoardType gameboard_type);

	PackedStringArray get_available_glasses_ids();
	PackedStringArray get_reserved_glasses_ids();

	// Overriden from XRInterfaceExtension
	virtual StringName _get_name() const override;
	virtual uint32_t _get_capabilities() const override;

	virtual bool _is_initialized() const override;
	virtual bool _initialize() override;
	virtual void _uninitialize() override;

	virtual bool _supports_play_area_mode(XRInterface::PlayAreaMode mode) const override;
	virtual XRInterface::PlayAreaMode _get_play_area_mode() const override;

	virtual XRInterface::TrackingStatus _get_tracking_status() const override;

	virtual Vector2 _get_render_target_size() override;
	virtual uint32_t _get_view_count() override;
	virtual Transform3D _get_camera_transform() override;
	virtual Transform3D _get_transform_for_view(uint32_t view, const Transform3D &cam_transform) override;
	virtual PackedFloat64Array _get_projection_for_view(uint32_t view, double aspect, double z_near, double z_far) override;

	virtual bool _pre_draw_viewport(const RID &render_target);
	virtual void _post_draw_viewport(const RID &render_target, const Rect2 &screen_rect) override;
	virtual void _end_frame() override;

	virtual PackedStringArray _get_suggested_tracker_names() const override;
	virtual PackedStringArray _get_suggested_pose_names(const StringName &tracker_name) const override;

	virtual void _process() override;

	virtual RID _get_color_texture() override;

	virtual bool _get_anchor_detection_is_enabled() const override;
	virtual void _set_anchor_detection_is_enabled(bool enabled) override;
	virtual int32_t _get_camera_feed_id() const override;

	TiltFiveXRInterface();
	~TiltFiveXRInterface();

protected:
	bool setup_android();

	static void _bind_methods();

	void _start_display(GlassesIndexEntry &entry, SubViewport *viewport, T5Origin3D *xr_origin);
	void _stop_display(GlassesIndexEntry &entry);

	GlassesIndexEntry *lookup_glasses_entry(StringName glasses_id);
	GlassesIndexEntry *lookup_glasses_by_render_target(RID render_target);
	GlassesIndexEntry *lookup_glasses_by_viewport(RID render_target);

private:
	void log_service_events();
	void log_glasses_events();

	bool _initialised = false;
	XRServer *xr_server = nullptr;

	String application_id;
	String application_version;
	float _trigger_click_threshold = 0.5;
	bool _is_debug_logging = false;
	long _platform_context = 0;

	std::vector<GlassesIndexEntry> _glasses_index;
	std::vector<GlassesEvent> _glasses_events;
	std::vector<T5ServiceEvent> _service_events;

	GodotT5Service::Ptr t5_service;

	GodotT5Glasses::Ptr _render_glasses;

	int reserved_glasses_count = 0;
};

VARIANT_ENUM_CAST(TiltFiveXRInterface::GameBoardType)
VARIANT_ENUM_CAST(TiltFiveXRInterface::ServiceEventType);
VARIANT_ENUM_CAST(TiltFiveXRInterface::GlassesEventType);

#endif // ! TILT_FIVE_XR_INTERFACE_H