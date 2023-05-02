#ifndef TILT_FIVE_XR_ORIGIN_H
#define TILT_FIVE_XR_ORIGIN_H

#include "TiltFiveXRInterface.h"

#include <godot_cpp/classes/xr_origin3d.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <GodotT5Service.h>
#include <GodotT5Glasses.h>

using godot::XROrigin3D;
using godot::XRServer;
using godot::String;
using godot::StringName;
using godot::Vector2;
using godot::Transform3D;
using godot::PackedFloat64Array;
using godot::Rect2;
using godot::RID;
using godot::PackedStringArray;
using GodotT5Integration::GodotT5Service;
using GodotT5Integration::GodotT5Glasses;
using T5Integration::GlassesEvent;

class TiltFiveXROrigin : public XROrigin3D {
	GDCLASS(TiltFiveXROrigin, XROrigin3D);

public:

    virtual void _notification(int p_what) ;


protected:
	static void _bind_methods();

private:
    RID _viewport;
    Ref<TiltFiveXRInterface> _tilt_five_interface;
};

#endif // TILT_FIVE_XR_ORIGIN_H