#include <T5Node3D.h>
#include <T5Origin3D.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/xr_server.hpp>

using godot::ClassDB;
using godot::D_METHOD;
using godot::PropertyInfo;
using godot::Variant;
using godot::Object;
using godot::Callable;
using godot::Object;

// This doesn't seem to be exposed in GDExtension
/* void T5Node3D::_validate_property(PropertyInfo &p_property) const {
	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL(xr_server);

    auto t5_interface = Object::cast_to<TiltFiveXRInterface>(xr_server->find_interface("TiltFive").ptr());
	ERR_FAIL_NULL(t5_interface);

	if (p_property.name == String("tracker")) {
		PackedStringArray names = t5_interface->_get_suggested_tracker_names();
		String hint_string;
		for (const String &name : names) {
			hint_string += name + String(",");
		}
		p_property.hint_string = hint_string;
	} else if (p_property.name == String("pose")) {
		PackedStringArray names = t5_interface->_get_suggested_pose_names(tracker_name);
		String hint_string;
		for (const String &name : names) {
			hint_string += name + String(",");
		}
		p_property.hint_string = hint_string;
	}
} */


void T5Node3D::set_tracker(const StringName p_tracker_name) {
    if (tracker.is_valid() && tracker->get_tracker_name() == p_tracker_name) {
        // didn't change
        return;
    }

    // just in case
    _unbind_tracker();

    // copy the name
    tracker_name = p_tracker_name;
    pose_name = "default";

    // see if it's already available
    _bind_tracker();

    update_configuration_warnings();
    notify_property_list_changed();
}

StringName T5Node3D::get_tracker() const {
    return tracker_name;
}

void T5Node3D::set_pose_name(const StringName p_pose_name) {
    pose_name = p_pose_name;

    // Update pose if we are bound to a tracker with a valid pose
    Ref<XRPose> pose = get_pose();
    if (pose.is_valid()) {
        set_transform(pose->get_adjusted_transform());
    }
}

StringName T5Node3D::get_pose_name() const {
    return pose_name;
}

bool T5Node3D::get_is_active() const {
    if (tracker.is_null()) {
        return false;
    } else if (!tracker->has_pose(pose_name)) {
        return false;
    } else {
        return true;
    }
}

bool T5Node3D::get_has_tracking_data() const {
    if (tracker.is_null()) {
        return false;
    } else if (!tracker->has_pose(pose_name)) {
        return false;
    } else {
        return tracker->get_pose(pose_name)->get_has_tracking_data();
    }
}

Ref<XRPose> T5Node3D::get_pose() {
    if (tracker.is_valid()) {
        return tracker->get_pose(pose_name);
    } else {
        return Ref<XRPose>();
    }
}

PackedStringArray T5Node3D::get_configuration_warnings(PackedStringArray& warnings) const {

    if (is_visible() && is_inside_tree()) {
		// must be child node of T5Origin3D!
		T5Origin3D *origin = Object::cast_to<T5Origin3D>(get_parent());
		if (origin == nullptr) {
			warnings.push_back("T5Node3D must have an T5Origin3D node as its parent.");
		}

        if (tracker_name.is_empty()) {
            warnings.push_back("No tracker name is set.");
        }

        if (pose_name.is_empty()) {
            warnings.push_back("No pose is set.");
        }
    }

    return warnings;
}

T5Node3D::T5Node3D()  {
    XRServer *xr_server = XRServer::get_singleton();
    ERR_FAIL_NULL(xr_server);

    xr_server->connect("tracker_added", Callable(this, "_changed_tracker"));
    xr_server->connect("tracker_updated", Callable(this, "_changed_tracker"));
    xr_server->connect("tracker_removed", Callable(this, "_removed_tracker"));
 
	// check if our tracker already exists and if so, bind it...   
	_bind_tracker();
}

T5Node3D::~T5Node3D() {
	_unbind_tracker();

    XRServer *xr_server = XRServer::get_singleton();
    ERR_FAIL_NULL(xr_server);

    xr_server->disconnect("tracker_added", Callable(this, "_changed_tracker"));
    xr_server->disconnect("tracker_updated", Callable(this, "_changed_tracker"));
    xr_server->disconnect("tracker_removed", Callable(this, "_removed_tracker"));
}

void T5Node3D::_bind_methods() {
   	ClassDB::bind_method(D_METHOD("set_tracker", "tracker_name"), &T5Node3D::set_tracker);
	ClassDB::bind_method(D_METHOD("get_tracker"), &T5Node3D::get_tracker);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "tracker", godot::PROPERTY_HINT_ENUM_SUGGESTION), "set_tracker", "get_tracker");

	ClassDB::bind_method(D_METHOD("set_pose_name", "pose"), &T5Node3D::set_pose_name);
	ClassDB::bind_method(D_METHOD("get_pose_name"), &T5Node3D::get_pose_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "pose", godot::PROPERTY_HINT_ENUM_SUGGESTION), "set_pose_name", "get_pose_name");

	ClassDB::bind_method(D_METHOD("get_is_active"), &T5Node3D::get_is_active);
	ClassDB::bind_method(D_METHOD("get_has_tracking_data"), &T5Node3D::get_has_tracking_data);
	ClassDB::bind_method(D_METHOD("get_pose"), &T5Node3D::get_pose);

    ClassDB::bind_method(D_METHOD("_changed_tracker", "tracker_name", "tracker_type"), &T5Node3D::_changed_tracker);
    ClassDB::bind_method(D_METHOD("_removed_tracker", "tracker_name", "tracker_type"), &T5Node3D::_removed_tracker);
    ClassDB::bind_method(D_METHOD("_pose_changed", "pose"), &T5Node3D::_pose_changed);
};

void T5Node3D::_bind_tracker()  {
    ERR_FAIL_COND_MSG(tracker.is_valid(), "Unbind the current tracker first");

    XRServer *xr_server = XRServer::get_singleton();
    if (xr_server != nullptr) {
        tracker = xr_server->get_tracker(tracker_name);
        if (tracker.is_null()) {
            // It is possible and valid if the tracker isn't available (yet), in this case we just exit
            return;
        }

        tracker->connect("pose_changed", Callable(this, "_pose_changed"));

        Ref<XRPose> pose = get_pose();
        if (pose.is_valid()) {
            set_transform(pose->get_adjusted_transform());
        }
    }
}

void T5Node3D::_unbind_tracker() {
    if (tracker.is_valid()) {
        tracker->disconnect("pose_changed", Callable(this, "_pose_changed"));
        tracker.unref();
    }
}

void T5Node3D::_changed_tracker(const StringName p_tracker_name, int p_tracker_type) {
    if (tracker_name == p_tracker_name) {
        // just in case unref our current tracker
        _unbind_tracker();

        // get our new tracker
        _bind_tracker();
    }
}

void T5Node3D::_removed_tracker(const StringName p_tracker_name, int p_tracker_type) {
    if (tracker_name == p_tracker_name) {
        // unref our tracker, it's no longer available
        _unbind_tracker();
    }
}

void T5Node3D::_pose_changed(Object* p_obj) {
    auto pose = Object::cast_to<XRPose>(p_obj);
    if (pose && pose->get_name() == pose_name) {
        set_transform(pose->get_adjusted_transform());
    }
}

