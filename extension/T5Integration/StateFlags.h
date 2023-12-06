#pragma once
#include <atomic>
#include <type_traits>

template <typename T>
class StateFlags {
	static_assert(std::is_integral_v<T>);

public:
	using FlagType = T;

	FlagType get_current() {
		return _current.load();
	}

	void set(FlagType state) {
		_current.fetch_or(state);
	}

	bool set_and_was_toggled(FlagType state) {
		T old = _current.fetch_or(state);
		return (old & state) != state;
	}

	void clear(FlagType state) {
		_current.fetch_and(~state);
	}

	bool clear_and_was_toggled(FlagType state) {
		T old = _current.fetch_and(~state);
		return (old & state) == state;
	}

	void clear_all() {
		_current.store(0);
	}

	void reset(FlagType state) {
		_current.store(state);
	}

	bool is_current(FlagType state) const {
		return (_current.load() & state) == state;
	}

	bool is_any_current(FlagType state) const {
		return (_current.load() & state) != 0;
	}

	bool is_not_current(FlagType state) const {
		return (_current.load() & state) != state;
	}

	bool any_changed(const StateFlags<FlagType>& from, FlagType query_state) const {
		return ((_current.load() ^ from._current.load()) & query_state) != 0;
	}

	bool became_set(const StateFlags<FlagType>& from, FlagType query_state) const {
		return (_current.load() & query_state) == query_state &&
				(from._current.load() & query_state) != query_state;
	}

	bool became_clear(const StateFlags<FlagType>& from, FlagType query_state) const {
		return (_current.load() & query_state) != query_state &&
				(from._current.load() & query_state) == query_state;
	}

	void sync_from(const StateFlags<FlagType>& from) {
		_current.store(from._current.load());
	}

private:
	std::atomic<FlagType> _current;
};
