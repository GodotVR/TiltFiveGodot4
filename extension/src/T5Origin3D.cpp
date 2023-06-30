#include <T5Origin3D.h>
#include <godot_cpp/core/class_db.hpp>

using godot::ClassDB;
using godot::PropertyInfo;
using godot::D_METHOD;
using godot::Variant;

void T5Origin3D::_bind_methods() {
	// Methods.
	ClassDB::bind_method(D_METHOD("get_gameboard_scale"), &T5Origin3D::get_gameboard_scale);
	ClassDB::bind_method(D_METHOD("set_gameboard_scale", "scale"), &T5Origin3D::set_gameboard_scale);

	// Properties.
    ClassDB::add_property("T5Origin3D", PropertyInfo(Variant::FLOAT, "gameboard_scale"), "set_gameboard_scale", "get_gameboard_scale");
}


real_t T5Origin3D::get_gameboard_scale() {
    return _scale;
}

void T5Origin3D::set_gameboard_scale(real_t scale) {
    _scale = scale;
}
