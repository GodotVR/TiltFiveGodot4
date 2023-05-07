#ifndef TILT_FIVE_XR_INTERFACE_H
#define TILT_FIVE_XR_INTERFACE_H

#include <godot_cpp/classes/xr_interface_extension.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <GodotT5Service.h>
#include <GodotT5Glasses.h>

using godot::XRInterfaceExtension;
using godot::XRServer;
using godot::String;
using godot::StringName;
using godot::Vector2;
using godot::Transform3D;
using godot::PackedFloat64Array;
using godot::Rect2;
using godot::RID;
using godot::SubViewport;
using godot::PackedStringArray;
using godot::ObjectID;
using godot::XROrigin3D;
using godot::Variant;
using GodotT5Integration::GodotT5Service;
using GodotT5Integration::GodotT5Glasses;
using T5Integration::GlassesEvent;

class TiltFiveXRInterface : public XRInterfaceExtension {
	GDCLASS(TiltFiveXRInterface, XRInterfaceExtension);

	struct GlassesIndexEntry {
		StringName id;
		int idx;
		std::weak_ptr<GodotT5Glasses> glasses;
		ObjectID viewport_id;
		ObjectID xr_origin_id;
		bool rendering;
	};

public:
	// Constants.

	// Property setters and getters);

	// Functions.

	bool start_service(const String application_id, const String application_version);
	void stop_service();
	void reserve_glasses(const StringName glasses_id, const String display_name);
	void start_display(const StringName glasses_id, Variant viewport, Variant xr_origin);
	void stop_display(const StringName glasses_id);
	void release_glasses(const StringName glasses_id);

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
	static void _bind_methods();
	
	void _start_display(GlassesIndexEntry& entry, SubViewport* viewport, XROrigin3D* xr_origin);
	void _stop_display(GlassesIndexEntry& entry);

	GlassesIndexEntry* lookup_glasses_entry(StringName glasses_id);
	GlassesIndexEntry* lookup_glasses_by_render_target(RID render_target);
	GlassesIndexEntry* lookup_glasses_by_viewport(RID render_target);


private:

	bool setup();
	void teardown();

	bool _initialised = false;
	XRServer *xr_server = nullptr;

	std::vector<GlassesIndexEntry> _glasses_index;
	std::vector<GlassesEvent> _events;

    GodotT5Service::Ptr t5_service;

	GodotT5Glasses::Ptr _render_glasses;

    int reserved_glasses_count = 0;
};

#endif // ! TILT_FIVE_XR_INTERFACE_H