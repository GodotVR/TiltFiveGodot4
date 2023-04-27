#pragma once
#include <string>
#include <vector>
#include <memory>
#include <TaskSystem.h>
#include <StateFlags.h>
#include <Glasses.h>

namespace T5Integration {

using TaskSystem::CotaskPtr;
using TaskSystem::Scheduler;
using TaskSystem::run_in_foreground;
using TaskSystem::task_sleep;

extern std::mutex g_t5_exclusivity_group_1;
//extern std::mutex g_t5_exclusivity_group_2;

class T5Service {
protected:

	CotaskPtr query_ndk_version();
	CotaskPtr query_glasses_list();

	virtual std::unique_ptr<Glasses> create_glasses(const std::string_view id);

	virtual bool should_glasses_be_reserved(int glasses_idx) { return true; }

	virtual void connection_updated() {}
	virtual void tracking_updated() {}

public:
	using Ptr = std::shared_ptr<T5Service>;

	T5Service();
	virtual ~T5Service();

	bool start_service(const std::string_view applicationID, std::string_view applicationVersion);
	void stop_service();
	bool is_service_started();

	void reserve_glasses(int glasses_num, const std::string_view display_name);
	void release_glasses(int glasses_num);
	[[deprecated]] void connect_glasses(int glasses_num, const std::string display_name);
	[[deprecated]] void disconnect_glasses(int glasses_num);

    void set_upside_down_texture(int glasses_num, bool is_upside_down);
	
	size_t get_glasses_count() { return _glasses_list.size(); }
	std::optional<int> find_glasses_idx(const std::string_view glasses_id);

	const std::string get_glasses_id(size_t glasses_idx) const;
	const std::string get_glasses_name(size_t glasses_idx) const;

	void update_connection();
	void update_tracking();
	void get_events(std::vector<GlassesEvent>& out_events);


protected:

	std::string _application_id;
	std::string _application_version;

	T5_Context _context = nullptr;
	std::string _ndk_version;
	std::vector<Glasses::Ptr> _glasses_list;

	bool _is_started = false;

	std::chrono::milliseconds _poll_rate_for_monitoring = 2s;
	std::chrono::milliseconds _poll_rate_for_retry = 100ms;

	T5_Result _last_error;

	Scheduler::Ptr _scheduler;
};


inline const std::string T5Service::get_glasses_id(size_t glasses_idx) const {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_id() : std::string();
}

inline const std::string T5Service::get_glasses_name(size_t glasses_idx) const {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_name() : std::string();
}

inline void T5Service::set_upside_down_texture(int glasses_idx, bool is_upside_down) {
	if(glasses_idx < _glasses_list.size()) _glasses_list[glasses_idx]->set_upside_down_texture(is_upside_down);
}

}
