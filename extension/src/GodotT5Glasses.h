#pragma once
#include <Glasses.h>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/ref.hpp>   
#include <godot_cpp/classes/xr_positional_tracker.hpp>  
#include <godot_cpp/classes/global_constants.hpp> 
#include <godot_cpp/classes/packed_data_container.hpp>

using godot::RID;
using godot::Ref;
using godot::XRPositionalTracker;
using godot::Vector2;
using godot::Transform3D;
using godot::StringName;
using godot::PackedFloat64Array;

using T5Integration::Glasses;

namespace GodotT5Integration {

    constexpr int g_swap_chain_length = 3;

    class GodotT5Service;

	class GodotT5Glasses : public Glasses {
        friend GodotT5Service;

        public:
		using Ptr = std::shared_ptr<GodotT5Glasses>;

        GodotT5Glasses(std::string_view id);
		virtual ~GodotT5Glasses() {  }

        bool is_in_use();
        bool is_reserved();

        Vector2 get_render_size();
        virtual Transform3D get_head_transform() = 0;
        virtual Transform3D get_eye_offset(Glasses::Eye eye) = 0;
        virtual Transform3D get_eye_transform(Glasses::Eye eye) = 0;
	    virtual PackedFloat64Array get_projection_for_eye(Glasses::Eye view, double aspect, double z_near, double z_far) = 0;

        virtual Transform3D get_wand_transform(size_t wand_num) = 0;
    
        virtual RID get_color_texture() = 0;

        StringName get_wand_tracker_name(size_t wand_idx);

        protected:
        virtual void on_tracking_updated() override;

        private:

        void add_tracker() ;
        void update_wand(size_t wand_idx);

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



