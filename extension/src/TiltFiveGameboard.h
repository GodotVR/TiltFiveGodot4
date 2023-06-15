#ifndef TILT_FIVE_XR_ORIGIN_H
#define TILT_FIVE_XR_ORIGIN_H

#include "TiltFiveXRInterface.h"

#include <godot_cpp/classes/xr_origin3d.hpp>



using godot::XROrigin3D;

class TiltFiveGameboard : public XROrigin3D {
	GDCLASS(TiltFiveGameboard, XROrigin3D);

public:

    real_t get_gameboard_scale();
    void set_gameboard_scale(real_t scale);

protected:
	static void _bind_methods();

private:
    float _scale = 1.0;

};

#endif // TILT_FIVE_XR_ORIGIN_H