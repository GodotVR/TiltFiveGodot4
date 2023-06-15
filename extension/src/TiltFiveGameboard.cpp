#include "TiltFiveGameboard.h"

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using godot::ClassDB;
using godot::PropertyInfo;
using godot::D_METHOD;

void TiltFiveGameboard::_bind_methods() {
	// Methods.
	ClassDB::bind_method(D_METHOD("get_gameboard_scale"), &TiltFiveGameboard::get_gameboard_scale);
	ClassDB::bind_method(D_METHOD("set_gameboard_scale", "scale"), &TiltFiveGameboard::set_gameboard_scale);

	// Properties.
    ClassDB::add_property("TiltFiveGameboard", PropertyInfo(Variant::FLOAT, "gameboard_scale"), "set_gameboard_scale", "get_gameboard_scale");
}


real_t TiltFiveGameboard::get_gameboard_scale() {
    return _scale;
}

void TiltFiveGameboard::set_gameboard_scale(real_t scale) {
    _scale = scale;
}
