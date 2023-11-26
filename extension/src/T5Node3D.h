#ifndef T5_NODE_3D_H
#define T5_NODE_3D_H
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
#include <GodotT5Service.h>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/packed_data_container.hpp>
#include <godot_cpp/classes/xr_pose.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/classes/node3d.hpp>

using godot::Ref;
using godot::XRServer;
using godot::String;
using godot::StringName;
using godot::PackedStringArray;
using godot::XRPose;
using godot::XRPositionalTracker;
using godot::Node3D;
using GodotT5Integration::GodotT5Service;
using GodotT5Integration::GodotT5Glasses;
using T5Integration::ObjectRegistry;

class T5Node3D : public Node3D {
	GDCLASS(T5Node3D, Node3D);

public:

	//void _validate_property(PropertyInfo &p_property) const;
	void set_tracker(const StringName p_tracker_name);
	StringName get_tracker() const;
	void set_pose_name(const StringName p_pose_name);
	StringName get_pose_name() const;
    bool get_is_active() const;
	bool get_has_tracking_data() const;
	Ref<XRPose> get_pose();
	PackedStringArray get_configuration_warnings(PackedStringArray& warnings) const;

    T5Node3D();
    ~T5Node3D();
    
protected:
	Ref<XRPositionalTracker> tracker;

	static void _bind_methods();

	virtual void _bind_tracker();
	virtual void _unbind_tracker();

	void _changed_tracker(const StringName p_tracker_name, int p_tracker_type);
	void _removed_tracker(const StringName p_tracker_name, int p_tracker_type);
	void _pose_changed(Object* p_obj);

	GodotT5Glasses::Ptr get_associated_glasses();
	int get_associated_wand_num();

private:
	StringName tracker_name;
	StringName pose_name = "default";

	int _indexes_associated = false;
	int _glasses_idx = -1;
	int _wand_idx = -1;
};

#endif // T5_NODE_3D_H