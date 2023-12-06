#pragma once
#include <Glasses.h>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/packed_data_container.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/xr_positional_tracker.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

using godot::PackedFloat64Array;
using godot::Ref;
using godot::RID;
using godot::StringName;
using godot::Transform3D;
using godot::Vector2;
using godot::Vector3;
using godot::XRPositionalTracker;

using T5Integration::Glasses;

namespace GodotT5Integration {

constexpr int g_swap_chain_length = 3;
constexpr float g_trigger_hysteresis_range = 0.002; // Sort of arbitrary assume 8 bit DAC +/-(1/256)/2

class GodotT5Service;

class GodotT5Glasses : public Glasses {
	friend GodotT5Service;

public:
	using Ptr = std::shared_ptr<GodotT5Glasses>;

	GodotT5Glasses(std::string_view id);
	virtual ~GodotT5Glasses() {}

	bool is_in_use();
	bool is_reserved();

	Vector2 get_render_size();
	virtual Transform3D get_head_transform(Vector3 eye_offset = Vector3());
	virtual Vector3 get_eye_offset(Glasses::Eye eye);
	virtual Transform3D get_eye_transform(Glasses::Eye eye);
	virtual PackedFloat64Array get_projection_for_eye(Glasses::Eye view, double aspect, double z_near, double z_far);

	virtual Transform3D get_wand_transform(int wand_num);

	virtual RID get_color_texture() = 0;

	StringName get_wand_tracker_name(int wand_idx);

	bool get_tracker_association(StringName tracker_name, int& out_wand_idx);

	void set_trigger_click_threshold(float threshold);

protected:
	virtual void on_glasses_reserved() override;
	virtual void on_glasses_released() override;
	virtual void on_glasses_dropped() override;

	virtual void on_tracking_updated() override;

private:
	void add_tracker();
	void update_wand(int wand_idx);

	Ref<XRPositionalTracker> _head;
	std::vector<Ref<XRPositionalTracker>> _wand_trackers;

	float _trigger_click_threshold;
};

inline bool GodotT5Glasses::is_reserved() {
	return is_connected();
}

inline StringName GodotT5Glasses::get_wand_tracker_name(int wand_idx) {
	ERR_FAIL_INDEX_V(wand_idx, get_num_wands(), StringName());

	return _wand_trackers[wand_idx]->get_tracker_name();
}

inline void GodotT5Glasses::set_trigger_click_threshold(float threshold) {
	_trigger_click_threshold = threshold;
}

} //namespace GodotT5Integration
