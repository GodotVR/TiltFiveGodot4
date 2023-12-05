#pragma once
#include <GodotT5Glasses.h>
#include <godot_cpp/variant/rid.hpp>

using godot::RID;

namespace GodotT5Integration {

    class VulkanGlasses : public GodotT5Glasses {

        struct SwapChainTextures {
            void allocate_textures(int width, int height);
            void deallocate_textures();

			bool is_allocated = false;
            RID render_tex;
            RID left_eye_tex;
            RID right_eye_tex;

            intptr_t left_tex_handle;
            intptr_t right_tex_handle;
        };

        public:

        VulkanGlasses(std::string_view id);
    
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
