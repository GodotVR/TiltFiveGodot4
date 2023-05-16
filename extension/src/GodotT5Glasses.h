#pragma once
#include <Glasses.h>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/ref.hpp>   
#include <godot_cpp/classes/xr_positional_tracker.hpp>  
#include <godot_cpp/classes/global_constants.hpp> 

using godot::RID;
using godot::Ref;
using godot::XRPositionalTracker;
using godot::Vector2;
using godot::Transform3D;
using godot::StringName;

using T5Integration::Glasses;

namespace GodotT5Integration {

    constexpr int g_swap_chain_length = 3;

    class GodotT5Service;

	class GodotT5Glasses : public Glasses {
        friend GodotT5Service;

        using Glasses::SwapChainFrame;

        struct SwapChainTextures {

            void allocate_textures(int width, int height);
            void deallocate_textures();

            RID render;
            RID left_eye;
            RID right_eye;
        };

        void allocate_textures();
        void deallocate_textures();

        virtual void on_glasses_reserved() override;
        virtual void on_glasses_released() override;
        virtual void on_glasses_dropped() override;
        virtual void on_tracking_updated() override;
        virtual void on_send_frame(int swap_chain_idx) override;

        public:
		using Ptr = std::shared_ptr<GodotT5Glasses>;

        GodotT5Glasses(std::string_view id);
		virtual ~GodotT5Glasses() {  }

        bool is_in_use();
        bool is_reserved();

        Vector2 get_display_size();
        Transform3D get_head_transform();
        Transform3D get_eye_offset(Glasses::Eye eye);
        Transform3D get_eye_transform(Glasses::Eye eye);

        Transform3D get_wand_transform(size_t wand_num);
    
        RID get_color_texture();

        StringName get_wand_tracker_name(size_t wand_idx);

        private:

        void add_tracker() ;
        void update_wand(size_t wand_idx);

    	std::vector<SwapChainTextures> _swap_chain_textures;
		std::vector<Ref<XRPositionalTracker>> _wand_trackers;
	};

    inline bool GodotT5Glasses::is_reserved() {
        return is_connected();
    }

    inline StringName GodotT5Glasses::get_wand_tracker_name(size_t wand_idx) {
        ERR_FAIL_INDEX_V(wand_idx, get_num_wands(), StringName());

        return _wand_trackers[wand_idx]->get_tracker_name();
    }

}



