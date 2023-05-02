#include "TiltFiveXROrigin.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/viewport.hpp>


void TiltFiveXROrigin::_bind_methods() {
	// Methods.
	// ClassDB::bind_method(D_METHOD("simple_func"), &Example::simple_func);

	// Properties.

	// Signals.
	// ClassDB::bind_method(D_METHOD("emit_custom_signal", "name", "value"), &Example::emit_custom_signal);

	// Constants.
	// BIND_ENUM_CONSTANT(FIRST);
}


void TiltFiveXROrigin::_notification(int what) {
    
    switch (what) {
        case NOTIFICATION_READY: {
            auto xr_server = XRServer::get_singleton();
            _tilt_five_interface = xr_server->find_interface("TiltFive");
            ERR_FAIL_COND_MSG(_tilt_five_interface.is_null(), "Tilt Five interface not found");
            _viewport = Node::get_viewport()->get_viewport_rid();
            ERR_FAIL_COND_MSG(!_viewport.is_valid(), "Viewport is not initialized.");
            _tilt_five_interface->set_viewport_origin(_viewport, get_global_transform());
        } break;
        case NOTIFICATION_EXIT_TREE: {
            _viewport = RID();
        } break;
        case NOTIFICATION_TRANSFORM_CHANGED:
        case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
            if(_viewport != RID()) {
                _tilt_five_interface->set_viewport_origin(_viewport, get_global_transform());
            }
        } break;
        default:
            break;
    }

}
