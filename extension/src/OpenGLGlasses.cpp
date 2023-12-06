#include <OpenGLGlasses.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp> 
#include <Wand.h>
#include <ObjectRegistry.h>


using godot::RenderingServer;
using godot::Image;
using godot::TypedArray;

namespace GodotT5Integration {

OpenGLGlasses::OpenGLGlasses(std::string_view id) 
: GodotT5Glasses(id) {
    _swap_chain_textures.resize(g_swap_chain_length);
}

void OpenGLGlasses::SwapChainTextures::allocate_textures(int width, int height) {
    auto render_server = RenderingServer::get_singleton();

	if(is_allocated)
    	deallocate_textures();

    Ref<Image> dummy_image = Image::create(width, height, false, godot::Image::FORMAT_RGBA8);
    godot::Color bg(0,0,0);
    dummy_image->fill(bg);

    TypedArray<Image> image_arr;
    image_arr.append(dummy_image);
    image_arr.append(dummy_image);

    render_tex.instantiate();
    render_tex->create_from_images(image_arr);

	is_allocated = true;
}

void OpenGLGlasses::SwapChainTextures::deallocate_textures() {
    render_tex.unref();
	is_allocated = false;
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

    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].deallocate_textures();
        set_swap_chain_texture_array(i, 0);
    }
}

void OpenGLGlasses::on_start_display() {
    allocate_textures();
}

void OpenGLGlasses::on_stop_display()  {
    deallocate_textures();
}

RID OpenGLGlasses::get_color_texture() 
{ 
    int current_frame = get_current_frame_idx();
    return _swap_chain_textures[current_frame].render_tex->get_rid();
}
} // GodotT5Integration