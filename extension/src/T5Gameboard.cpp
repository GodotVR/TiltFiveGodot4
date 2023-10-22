#include <T5Gameboard.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/engine.hpp>


using godot::ClassDB;
using godot::D_METHOD;
using godot::PropertyInfo;
using godot::Variant;
using godot::Callable;
using godot::Object;
using godot::UtilityFunctions;
using godot::ResourceLoader;
using godot::PackedScene;
using godot::SceneTree;
using godot::Vector3;
using godot::Engine;

const char le_name[] = "LE";
const char xe_name[] = "XE";
const char raised_xe_name[] = "Raised XE";

void T5Gameboard::set_content_scale(float scale) {
    content_scale = scale;
    set_board_state();
}

float T5Gameboard::get_content_scale() const{
    return content_scale;
}

void T5Gameboard::set_gameboard_type(String gb_type) {
    if(gb_type == le_name) {
        gameboard_type = TiltFiveXRInterface::LE_GAMEBOARD;
    } 
    else if(gb_type == xe_name) {
        gameboard_type = TiltFiveXRInterface::XE_GAMEBOARD;
    } 
    else if(gb_type == raised_xe_name) {
        gameboard_type = TiltFiveXRInterface::XE_RAISED_GAMEBOARD;
    } 
    else {
        gameboard_type = TiltFiveXRInterface::NO_GAMEBOARD_SET;
    }
    set_board_state();

}

String T5Gameboard::get_gameboard_type() const{
    switch (gameboard_type)
    {
    case TiltFiveXRInterface::LE_GAMEBOARD:
        return le_name;
    case TiltFiveXRInterface::XE_GAMEBOARD:
        return xe_name;
    case TiltFiveXRInterface::XE_RAISED_GAMEBOARD:
        return raised_xe_name;
    default:
        break;
    }
    return "Unknown";
}

void T5Gameboard::set_show_at_runtime(bool show) {
    show_at_runtime = show;
    set_board_state();
}

bool T5Gameboard::get_show_at_runtime() const {
    return show_at_runtime;
}

void T5Gameboard::set_layer_mask(uint32_t p_mask) {
	layers = p_mask;
    set_board_state();
}

uint32_t T5Gameboard::get_layer_mask() const {
	return layers;
}

void T5Gameboard::set_layer_mask_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1, "Render layer number must be between 1 and 20 inclusive.");
	ERR_FAIL_COND_MSG(p_layer_number > 20, "Render layer number must be between 1 and 20 inclusive.");
	uint32_t mask = get_layer_mask();
	if (p_value) {
		mask |= 1 << (p_layer_number - 1);
	} else {
		mask &= ~(1 << (p_layer_number - 1));
	}
	set_layer_mask(mask);
}

bool T5Gameboard::get_layer_mask_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Render layer number must be between 1 and 20 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 20, false, "Render layer number must be between 1 and 20 inclusive.");
	return layers & (1 << (p_layer_number - 1));
}


T5Gameboard::T5Gameboard(){

}

T5Gameboard::~T5Gameboard(){

}

Node3D* T5Gameboard::add_scene(String path) {
    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path, "PackedScene");
    ERR_FAIL_COND_V_MSG(scene.is_null(), nullptr, godot::vformat("Failed to load: %s", path));

    auto node = scene->instantiate();
    auto node_3d = Object::cast_to<Node3D>(node);
    if(!node_3d) {
        node->queue_free();
        ERR_FAIL_V_MSG(nullptr, godot::vformat("Not Node3D: %s", path));
    }
    if(node_3d->get_child_count() == 0 || node->get_child(0)->get_class() != "MeshInstance3D") {
        node->queue_free();
        ERR_FAIL_V_MSG(nullptr, godot::vformat("Not Node3D and MeshInstance3D: %s", path));
    }

    add_child(node_3d);
    return node_3d;
}

void T5Gameboard::set_board_state() {

    bool show_board = show_at_runtime || Engine::get_singleton()->is_editor_hint();
    if(le_node) {
        le_node->set_scale(Vector3(content_scale, content_scale, content_scale));
        le_node->set_visible(gameboard_type == TiltFiveXRInterface::LE_GAMEBOARD && show_board);
        auto mesh_inst = Object::cast_to<VisualInstance3D>(le_node->get_child(0));
        mesh_inst->set_layer_mask(layers);
    }
    if(xe_node) {
        xe_node->set_scale(Vector3(content_scale, content_scale, content_scale));
        xe_node->set_visible(gameboard_type == TiltFiveXRInterface::XE_GAMEBOARD && show_board);
        auto mesh_inst = Object::cast_to<VisualInstance3D>(le_node->get_child(0));
        mesh_inst->set_layer_mask(layers);
    }
    if(raised_xe_node) {
        raised_xe_node->set_scale(Vector3(content_scale, content_scale, content_scale));
        raised_xe_node->set_visible(gameboard_type == TiltFiveXRInterface::XE_RAISED_GAMEBOARD && show_board);
        auto mesh_inst = Object::cast_to<VisualInstance3D>(le_node->get_child(0));
        mesh_inst->set_layer_mask(layers);
    }
}

void T5Gameboard::_ready() {
    le_node = add_scene("res://addons/tiltfive/assets/T5_border.glb");
    xe_node = add_scene("res://addons/tiltfive/assets/T5_border_XE.glb");
    raised_xe_node = add_scene("res://addons/tiltfive/assets/T5_border_XE_raised.glb");
    set_board_state();
}

void T5Gameboard::_bind_methods() {
   	ClassDB::bind_method(D_METHOD("set_content_scale", "scale"), &T5Gameboard::set_content_scale);
	ClassDB::bind_method(D_METHOD("get_content_scale"), &T5Gameboard::get_content_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "content_scale", godot::PROPERTY_HINT_RANGE, "0.001, 1000.0, or_greater, hide_slider"), "set_content_scale", "get_content_scale");

	ClassDB::bind_method(D_METHOD("set_gameboard_type", "gameboard_type"), &T5Gameboard::set_gameboard_type);
	ClassDB::bind_method(D_METHOD("get_gameboard_type"), &T5Gameboard::get_gameboard_type);

    auto hint = godot::vformat("%s,%s,%s", le_name, xe_name, raised_xe_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "gameboard_type", godot::PROPERTY_HINT_ENUM, hint), "set_gameboard_type", "get_gameboard_type");

   	ClassDB::bind_method(D_METHOD("set_show_at_runtime", "show"), &T5Gameboard::set_show_at_runtime);
	ClassDB::bind_method(D_METHOD("get_show_at_runtime"), &T5Gameboard::get_show_at_runtime);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_at_runtime", godot::PROPERTY_HINT_NONE), "set_show_at_runtime", "get_show_at_runtime");

   	ClassDB::bind_method(D_METHOD("set_layer_mask", "mask"), &T5Gameboard::set_layer_mask);
	ClassDB::bind_method(D_METHOD("get_layer_mask"), &T5Gameboard::get_layer_mask);
   	ADD_PROPERTY(PropertyInfo(Variant::INT, "layers", godot::PROPERTY_HINT_LAYERS_3D_RENDER), "set_layer_mask", "get_layer_mask");

	ClassDB::bind_method(D_METHOD("set_layer_mask_value", "layer_number", "value"), &T5Gameboard::set_layer_mask_value);
	ClassDB::bind_method(D_METHOD("get_layer_mask_value", "layer_number"), &T5Gameboard::get_layer_mask_value);

};

    