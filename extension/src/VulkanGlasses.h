#pragma once
#include <GodotT5Glasses.h>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>
#include <godot_cpp/classes/image_texture.hpp>

using godot::RID;
using godot::Transform3D;
using godot::Texture2DArray;

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
        
        virtual Transform3D get_head_transform() override;
        virtual Transform3D get_eye_offset(Glasses::Eye eye) override;
        virtual Transform3D get_eye_transform(Glasses::Eye eye) override;
	    virtual PackedFloat64Array get_projection_for_eye(Glasses::Eye view, double aspect, double z_near, double z_far) override;

        virtual Transform3D get_wand_transform(size_t wand_num) override;
    
        virtual RID get_color_texture() override;

        private:

    	std::vector<SwapChainTextures> _swap_chain_textures;
    };
}
