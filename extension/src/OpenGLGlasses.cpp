#include <OpenGLGlasses.h>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp> 
#include <Wand.h>
#include <ObjectRegistry.h>


using godot::RenderingServer;
using godot::Image;
using godot::TypedArray;
using godot::Quaternion;
using godot::Vector3;
using godot::Projection;

namespace GodotT5Integration {

OpenGLGlasses::OpenGLGlasses(std::string_view id) 
: GodotT5Glasses(id) {
    _swap_chain_textures.resize(g_swap_chain_length);
}

void OpenGLGlasses::SwapChainTextures::allocate_textures(int width, int height) {
    auto render_server = RenderingServer::get_singleton();

    deallocate_textures();

    Ref<Image> dummy_image = Image::create(width, height, false, godot::Image::FORMAT_RGBA8);
    godot::Color bg(0,0,0);
    dummy_image->fill(bg);

    TypedArray<Image> image_arr;
    image_arr.append(dummy_image);
    image_arr.append(dummy_image);

    render_tex.instantiate();
    render_tex->create_from_images(image_arr);
}

void OpenGLGlasses::SwapChainTextures::deallocate_textures() {
    render_tex.unref();
}

void OpenGLGlasses::allocate_textures() {
    auto render_server = RenderingServer::get_singleton();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].allocate_textures(width, height);
        set_swap_chain_texture_array(i, render_server->texture_get_native_handle(_swap_chain_textures[i].render_tex->get_rid()));
    }
}

void OpenGLGlasses::deallocate_textures() {
    auto render_server = RenderingServer::get_singleton();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].deallocate_textures();
        set_swap_chain_texture_array(i, 0);
    }
}

void OpenGLGlasses::on_glasses_reserved() {
    allocate_textures();
}

void OpenGLGlasses::on_glasses_released() {
    deallocate_textures();
}

void OpenGLGlasses::on_glasses_dropped() {
    deallocate_textures();
}

Transform3D OpenGLGlasses::get_head_transform() {
	Quaternion orientation;
	Vector3 position;
	get_glasses_orientation(orientation.x, orientation.y, orientation.z, orientation.w);
	get_glasses_position(position.x, position.y, position.z);

	Transform3D headPose;
    headPose.set_origin(position);
	headPose.set_basis(orientation.inverse());
    headPose.rotate(Vector3(1,0,0), -Math_PI / 2.0f);

	return headPose;
}

Transform3D OpenGLGlasses::get_eye_offset(Glasses::Eye eye) {
	float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
	auto ipd = get_ipd();
    Transform3D eye_pose;
    eye_pose.set_origin(Vector3(dir * ipd / 2.0f, 0, 0));

	return eye_pose;
}

Transform3D OpenGLGlasses::get_eye_transform(Glasses::Eye eye) {
	return get_eye_offset(eye) * get_head_transform();
}

Transform3D OpenGLGlasses::get_wand_transform(size_t wand_num) {
	Quaternion orientation;
	Vector3 position;
	get_wand_position(wand_num, position.x, position.y, position.z);
	get_wand_orientation(wand_num, orientation.x, orientation.y, orientation.z, orientation.w);

    position = Vector3(position.x, position.z, -position.y);
    orientation = Quaternion(orientation.x, orientation.z, -orientation.y, orientation.w);
    orientation = orientation.inverse();

	Transform3D wandPose;
    wandPose.set_origin(position);
	wandPose.set_basis(orientation * Quaternion(Vector3(1,0,0), Math_PI / 2.0f));

	return wandPose;
}

PackedFloat64Array OpenGLGlasses::get_projection_for_eye(Glasses::Eye view, double aspect, double z_near, double z_far) {
	PackedFloat64Array arr;
	arr.resize(16); // 4x4 matrix

    Projection cm;
    cm.set_perspective(get_fov(), aspect, z_near, z_far);

    real_t *m = (real_t *)cm.columns;
	for (int i = 0; i < 16; i++) {
		arr[i] = m[i];
	}

    return arr;    
}
 
RID OpenGLGlasses::get_color_texture() 
{ 
    int current_frame = get_current_frame_idx();
    return _swap_chain_textures[current_frame].render_tex->get_rid();
}
} // GodotT5Integration