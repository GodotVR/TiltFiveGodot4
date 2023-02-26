#include <algorithm>  
#include <T5Service.h>
#include <ObjectRegistry.h>
#include <Logging.h>

namespace T5Integration {

std::mutex g_t5_exclusivity_group_1;
std::mutex g_t5_exclusivity_group_2;

T5Service::T5Service() {
	_scheduler = ObjectRegistry::scheduler();
}

T5Service::~T5Service() {

	stop_service();
}

bool T5Service::start_service(const char* application_id, const char* application_version) {
	if(_is_started) return true;

	T5_ClientInfo clientInfo;
	clientInfo.applicationId = application_id;
	clientInfo.applicationVersion = application_version;
	
	auto result = t5CreateContext(&_context, &clientInfo, nullptr);

	if(result != T5_SUCCESS) {
		LOG_T5_ERROR(result);
		return false;
	}

	_is_started = true;

	_scheduler->start();

	_scheduler->add_task(query_ndk_version());
	_scheduler->add_task(query_glasses_list());

	return true;
}

void T5Service::stop_service() {
	if(!_is_started) return;
	_is_started = false;

	_scheduler->stop();

	for(Glasses::Ptr glasses : _glasses_list) {
		glasses->disconnect();
		glasses_were_disconnected(glasses);
		glasses->destroy_handle();
	}
	_glasses_list.clear();
	if(_context)
	{
		std::lock_guard lock(g_t5_exclusivity_group_1);
		t5DestroyContext(&_context);
	}
	_context = nullptr;
}

const std::vector<GlassesEvent> T5Service::get_events() {
	_events.clear();
	for(int i = 0; i < _glasses_list.size(); ++i) {
		auto changes = _glasses_list[i]->get_changed_state();
		auto current_state = _glasses_list[i]->get_current_state();
		if((changes & GlassesState::CREATED) == GlassesState::CREATED) {
			_events.push_back(GlassesEvent(i, current_state & GlassesState::CREATED ? GlassesEvent::E_ADDED : GlassesEvent::E_LOST));
		}
		if((changes & GlassesState::UNAVAILABLE) == GlassesState::UNAVAILABLE) {
			_events.push_back(GlassesEvent(i, current_state & GlassesState::UNAVAILABLE ? GlassesEvent::E_UNAVAILABLE : GlassesEvent::E_AVAILABLE));
		}
		if((changes & GlassesState::CONNECTED) == GlassesState::CONNECTED) {
			if(current_state & GlassesState::CONNECTED) {
				glasses_were_connected(_glasses_list[i]);
				_events.push_back(GlassesEvent(i, GlassesEvent::E_CONNECTED)); 
			}
			else {
				glasses_were_disconnected(_glasses_list[i]);
				_events.push_back(GlassesEvent(i, GlassesEvent::E_DISCONNECTED));

			}
		}
		if((changes & GlassesState::TRACKING) == GlassesState::TRACKING) {
			_events.push_back(GlassesEvent(i, current_state & GlassesState::TRACKING ? GlassesEvent::E_TRACKING : GlassesEvent::E_NOT_TRACKING));
		}
		if((changes & GlassesState::ERROR) == GlassesState::ERROR) {
			// There is currently no way to recover from the error state
			_events.push_back(GlassesEvent(i, current_state & GlassesState::ERROR ? GlassesEvent::E_STOPPED_ON_ERROR : GlassesEvent::E_AVAILABLE));
		}
	}
	return _events;
}

CotaskPtr T5Service::query_ndk_version() {
	std::vector<char> buffer;
	size_t buffer_size = 32;
	buffer.resize(buffer_size);

	T5_Result result;
	for(int tries = 0; tries < 10; ++tries) {
		size_t buffer_size = buffer.size();
		{
			std::lock_guard lock(g_t5_exclusivity_group_1);
			result = t5GetSystemUtf8Param(_context, kT5_ParamSys_UTF8_Service_Version, buffer.data(), &buffer_size);
		}
		if(result == T5_ERROR_OVERFLOW) {
			buffer.resize(buffer_size);
			continue;
		}
		else if(result != T5_ERROR_NO_SERVICE && result != T5_ERROR_IO_FAILURE) {
			break;
		}
		co_await task_sleep(_poll_rate_for_retry);
	}
	co_await run_in_foreground;
	if(result != T5_SUCCESS) {
		LOG_T5_ERROR(result);
		_ndk_version = "unknown";
	}
	else {
		buffer.resize(buffer_size);
		_ndk_version = buffer.data();
	}
	log_message("Tilt Five NDK version: ", _ndk_version);
}

CotaskPtr T5Service::query_glasses_list() {
	std::vector<char> buffer;
	buffer.resize(64);
	T5_Result result;
	bool first_resize = true;

	for(;;) {
		size_t bufferSize = buffer.size();

		{
			std::lock_guard lock(g_t5_exclusivity_group_1);
			result = t5ListGlasses(_context, buffer.data(), &bufferSize);
		}
		if(result == T5_ERROR_NO_SERVICE || result == T5_ERROR_IO_FAILURE) {
			co_await task_sleep(_poll_rate_for_retry);
			continue;
		}
		else if(result == T5_ERROR_OVERFLOW && first_resize) {
			buffer.resize(bufferSize);
			first_resize = false;
			continue;
		}
		else if(result != T5_SUCCESS) {
			co_await run_in_foreground;
			LOG_T5_ERROR(result);
			co_return;
		}
		first_resize = true;

		std::string_view str_view(buffer.data(), buffer.size());
		std::vector<std::string_view> parsed_id_list;

		while(!str_view.empty()) {
			auto pos = str_view.find_first_of('\0');
			if(pos == 0 || pos == std::string_view::npos)
				break;
			parsed_id_list.emplace_back(str_view.substr(0, pos));
			str_view.remove_prefix(pos);
		}

		co_await run_in_foreground;

		for(auto& id : parsed_id_list) {
			auto found = std::find_if(
				_glasses_list.cbegin(),
				_glasses_list.cend(),
				[id](auto& gls) {return gls->get_id() == id; });

			if(found == _glasses_list.cend()) {
				auto new_glasses = std::make_shared<Glasses>(std::string(id));
				if(new_glasses->allocate_handle(_context))
					_glasses_list.push_back(new_glasses);
			}
		}

		co_await task_sleep(_poll_rate_for_monitoring);
	}
}

bool T5Service::is_service_started() {
	return _is_started;
}

void T5Service::connect_glasses(int glasses_num, std::string display_name) {
	if(glasses_num < 0 || glasses_num >= _glasses_list.size()) return;

	if(should_glasses_be_connected(_glasses_list[glasses_num]))
		_glasses_list[glasses_num]->connect(display_name);
}

void T5Service::disconnect_glasses(int glasses_num) {
	if(glasses_num < 0 || glasses_num >= _glasses_list.size()) return;

	_glasses_list[glasses_num]->disconnect();
}

void T5Service::update_connection() {
	_scheduler->schedule_tasks();
	_scheduler->log_exceptions([](auto msg) { log_message("Scheduler exception: ", msg); });

	for(int i = 0; i < _glasses_list.size(); ++i) {
		_glasses_list[i]->update_connection();
	}

	connection_updated();
}

void T5Service::update_tracking() {
	if(!_is_started)
		return;

	_scheduler->schedule_tasks();
	_scheduler->log_exceptions([](auto msg) { log_message("Scheduler exception: ", msg); });

	for(auto& glasses : _glasses_list) {
		if(glasses->is_connected())
			glasses->update_tracking();
	}

	tracking_updated();
}
} // T5Integration
