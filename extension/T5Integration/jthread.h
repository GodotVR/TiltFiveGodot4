/* This provides a drop in replacement for std::jthread, std::stop_token,
 * std::stop_source, and std::stop_callback for compilers that do not yet
 * support them. This was pulled from a reference implementation of the
 * jthread proposal
 */
#ifndef _JTHREAD_H_
#define _JTHREAD_H_

#include <functional>
#include <memory>
#include <thread>

namespace jthrd {
#ifdef __cpp_lib_jthread
// If jthread is available, pull it into this namespace
using std::jthread;
using std::nostopstate;
using std::stop_callback;
using std::stop_source;
using std::stop_token;
#else
// otherwise, implement a drop in replacement in this namespace
struct StopState;

struct nostopstate_t {};
constexpr auto nostopstate = nostopstate_t{};

class stop_token {
public:
	stop_token();
	explicit stop_token(std::shared_ptr<StopState> state);
	stop_token(const stop_token& other) noexcept;
	stop_token(stop_token&& other) noexcept;
	~stop_token();

	stop_token& operator=(const stop_token& other) noexcept;
	stop_token& operator=(stop_token&& other) noexcept;

	[[nodiscard]] bool stop_requested() noexcept;
	[[nodiscard]] bool stop_possible() const noexcept;

	void swap(stop_token& other) noexcept;

private:
	std::shared_ptr<StopState> state;

	friend class stop_callback;
};

inline void swap(stop_token& lhs, stop_token& rhs) noexcept {
	lhs.swap(rhs);
}

class stop_source {
public:
	stop_source();
	explicit stop_source(nostopstate_t nss) noexcept;
	stop_source(const stop_source& other) noexcept;
	stop_source(stop_source&& other) noexcept;
	~stop_source();

	stop_source& operator=(const stop_source& other) noexcept;
	stop_source& operator=(stop_source&& other) noexcept;

	[[nodiscard]] stop_token get_token() const noexcept;

	bool request_stop() noexcept;
	[[nodiscard]] bool stop_possible() const noexcept;

	void swap(stop_source& other) noexcept;

private:
	std::shared_ptr<StopState> state;
};

inline void swap(stop_source& lhs, stop_source& rhs) noexcept {
	lhs.swap(rhs);
}

class stop_callback {
public:
	template <class C>
	explicit stop_callback(const stop_token& st, C&& cb) :
			token(st), callback(cb) {
		self_register();
	}

	template <class C>
	explicit stop_callback(stop_token&& st, C&& cb) :
			token(std::forward<stop_token>(st)), callback(cb) {
		self_register();
	}

	~stop_callback();

private:
	stop_token token;
	std::function<void()> callback;

	void self_register();

	stop_callback(const stop_callback&) = delete;
	stop_callback(stop_callback&&) = delete;
	stop_callback& operator=(const stop_callback& other) noexcept = delete;
	stop_callback& operator=(stop_callback&& other) noexcept = delete;

	friend struct StopState;
};

class jthread {
public:
	using id = std::thread::id;

	jthread() noexcept;
	jthread(jthread&& other) noexcept;

	template <class Function, class... Args>
		requires std::is_invocable_v<std::decay_t<Function>, std::decay_t<Args>...>
	explicit jthread(Function&& f, Args&&... args) :
			impl(std::forward<Function>(f), std::forward<Args>(args)...) {}

	template <class Function, class... Args>
		requires std::is_invocable_v<std::decay_t<Function>, stop_token, std::decay_t<Args>...>
	explicit jthread(Function&& f, Args&&... args) :
			stop(),
			impl(std::forward<Function>(f), stop.get_token(), std::forward<Args>(args)...) {}

	~jthread();

	jthread& operator=(jthread&& other) noexcept;

	[[nodiscard]] id get_id() const noexcept;
	[[nodiscard]] bool joinable() const noexcept;

	void join();
	void detach();

	[[nodiscard]] stop_source get_stop_source() noexcept;
	[[nodiscard]] stop_token get_stop_token() noexcept;
	bool request_stop() noexcept;

	void swap(jthread& other) noexcept;

private:
	stop_source stop = {};
	std::thread impl;

	jthread(const jthread&) = delete;
	jthread& operator=(const jthread& other) noexcept = delete;
};

inline void swap(jthread& lhs, jthread& rhs) noexcept {
	lhs.swap(rhs);
}
#endif
} //namespace jthrd

#endif //_JTHREAD_H_