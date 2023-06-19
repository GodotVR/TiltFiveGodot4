#include <algorithm>  
#include <T5Service.h>
#include <ObjectRegistry.h>
#include <Logging.h>

namespace T5Integration {

std::mutex g_t5_exclusivity_group_1;
std::mutex g_t5_exclusivity_group_2;

T5Service::T5Service() {
	_graphics_api = T5_GraphicsApi::kT5_GraphicsApi_None;
	_scheduler = ObjectRegistry::scheduler();
}

T5Service::~T5Service() {

	stop_service();
}

bool T5Service::start_service(const std::string_view application_id, std::string_view application_version) {
	if(_graphics_api == T5_GraphicsApi::kT5_GraphicsApi_None) {
		LOG_ERROR("TiltFive graphics api is not set");
		return false;
	}
	if(_is_started) return true;

	T5_ClientInfo clientInfo;
	clientInfo.applicationId = application_id.data();
	clientInfo.applicationVersion = application_version.data();
	
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
	for(int i = 0; i < _glasses_list.size(); i++) {
		_glasses_list[i]->disconnect();
		_glasses_list[i]->destroy_handle();
	}
	_glasses_list.clear();

	if(_context)
	{
		std::lock_guard lock(g_t5_exclusivity_group_1);
		t5DestroyContext(&_context);
	}
	_context = nullptr;
}

std::optional<int> T5Service::find_glasses_idx(const std::string_view glasses_id) {
	for(int i = 0; i < _glasses_list.size(); ++i) {
		if(_glasses_list[i]->get_id() == glasses_id)
			return i;
	}
	return {};
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
			str_view.remove_prefix(pos + 1);

		}

		co_await run_in_foreground;

		for(auto& id : parsed_id_list) {
			auto found = std::find_if(
				_glasses_list.cbegin(),
				_glasses_list.cend(),
				[id](auto& gls) {return gls->get_id() == id; });

			if(found == _glasses_list.cend()) {
				auto new_glasses = create_glasses(id);
				if(new_glasses->allocate_handle(_context))
					_glasses_list.emplace_back(std::move(new_glasses));
			}
		}

		co_await task_sleep(_poll_rate_for_monitoring);
	}
}

std::unique_ptr<Glasses> T5Service::create_glasses(const std::string_view id) {
	return std::make_unique<Glasses>(id);
}

bool T5Service::is_service_started() {
	return _is_started;
}

void T5Service::reserve_glasses(int glasses_num, const std::string_view display_name) {
	if(glasses_num < 0 || glasses_num >= _glasses_list.size()) return;

	if(should_glasses_be_reserved(glasses_num))
		_glasses_list[glasses_num]->connect(display_name);
}

void T5Service::release_glasses(int glasses_num) {
	if(glasses_num < 0 || glasses_num >= _glasses_list.size()) return;

	_glasses_list[glasses_num]->disconnect();
}

void T5Service::connect_glasses(int glasses_num, std::string display_name) {
	reserve_glasses(glasses_num, display_name);
}

void T5Service::disconnect_glasses(int glasses_num) {
	release_glasses(glasses_num);
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

void T5Service::get_events(std::vector<GlassesEvent>& out_events) {
	for(int i = 0; i < _glasses_list.size(); ++i) {
		_glasses_list[i]->get_events(i, out_events);
	}
}

void T5Service::get_gameboard_size(T5_GameboardType gameboard_type, T5_GameboardSize& gameboard_size) {
	auto result = t5GetGameboardSize(_context, gameboard_type, &gameboard_size);
	if(result != T5_SUCCESS)
		LOG_T5_ERROR(result);
}


} // T5Integration
