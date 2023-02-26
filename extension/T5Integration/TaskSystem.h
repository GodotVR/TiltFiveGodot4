#pragma once

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>
#if __cplusplus >= 202002L
#include <coroutine>
#include <exception>
#endif

namespace TaskSystem {

using Clock = std::chrono::steady_clock;
using TaskTime = std::chrono::time_point<Clock>;
using Duration = std::chrono::milliseconds;

struct TaskStatus {
	TaskTime _scheduled_time;
	enum : uint8_t {
		BACKGROUND,
		FOREGROUND,
		DONE,
		ERROR,
		EXCEPTION_THROWN
	} _type;
	std::exception_ptr _exception = nullptr;

	bool is_background() { return _type == BACKGROUND; }
	bool is_foreground() { return _type == FOREGROUND; }
	bool is_exception() { return _type == EXCEPTION_THROWN; }
	bool is_error() { return _type == ERROR || is_exception(); }
	bool is_done() { return _type == DONE || is_error(); }
};

const TaskStatus run_now{ TaskTime{}, TaskStatus::BACKGROUND, nullptr };
const TaskStatus task_done{ TaskTime{}, TaskStatus::DONE, nullptr };
const TaskStatus task_error{ TaskTime{}, TaskStatus::ERROR, nullptr };
const TaskStatus run_in_foreground{ TaskTime{}, TaskStatus::FOREGROUND, nullptr };

inline TaskStatus task_sleep(Duration duration) {
	return { Clock::now() + duration, TaskStatus::BACKGROUND, nullptr };
}

inline TaskStatus task_sleep(int duration) {
	return { Clock::now() + Duration{duration}, TaskStatus::BACKGROUND, nullptr };
}

inline TaskStatus capture_exception() {
	return { TaskTime{}, TaskStatus::EXCEPTION_THROWN, std::current_exception() };
}
inline TaskStatus return_exception(std::exception_ptr exception) {
	return { TaskTime{}, TaskStatus::EXCEPTION_THROWN, exception };
}

class Scheduler;
class TaskBase {
public:

	friend Scheduler;
	using Ptr = std::unique_ptr<TaskBase>;

	TaskBase() = default;
	virtual ~TaskBase() = default;

	virtual TaskTime get_scheduled_time() = 0;
	virtual bool is_foreground() = 0;
	virtual bool is_background() = 0;
	virtual bool is_done() = 0;
	virtual bool is_error() = 0;
	virtual bool is_exception() = 0;

	virtual void set_status(const TaskStatus& status) = 0;
	virtual TaskStatus get_status() = 0;

	virtual TaskStatus run_background_task();
	virtual TaskStatus run_foreground_task();
};

class Task : public TaskBase {
public:

	Task();
	~Task() override = default;

	TaskTime get_scheduled_time() override;
	bool is_foreground() override;
	bool is_background() override;
	bool is_done() override;
	bool is_error() override;
	bool is_exception() override;

	void set_status(const TaskStatus& status) override;
	TaskStatus get_status() override;

private:

	TaskStatus _status;
};

inline Task::Task()
	: _status(run_now) {}

#if __cplusplus >= 202002L

class CotaskPtr;
struct CotaskPromiseType {
	CotaskPtr get_return_object();
	std::suspend_always initial_suspend() { return{}; }

	void return_void();
	void unhandled_exception();
	std::suspend_always final_suspend() noexcept { return{}; }

	std::suspend_always await_transform(const TaskStatus& status);
	std::suspend_always await_transform(Task::Ptr&& sub_task);

	TaskStatus _status;
	Task::Ptr _sub_task;
};

class Cotask : public TaskBase {
public:

	std::coroutine_handle<CotaskPromiseType> _handle = nullptr;

	Cotask(std::coroutine_handle<CotaskPromiseType> handle)
		: _handle(handle) {}

	Cotask() = default;

	Cotask(Cotask&& rhs) noexcept
		: _handle(std::exchange(rhs._handle, nullptr)) {}

	Cotask& operator=(Cotask&& rhs) noexcept {
		_handle = std::exchange(rhs._handle, nullptr);
		return *this;
	}

	~Cotask() override {
		if(_handle) {
			_handle.destroy();
		}
	}

	TaskStatus run_background_task() override;
	TaskStatus run_foreground_task() override;

