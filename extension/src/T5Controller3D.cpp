#include <T5Controller3D.h>
#include <godot_cpp/core/class_db.hpp>

using godot::ClassDB;
using godot::PropertyInfo;
using godot::D_METHOD;
using godot::Variant;
using godot::MethodInfo;
using godot::Callable;

#define DEF_SNAME(arg) StringName& sn_##arg() { \
	static StringName name = #arg; \
	return name; \
}

DEF_SNAME(button_pressed)
DEF_SNAME(button_released)
DEF_SNAME(input_float_changed)
DEF_SNAME(input_vector2_changed)

void T5Controller3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_button_pressed", "name"), &T5Controller3D::is_button_pressed);
	ClassDB::bind_method(D_METHOD("get_input", "name"), &T5Controller3D::get_input);
	ClassDB::bind_method(D_METHOD("get_float", "name"), &T5Controller3D::get_float);
	ClassDB::bind_method(D_METHOD("get_vector2", "name"), &T5Controller3D::get_vector2);
	
	ClassDB::bind_method(D_METHOD("_button_pressed", "name"), &T5Controller3D::_button_pressed);
	ClassDB::bind_method(D_METHOD("_button_released", "name"), &T5Controller3D::_button_released);
	ClassDB::bind_method(D_METHOD("_input_float_changed", "name", "value"), &T5Controller3D::_input_float_changed);
	ClassDB::bind_method(D_METHOD("_input_vector2_changed", "name", "value"), &T5Controller3D::_input_vector2_changed);

	ADD_SIGNAL(MethodInfo(sn_button_pressed(), PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo(sn_button_released(), PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo(sn_input_float_changed(), PropertyInfo(Variant::STRING, "name"), PropertyInfo(Variant::FLOAT, "value")));
	ADD_SIGNAL(MethodInfo(sn_input_vector2_changed(), PropertyInfo(Variant::STRING, "name"), PropertyInfo(Variant::VECTOR2, "value")));
};

void T5Controller3D::_bind_tracker() {
	T5Node3D::_bind_tracker();
	if (tracker.is_valid()) {
		// bind to input signals
		tracker->connect(sn_button_pressed(), Callable(this, "_button_pressed"));
		tracker->connect(sn_button_released(), Callable(this, "_button_released"));
		tracker->connect(sn_input_float_changed(), Callable(this, "_input_float_changed"));
		tracker->connect(sn_input_vector2_changed(), Callable(this, "_input_vector2_changed"));
	}
}

void T5Controller3D::_unbind_tracker() {
	if (tracker.is_valid()) {
		// unbind input signals
		tracker->disconnect(sn_button_pressed(), Callable(this, "_button_pressed"));
		tracker->disconnect(sn_button_released(), Callable(this, "_button_released"));
		tracker->disconnect(sn_input_float_changed(), Callable(this, "_input_float_changed"));
		tracker->disconnect(sn_input_vector2_changed(), Callable(this, "_input_vector2_changed"));
	}

	T5Node3D::_unbind_tracker();
}

void T5Controller3D::_button_pressed(const String &p_name) {
	// just pass it on...
	emit_signal(sn_button_pressed(), p_name);
}

void T5Controller3D::_button_released(const String &p_name) {
	// just pass it on...
	emit_signal(sn_button_released(), p_name);
}

void T5Controller3D::_input_float_changed(const String &p_name, float p_value) {
	// just pass it on...
	emit_signal(sn_input_float_changed(), p_name, p_value);
}

void T5Controller3D::_input_vector2_changed(const String &p_name, Vector2 p_value) {
	// just pass it on...
	emit_signal(sn_input_vector2_changed(), p_name, p_value);
}

bool T5Controller3D::is_button_pressed(const StringName &p_name) const {
	if (tracker.is_valid()) {
		// Inputs should already be of the correct type, our XR runtime handles conversions between raw input and the desired type
		bool pressed = tracker->get_input(p_name);
		return pressed;
	} else {
		return false;
	}
}

Variant T5Controller3D::get_input(const StringName &p_name) const {
	if (tracker.is_valid()) {
		return tracker->get_input(p_name);
	} else {
		return Variant();
	}
}

float T5Controller3D::get_float(const StringName &p_name) const {
	if (tracker.is_valid()) {
		// Inputs should already be of the correct type, our XR runtime handles conversions between raw input and the desired type, but just in case we convert
		Variant input = tracker->get_input(p_name);
		switch (input.get_type()) {
			case Variant::BOOL: {
				bool value = input;
				return value ? 1.0 : 0.0;
			} break;
			case Variant::FLOAT: {
				float value = input;
				return value;
			} break;
			default:
				return 0.0;
		};
	} else {
		return 0.0;
	}
}

Vector2 T5Controller3D::get_vector2(const StringName &p_name) const {
	if (tracker.is_valid()) {
		// Inputs should already be of the correct type, our XR runtime handles conversions between raw input and the desired type, but just in case we convert
		Variant input = tracker->get_input(p_name);
		switch (input.get_type()) {
			case Variant::BOOL: {
				bool value = input;
				return Vector2(value ? 1.0 : 0.0, 0.0);
			} break;
			case Variant::FLOAT: {
				float value = input;
				return Vector2(value, 0.0);
			} break;
			case Variant::VECTOR2: {
				Vector2 axis = input;
				return axis;
			}
			default:
				return Vector2();
		}
	} else {
		return Vector2();
	}
}
