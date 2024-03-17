#include "jthread.h"

#ifndef __cpp_lib_jthread
#include <atomic>
#include <cassert>
#include <mutex>
#include <vector>

namespace jthrd {
struct StopState {
	std::atomic<bool> request_stop = false;

	// this is not efficient, but it only needs to be correct
	std::mutex callbacks_mutex;
	std::vector<stop_callback*> callbacks;

	void add_callback(stop_callback* callback) {
		auto lk = std::unique_lock<std::mutex>(callbacks_mutex);
		if (!request_stop) {
			callbacks.push_back(callback);
		} else {
			callback->callback();
		}
	}

	void remove_callback(stop_callback* callback) {
		auto lk = std::unique_lock<std::mutex>(callbacks_mutex);
		auto i = std::find(begin(callbacks), end(callbacks), callback);
		if (i != end(callbacks)) {
			callbacks.erase(i);
		}
	}

	void exec_callbacks() {
		auto lk = std::unique_lock<std::mutex>(callbacks_mutex);
		for (const auto& cb : callbacks) {
			assert(cb->callback);
			cb->callback();
		}
		callbacks.clear();
	}
};

stop_token::stop_token() = default;

stop_token::stop_token(std::shared_ptr<StopState> s) :
		state(s) {}

stop_token::stop_token(const stop_token& other) noexcept = default;
stop_token::stop_token(stop_token&& other) noexcept = default;
stop_token::~stop_token() = default;

stop_token& stop_token::operator=(const stop_token& other) noexcept = default;
stop_token& stop_token::operator=(stop_token&& other) noexcept = default;

bool stop_token::stop_requested() noexcept {
	if (state) {
		return state->request_stop;
	}
	return false;
}

bool stop_token::stop_possible() const noexcept {
	return static_cast<bool>(state);
}

void stop_token::swap(stop_token& other) noexcept {
	state.swap(other.state);
}

stop_source::stop_source() :
		state(std::make_shared<StopState>()) {}

stop_source::stop_source(nostopstate_t nss) noexcept {}

stop_source::stop_source(const stop_source& other) noexcept = default;
stop_source::stop_source(stop_source&& other) noexcept = default;
stop_source::~stop_source() = default;

stop_source& stop_source::operator=(const stop_source& other) noexcept = default;
stop_source& stop_source::operator=(stop_source&& other) noexcept = default;

stop_token stop_source::get_token() const noexcept {
	return stop_token{ state };
}

bool stop_source::request_stop() noexcept {
	if (state && (state->request_stop.exchange(true) == false)) {
		state->exec_callbacks();
		return true;
	}
	return false;
}

bool stop_source::stop_possible() const noexcept {
	return static_cast<bool>(state);
}

void stop_source::swap(stop_source& other) noexcept {
	state.swap(other.state);
}

stop_callback::~stop_callback() {
	if (token.state) {
		token.state->remove_callback(this);
	}
}

void stop_callback::self_register() {
	if (token.state) {
		token.state->add_callback(this);
	} else {
		callback();
	}
}

jthread::jthread() noexcept
		:
		stop{ nostopstate } {}

jthread::jthread(jthread&& other) noexcept = default;

jthread::~jthread() {
	if (joinable()) {
		request_stop();
		join();
	}
}

jthread& jthread::operator=(jthread&& other) noexcept = default;

jthread::id jthread::get_id() const noexcept {
	return impl.get_id();
}

bool jthread::joinable() const noexcept {
	return impl.joinable();
}

void jthread::join() {
	impl.join();
}

void jthread::detach() {
	impl.detach();
}

stop_source jthread::get_stop_source() noexcept {
	if (joinable()) {
		return stop;
	} else {
		return stop_source{ nostopstate };
	}
}

stop_token jthread::get_stop_token() noexcept {
	return get_stop_source().get_token();
}

bool jthread::request_stop() noexcept {
	return stop.request_stop();
}

void jthread::swap(jthread& other) noexcept {
	stop.swap(other.stop);
	impl.swap(other.impl);
}
} //namespace jthrd
#endif