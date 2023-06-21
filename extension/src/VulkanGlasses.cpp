#include <VulkanGlasses.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp> 
#include <Wand.h>
#include <ObjectRegistry.h>


using godot::RenderingServer;
using godot::RenderingDevice;
using godot::RDTextureFormat;
using godot::RDTextureView;
using godot::Vector3;

namespace GodotT5Integration {

VulkanGlasses::VulkanGlasses(std::string_view id) 
: GodotT5Glasses(id) {
    _swap_chain_textures.resize(g_swap_chain_length);
}

void VulkanGlasses::SwapChainTextures::allocate_textures(int width, int height) {
    deallocate_textures();

    auto render_server = RenderingServer::get_singleton();
    auto service = T5Integration::ObjectRegistry::service();
    auto render_device = render_server->get_rendering_device();

    Ref<RDTextureFormat> texture_format_render;
    texture_format_render.instantiate();

    texture_format_render->set_texture_type(RenderingDevice::TextureType::TEXTURE_TYPE_2D_ARRAY);
    texture_format_render->set_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_UNORM);
    texture_format_render->set_width(width);
    texture_format_render->set_height(height);
    texture_format_render->set_depth(1);
    texture_format_render->set_array_layers(2);
    texture_format_render->set_mipmaps(1);
    texture_format_render->set_samples(RenderingDevice::TextureSamples::TEXTURE_SAMPLES_1);
    texture_format_render->set_usage_bits(RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RenderingDevice::TEXTURE_USAGE_CAN_COPY_FROM_BIT);
	texture_format_render->add_shareable_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_UNORM);
	texture_format_render->add_shareable_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_SRGB);

    Ref<RDTextureView> texture_view;
    texture_view.instantiate();
    
    render_tex = render_device->texture_create(texture_format_render, texture_view);

    Ref<RDTextureFormat> texture_format_send;
    texture_format_send.instantiate();

    texture_format_send->set_texture_type(RenderingDevice::TextureType::TEXTURE_TYPE_2D);
    texture_format_send->set_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_UNORM);
    texture_format_send->set_width(width);
    texture_format_send->set_height(height);
    texture_format_send->set_depth(1);
    texture_format_send->set_array_layers(1);
    texture_format_send->set_mipmaps(1);
    texture_format_send->set_samples(RenderingDevice::TextureSamples::TEXTURE_SAMPLES_1);
    texture_format_send->set_usage_bits(RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_CAN_COPY_TO_BIT | RenderingDevice::TEXTURE_USAGE_CAN_COPY_FROM_BIT);
	texture_format_send->add_shareable_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_UNORM);
	texture_format_send->add_shareable_format(RenderingDevice::DataFormat::DATA_FORMAT_R8G8B8A8_SRGB);

    left_eye_tex = render_device->texture_create(texture_format_send, texture_view);
    right_eye_tex = render_device->texture_create(texture_format_send, texture_view);
}

void VulkanGlasses::SwapChainTextures::deallocate_textures() {
    auto render_server = RenderingServer::get_singleton();
    auto service = T5Integration::ObjectRegistry::service();
    auto render_device = render_server->get_rendering_device();

    if(render_tex.is_valid())
        render_device->free_rid(render_tex);
    if(left_eye_tex.is_valid())
        render_device->free_rid(left_eye_tex);
    if(right_eye_tex.is_valid())
        render_device->free_rid(right_eye_tex);
}

void VulkanGlasses::allocate_textures() {
    auto render_server = RenderingServer::get_singleton();
    auto render_device = render_server->get_rendering_device();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].allocate_textures(width, height);            
        set_swap_chain_texture_pair(
            i,
            render_device->texture_get_native_handle(_swap_chain_textures[i].left_eye_tex),
            render_device->texture_get_native_handle(_swap_chain_textures[i].right_eye_tex));
    }

    set_upside_down_texture(true);
}

void VulkanGlasses::deallocate_textures() {
    auto render_server = RenderingServer::get_singleton();

    int width, height;
    Glasses::get_display_size(width, height);
    for(int i = 0; i < _swap_chain_textures.size(); i++) {
        _swap_chain_textures[i].deallocate_textures();
        set_swap_chain_texture_pair(i, 0, 0);
    }
}

void VulkanGlasses::on_glasses_reserved() {
    GodotT5Glasses::on_glasses_reserved();
    allocate_textures();
}

void VulkanGlasses::on_glasses_released() {
    GodotT5Glasses::on_glasses_released();
    deallocate_textures();
}

void VulkanGlasses::on_glasses_dropped() {
    GodotT5Glasses::on_glasses_dropped();
    deallocate_textures();
}

void VulkanGlasses::on_send_frame(int swap_chain_idx) {
    auto service = T5Integration::ObjectRegistry::service();
    if(service->get_graphics_api() == kT5_GraphicsApi_Vulkan)
    {
        RenderingServer *rendering_server = RenderingServer::get_singleton();
        ERR_FAIL_NULL(rendering_server);
        RenderingDevice *rendering_device = rendering_server->get_rendering_device();
        ERR_FAIL_NULL(rendering_device);

        auto& textures = _swap_chain_textures[swap_chain_idx];

        auto size = get_render_size();

        rendering_device->texture_copy(
            textures.render_tex, // src
            textures.left_eye_tex, // dest
            Vector3(0,0,0), // src pos
            Vector3(0,0,0), // dest pos
            Vector3(size.x, size.y, 1), // size
            0, // src mipmap
            0, // dest mipmap
            0, // src layer
            0  // dest layer
            );

        rendering_device->texture_copy(
            textures.render_tex, // src
            textures.right_eye_tex, // dest
            Vector3(0,0,0), // src pos
            Vector3(0,0,0), // dest pos
            Vector3(size.x, size.y, 1), // size
            0, // src mipmap
            0, // dest mipmap
            1, // src layer
            0  // dest layer
            );
    }    
}
 
RID VulkanGlasses::get_color_texture() 
{ 
    int current_frame = get_current_frame_idx();
    return _swap_chain_textures[current_frame].render_tex;
}
} // GodotT5Integration