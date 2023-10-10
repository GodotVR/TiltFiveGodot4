#pragma once
#include <GodotT5Glasses.h>
#include <godot_cpp/variant/rid.hpp>

using godot::RID;

namespace GodotT5Integration {

    class VulkanGlasses : public GodotT5Glasses {

        struct SwapChainTextures {
            void allocate_textures(int width, int height);
            void deallocate_textures();

            RID render_tex;
            RID left_eye_tex;
            RID right_eye_tex;
        };

        public:

        VulkanGlasses(std::string_view id);
    
        virtual void on_post_draw() override;
        virtual RID get_color_texture() override;

        private:

        void allocate_textures();
        void deallocate_textures();

        virtual void on_glasses_reserved() override;
        virtual void on_glasses_released() override;
        virtual void on_glasses_dropped() override;


        private:

    	std::vector<SwapChainTextures> _swap_chain_textures;
    };
}
