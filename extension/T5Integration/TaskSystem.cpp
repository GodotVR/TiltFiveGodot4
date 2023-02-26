// BGTask.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <TaskSystem.h>
#include <utility>

namespace TaskSystem {

TaskStatus TaskBase::run_background_task() {
	return task_done;
}

TaskStatus TaskBase::run_foreground_task() {
	return task_done;
}

TaskTime Task::get_scheduled_time() {
	return _status._scheduled_time;
}

bool Task::is_background() {
	return _status.is_background();
}

bool Task::is_foreground() {
	return _status.is_foreground();
}
bool Task::is_done() {
	return _status.is_done();
}
bool Task::is_error() {
	return _status.is_error();
}
bool Task::is_exception() {
	return _status.is_exception();
}

void Task::set_status(const TaskStatus& status) {
	_status = status;
}

TaskStatus Task::get_status() {
	return _status;
}


#if __cplusplus >= 202002L


TaskStatus Cotask::run_background_task() {
	auto& promise = _handle.promise();
	while(true) {
		if(promise._sub_task) {
			auto status = promise._sub_task->run_background_task();
			if(!status.is_done())
				return status;
			promise._sub_task.reset();

			if(status.is_error())
				return status;
			if(is_foreground()) {
				return run_in_foreground;
			}
		}
		_handle.resume();
		if(!promise._sub_task)
			return promise._status;
		if(promise._sub_task->is_foreground()) {
			return run_in_foreground;
		}
	}
}

TaskStatus Cotask::run_foreground_task() {
	auto& promise = _handle.promise();
	while(true) {
		if(promise._sub_task) {
			auto status = promise._sub_task->run_foreground_task();
			if(!status.is_done())
				return status;
			promise._sub_task.reset();

			if(status.is_error())
				return status;
			if(is_background()) {
				return run_now;
			}
		}
		_handle.resume();
		if(!promise._sub_task)
			return _handle.promise()._status;
		if(promise._sub_task->is_background()) {
			return task_sleep(0);
		}
	}
}

#endif

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {
	stop();
}

void Scheduler::start() {
	_is_running = true;
	_is_initial_run = true;
	_background_thread = std::thread(&Scheduler::do_background_tasks, this);
}

void Scheduler::stop() {
	if(_is_running) {
		_is_running = false;
		_background_release.notify_one();
		if(_background_thread.joinable())
			_background_thread.join();
		_background_run_list.clear();
		_background_wait_list.clear();
		_foreground_list.clear();

	}
}

void Scheduler::add_task(TaskBase::Ptr&& task) {
	if(task->is_background()) {
		if(task->get_scheduled_time() < Clock::now() + _average_time) {
			{
				std::lock_guard<std::mutex> lk(_background_run_mutex);
				_background_run_list.push_front(std::forward<TaskBase::Ptr>(task));
			}
			_background_release.notify_one();
		}
		else {
			std::lock_guard<std::mutex> lk(_background_wait_mutex);
			_background_wait_list.push_front(std::forward<TaskBase::Ptr>(task));
		}
	}
	else {
		std::lock_guard<std::mutex> lk(_foreground_mutex);
		_foreground_list.push_front(std::forward<TaskBase::Ptr>(task));
	}
}

void Scheduler::schedule_tasks() {
	auto time_now = Clock::now();
	if(_is_initial_run) {
		_average_time = Duration(0);
		_is_initial_run = false;
	}
	else {
		auto delta = std::chrono::duration_cast<Duration>(time_now - _last_run);

		_average_time = (delta + _run_count * _average_time) / (_run_count + 1);
		_run_count++;
	}
	_last_run = time_now;

	queue_background_tasks();
	do_foreground_tasks();
}

void Scheduler::do_background_tasks() {
	while(_is_running) {
		// Swap all tasks to local stack
		std::list<TaskBase::Ptr> do_list;
		{
			std::unique_lock lk(_background_run_mutex);
			_background_release.wait(lk, [this] { return !_background_run_list.empty() || !_is_running; });
			std::swap(_background_run_list, do_list);
		}
		// run them
		for(auto& task : do_list) {
			if(!_is_running) break;
			try {
				task->set_status(task->run_background_task());
			}
			catch(...) {
				task->set_status(capture_exception());
			}
		}
		// move background tasks back to the background wait list
		std::list<TaskBase::Ptr> to_background;
		splice_if(do_list, to_background, [](const TaskBase::Ptr& task) { return task->is_background(); });
		{
			std::lock_guard lk(_background_wait_mutex);
			_background_wait_list.splice(_background_wait_list.end(), to_background);
		}
		// move foreground tasks back to the forground list
		std::list<TaskBase::Ptr> to_foreground;
		splice_if(do_list, to_foreground, [](const TaskBase::Ptr& task) { return task->is_foreground(); });
		{
			std::lock_guard lk(_foreground_mutex);
			_foreground_list.splice(_foreground_list.end(), to_foreground);
		}
		std::list<std::exception_ptr> to_exception;
		for(auto& task : do_list) {
			if(task->is_exception()) {
				to_exception.push_back(task->get_status()._exception);
			}
			std::lock_guard lk(_exception_mutex);
			_exception_list.splice(_exception_list.end(), to_exception);
		}
		// all done tasks die here
	}
}

void Scheduler::queue_background_tasks() {
	auto time_now = Clock::now() + _average_time;

	// Swap all tasks to local stack
	std::list<TaskBase::Ptr> test_list;
	{
		std::lock_guard lk(_background_wait_mutex);
		std::swap(_background_wait_list, test_list);
	}

	// Get the ones that need to run soon
	std::list<TaskBase::Ptr> do_background;
	splice_if(test_list, do_background, [time_now](const TaskBase::Ptr& task) { return task->get_scheduled_time() <= time_now; });

	// If we have some then move them to the run list
	if(!do_background.empty()) {
		{
			std::lock_guard lk(_background_run_mutex);
			_background_run_list.splice(_background_run_list.end(), do_background);
		}
		_background_release.notify_one();
	}

	// Put the ones that are left back
	if(!test_list.empty()) {
		std::lock_guard lk(_background_wait_mutex);
		_background_wait_list.splice(_background_wait_list.end(), test_list);
	}
}

void Scheduler::do_foreground_tasks() {
	// Swap all tasks to local 
	std::list<TaskBase::Ptr> do_list;
	{
		std::lock_guard lk(_foreground_mutex);
		std::swap(_foreground_list, do_list);
	}
	// run them
	for(auto& task : do_list) {
		try {
			task->set_status(task->run_foreground_task());
		}
		catch(...) {
			task->set_status(capture_exception());
		}
	}
	// move background tasks back to the background wait list
	std::list<TaskBase::Ptr> to_background;
	splice_if(do_list, to_background, [](const TaskBase::Ptr& task) { return task->is_background(); });
	{
		std::lock_guard lk(_background_wait_mutex);
		_background_wait_list.splice(_background_wait_list.end(), to_background);
	}
	// move foreground tasks back to the forground list
	std::list<TaskBase::Ptr> to_foreground;
	splice_if(do_list, to_foreground, [](const TaskBase::Ptr& task) { return task->is_foreground(); });
	{
		std::lock_guard lk(_foreground_mutex);
		_foreground_list.splice(_foreground_list.end(), to_foreground);
	}
	std::list<std::exception_ptr> to_exception;
	for(auto& task : do_list) {
		if(task->is_exception()) {
			to_exception.push_back(task->get_status()._exception);
		}
		std::lock_guard lk(_exception_mutex);
		_exception_list.splice(_exception_list.end(), to_exception);
	}
	// all done tasks die here
}
std::list<std::exception_ptr> Scheduler::get_exceptions() {
	std::list<std::exception_ptr> return_list;
	std::lock_guard lk(_exception_mutex);
	std::swap(return_list, _exception_list);
	return return_list;
}
void Scheduler::log_exceptions(ExceptionLogger func) {
	std::list<std::exception_ptr> except_list;
	{
		std::lock_guard lk(_exception_mutex);
		std::swap(except_list, _exception_list);
	}
	for(auto exc_ptr : except_list) {
		func(what(exc_ptr));
	}
}
}
