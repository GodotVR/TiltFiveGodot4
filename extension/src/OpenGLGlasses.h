#pragma once
#include <GodotT5Glasses.h>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>
#include <godot_cpp/variant/rid.hpp>

using godot::RID;
using godot::Texture2DArray;
using godot::Transform3D;

namespace GodotT5Integration {

class OpenGLGlasses : public GodotT5Glasses {
	struct SwapChainTextures {
		void allocate_textures(int width, int height);
		void deallocate_textures();

		bool is_allocated = false;
		Ref<Texture2DArray> render_tex;
	};

public:
	OpenGLGlasses(std::string_view id);

	virtual RID get_color_texture() override;

private:
	void allocate_textures();
	void deallocate_textures();

	virtual void on_allocate_render_textures() override;
	virtual void on_deallocate_render_textures() override;

private:
	std::vector<SwapChainTextures> _swap_chain_textures;
};
} //namespace GodotT5Integration
