#ifndef T5_GAMEBOARD_H
#define T5_GAMEBOARD_H

#include <TiltFiveXRInterface.h>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/packed_data_container.hpp>
#include <godot_cpp/classes/xr_pose.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>

using godot::Ref;
using godot::String;
using godot::StringName;
using godot::VisualInstance3D;

class T5Gameboard : public Node3D {
	GDCLASS(T5Gameboard, Node3D);

public:

	void _ready() override;

	void set_content_scale(float scale);
	float get_content_scale() const;

	void set_gameboard_type(String gbtype);
	String get_gameboard_type() const;

	void set_show_at_runtime(bool show);
	bool get_show_at_runtime() const;

	void set_layer_mask(uint32_t p_mask);
	uint32_t get_layer_mask() const;

	void set_layer_mask_value(int p_layer_number, bool p_enable);
	bool get_layer_mask_value(int p_layer_number) const;


    T5Gameboard();
    ~T5Gameboard();
    
protected:

	static void _bind_methods();

private:
    Node3D* add_scene(String path);
    void set_board_state();

	float content_scale = 1.0;
	TiltFiveXRInterface::GameBoardType gameboard_type = TiltFiveXRInterface::GameBoardType::LE_GAMEBOARD;
    bool show_at_runtime = false;
	uint32_t layers = 1;

    Node3D* le_node = nullptr;
    Node3D* xe_node = nullptr;
    Node3D* raised_xe_node = nullptr;
};




#endif // T5_GAMEBOARD_H