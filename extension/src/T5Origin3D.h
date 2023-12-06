#ifndef T5_ORIGIN_3D_H
#define T5_ORIGIN_3D_H
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
#include <godot_cpp/classes/node3d.hpp>

using godot::Node3D;

class T5Origin3D : public Node3D {
	GDCLASS(T5Origin3D, Node3D);

public:
	real_t get_gameboard_scale();
	void set_gameboard_scale(real_t scale);

protected:
	static void _bind_methods();

private:
	float _scale = 1.0;
};

#endif // T5_ORIGIN_3D_H