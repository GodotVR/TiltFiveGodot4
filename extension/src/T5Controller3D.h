#ifndef T5_CONTROLLER_3D_H
#define T5_CONTROLLER_3D_H
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
#include <T5Node3D.h>

using godot::Variant;
using godot::String;
using godot::StringName;
using godot::Vector2;

class T5Controller3D : public T5Node3D {
	GDCLASS(T5Controller3D, T5Node3D);

public:
	bool is_button_pressed(const StringName &p_name) const;
	Variant get_input(const StringName &p_name) const;
	float get_float(const StringName &p_name) const;
	Vector2 get_vector2(const StringName &p_name) const;

	void trigger_haptic_pulse(float amplitude, int duration);

	T5Controller3D() {}
	~T5Controller3D() {}

protected:
	static void _bind_methods();

	virtual void _bind_tracker() override;
	virtual void _unbind_tracker() override;

	void _button_pressed(const String &p_name);
	void _button_released(const String &p_name);
	void _input_float_changed(const String &p_name, float p_value);
	void _input_vector2_changed(const String &p_name, Vector2 p_value);

private:

};

#endif // T5_CONTROLLER_3D_H