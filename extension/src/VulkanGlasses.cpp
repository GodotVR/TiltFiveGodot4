#include <ObjectRegistry.h>
#include <VulkanGlasses.h>
#include <Wand.h>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/core/error_macros.hpp>

using godot::RDTextureFormat;
using godot::RDTextureView;
using godot::RenderingDevice;
using godot::RenderingServer;
using godot::Vector3;

namespace GodotT5Integration {

VulkanGlasses::VulkanGlasses(std::string_view id) :
		GodotT5Glasses(id) {
	_swap_chain_textures.resize(g_swap_chain_length);
}

void VulkanGlasses::SwapChainTextures::allocate_textures(int width, int height) {
	if (is_allocated)
		deallocate_textures();

	auto render_server = RenderingServer::get_singleton();
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

	left_eye_tex = render_device->texture_create_shared_from_slice(texture_view, render_tex, 0, 0);
	right_eye_tex = render_device->texture_create_shared_from_slice(texture_view, render_tex, 1, 0);

	left_tex_handle = render_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_IMAGE_VIEW, left_eye_tex, 0);
	right_tex_handle = render_device->get_driver_resource(RenderingDevice::DRIVER_RESOURCE_VULKAN_IMAGE_VIEW, right_eye_tex, 0);

	is_allocated = true;
}

void VulkanGlasses::SwapChainTextures::deallocate_textures() {
	if (!is_allocated)
		return;

	auto render_server = RenderingServer::get_singleton();
	auto render_device = render_server->get_rendering_device();

	if (right_eye_tex.is_valid())
		render_device->free_rid(right_eye_tex);
	if (left_eye_tex.is_valid())
		render_device->free_rid(left_eye_tex);
	if (render_tex.is_valid())
		render_device->free_rid(render_tex);

	left_tex_handle = 0;
	right_tex_handle = 0;

	is_allocated = false;
}

void VulkanGlasses::allocate_textures() {
	auto render_server = RenderingServer::get_singleton();
	auto render_device = render_server->get_rendering_device();

	int width, height;
	Glasses::get_display_size(width, height);
	for (int i = 0; i < _swap_chain_textures.size(); i++) {
		_swap_chain_textures[i].allocate_textures(width, height);
		set_swap_chain_texture_pair(
				i,
				reinterpret_cast<intptr_t>(&_swap_chain_textures[i].left_tex_handle),
				reinterpret_cast<intptr_t>(&_swap_chain_textures[i].right_tex_handle));
	}

	set_upside_down_texture(true);
}

void VulkanGlasses::deallocate_textures() {
	auto render_server = RenderingServer::get_singleton();

	for (int i = 0; i < _swap_chain_textures.size(); i++) {
		_swap_chain_textures[i].deallocate_textures();
		set_swap_chain_texture_pair(i, 0, 0);
	}
}

void VulkanGlasses::on_allocate_render_textures() {
	allocate_textures();
}

void VulkanGlasses::on_deallocate_render_textures() {
	deallocate_textures();
}

RID VulkanGlasses::get_color_texture() {
	int current_frame = get_current_frame_idx();
	return _swap_chain_textures[current_frame].render_tex;
}
} //namespace GodotT5Integration