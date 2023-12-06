#ifndef T5_CAMERA_3D_H
#define T5_CAMERA_3D_H
//
// The XR node hierarchy was duplicated because of the need
// to set a tracker name for the Camera node. This was not
// possible for XRCamera3D in Godot 4.1. This required creating
// a custom T5Camera3D node. However since XROrigin3D requires an
// XRCamera3D as a child this meant that it need to be replaced
// too. This cascaded to the whole XR hierarchy because of
// interdependencies. If the XRCamera3D nodes are fixed in the
// future it should be possible to depreciate this custom
// T5 node hierarchy.
//
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/variant/string_name.hpp>

using godot::Camera3D;
using godot::PackedStringArray;
using godot::Ref;
using godot::StringName;
using godot::Variant;
using godot::XRPose;
using godot::XRPositionalTracker;

class T5Camera3D : public Camera3D {
	GDCLASS(T5Camera3D, Camera3D);

public:
	void set_tracker(const StringName p_tracker_name);
	StringName get_tracker() const;
	bool get_is_active() const;
	bool get_has_tracking_data() const;
	Ref<XRPose> get_pose();
	PackedStringArray get_configuration_warnings(PackedStringArray& warnings) const;

	// These are override from Camera3D currently not exposed as virtual in
	//GDExtension

	// virtual Vector3 project_local_ray_normal(const Point2 &p_pos) const override;
	// virtual Point2 unproject_position(const Vector3 &p_pos) const override;
	// virtual Vector3 project_position(const Point2 &p_point, real_t p_z_depth) const override;
	// virtual Vector<Plane> get_frustum() const override;

	T5Camera3D();
	~T5Camera3D();

protected:
	static void _bind_methods();

	StringName tracker_name = "head";
	StringName pose_name = "default";
	Ref<XRPositionalTracker> tracker;

	void _bind_tracker();
	void _unbind_tracker();
	void _changed_tracker(const StringName p_tracker_name, int p_tracker_type);
	void _removed_tracker(const StringName p_tracker_name, int p_tracker_type);
	void _pose_changed(const Variant p_pose);
};

#endif // T5_CAMERA_3D_H