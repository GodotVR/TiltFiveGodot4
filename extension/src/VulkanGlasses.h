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

        void allocate_textures();
        void deallocate_textures();

        virtual void on_glasses_reserved() override;
        virtual void on_glasses_released() override;
        virtual void on_glasses_dropped() override;
        virtual void on_send_frame(int swap_chain_idx) override;

        public:

        VulkanGlasses(std::string_view id);
    
        virtual RID get_color_texture() override;

        private:

    	std::vector<SwapChainTextures> _swap_chain_textures;
    };
}