	TaskTime get_scheduled_time() override;

	bool is_foreground() override;
	bool is_background() override;
	bool is_done() override;
	bool is_error() override;
	bool is_exception() override;
	void set_status(const TaskStatus& status) override;
	TaskStatus get_status() override;
};

class CotaskPtr : public std::unique_ptr<Cotask> {
	friend CotaskPromiseType;
	CotaskPtr(Cotask* task_ptr) : std::unique_ptr<Cotask>(task_ptr) {}
public:
	using promise_type = CotaskPromiseType;
};

inline CotaskPtr CotaskPromiseType::get_return_object() {
	return CotaskPtr(new Cotask(std::coroutine_handle<CotaskPromiseType>::from_promise(*this)));
}

inline void CotaskPromiseType::return_void() {
	_status = task_done;
}

inline void CotaskPromiseType::unhandled_exception() {
	_status = capture_exception();
}

inline std::suspend_always CotaskPromiseType::await_transform(const TaskStatus& status) {
	_status = status;
	return {};
}

inline std::suspend_always CotaskPromiseType::await_transform(Task::Ptr&& sub_task) {
	_sub_task = std::move(sub_task);
	return {};
}

inline TaskTime Cotask::get_scheduled_time() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->get_scheduled_time();
	return promise._status._scheduled_time;
}

inline bool Cotask::is_foreground() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->is_foreground();
	return promise._status.is_foreground();
}

inline bool Cotask::is_background() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->is_background();
	return promise._status.is_background();
}

inline bool Cotask::is_done() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->is_done();
	return promise._status.is_done();
}

inline bool Cotask::is_error() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->is_error();
	return promise._status.is_error();
}

inline bool Cotask::is_exception() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->is_exception();
	return promise._status.is_exception();
}

inline void Cotask::set_status(const TaskStatus& status) {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		promise._sub_task->set_status(status);
	promise._status = status;
}

inline TaskStatus Cotask::get_status() {
	auto& promise = _handle.promise();
	if(promise._sub_task)
		return promise._sub_task->get_status();
	return promise._status;
}

#endif // __cplusplus >= 202002L

template<typename T, typename F>
void splice_if(std::list<T>& from_list, std::list<T>& to_list, F pred) {
	auto it = from_list.begin();
	while(it != from_list.end()) {
		auto cur = it++;
		if(pred(*cur)) {
			to_list.splice(to_list.begin(), from_list, cur);
		}
	}
}

std::string what(const std::exception_ptr& eptr);
template <typename T>
inline std::string nested_what(const T& e) {
	try { std::rethrow_if_nested(e); }
	catch(...) { return " (" + what(std::current_exception()) + ")"; }
	return {};
}
inline std::string what(const std::exception_ptr& eptr) {
	if(!eptr) { throw std::bad_exception(); }
	try { std::rethrow_exception(eptr); }
	catch(const std::exception& e) { return e.what() + nested_what(e); }
	catch(const std::string& e) { return e; }
	catch(const char* e) { return e; }
	catch(...) { return "unknown exception type"; }
}

class Scheduler {
public:
	using ExceptionLogger = void(std::string);
	using Ptr = std::shared_ptr<Scheduler>;

	Scheduler();
	virtual ~Scheduler();

	void start();
	void stop();

	void add_task(TaskBase::Ptr&& task);

	void schedule_tasks();
	std::list<std::exception_ptr> get_exceptions();
	void log_exceptions(ExceptionLogger func);

private:

	void do_background_tasks();
	void queue_background_tasks();
	void do_foreground_tasks();

	bool _is_initial_run{ true };
	TaskTime _last_run;
	long _run_count = 0;
	Duration _average_time = Duration(0);

	std::thread _background_thread;

	std::atomic_bool _is_running{ false };

	std::mutex _background_run_mutex;
	std::mutex _background_wait_mutex;
	std::mutex _foreground_mutex;
	std::mutex _exception_mutex;

	std::condition_variable _background_release;

	std::list<TaskBase::Ptr> _background_run_list;
	std::list<TaskBase::Ptr> _background_wait_list;
	std::list<TaskBase::Ptr> _foreground_list;
	std::list<std::exception_ptr> _exception_list;
};
} // namespace TaskSystem 

