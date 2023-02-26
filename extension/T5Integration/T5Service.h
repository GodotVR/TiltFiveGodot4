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

	virtual bool should_glasses_be_connected(Glasses::Ptr glasses) { return true; }
	virtual void glasses_were_connected(Glasses::Ptr glasses) {}
	virtual void glasses_were_disconnected(Glasses::Ptr glasses) {}

	virtual void connection_updated() {}
	virtual void tracking_updated() {}

public:
	using Ptr = std::shared_ptr<T5Service>;

	T5Service();
	virtual ~T5Service();

	bool start_service(const char* applicationID, const char* applicationVersion);
	void stop_service();
	bool is_service_started();

	void connect_glasses(int glasses_num, std::string display_name);
	void disconnect_glasses(int glasses_num);

    void set_upside_down_texture(int glasses_num, bool is_upside_down);
	
	size_t get_glasses_count() { return _glasses_list.size(); }

	const std::vector<GlassesEvent> get_events();
	const std::string get_glasses_id(size_t glasses_idx);
	const std::string get_glasses_name(size_t glasses_idx);

	void update_connection();
	void update_tracking();


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

	std::vector<GlassesEvent> _events;

	Scheduler::Ptr _scheduler;
};


inline const std::string T5Service::get_glasses_id(size_t glasses_idx) {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_id() : std::string();
}

inline const std::string T5Service::get_glasses_name(size_t glasses_idx) {
	return glasses_idx < _glasses_list.size() ? _glasses_list[glasses_idx]->get_name() : std::string();
}

inline void T5Service::set_upside_down_texture(int glasses_idx, bool is_upside_down) {
	if(glasses_idx < _glasses_list.size()) _glasses_list[glasses_idx]->set_upside_down_texture(is_upside_down);
}

}
