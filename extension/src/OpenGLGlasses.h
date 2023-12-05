#pragma once
#include <GodotT5Glasses.h>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>
#include <godot_cpp/classes/image_texture.hpp>

using godot::RID;
using godot::Transform3D;
using godot::Texture2DArray;

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

		virtual void on_start_display() override;
		virtual void on_stop_display() override;

        private:

    	std::vector<SwapChainTextures> _swap_chain_textures;
    };
}

