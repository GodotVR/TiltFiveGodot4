#pragma once
#include <atomic>
#include <type_traits>

template<typename T>
class StateFlags
{
	static_assert(std::is_integral_v<T>);

	using FlagType = T;
	public:

	std::atomic<T> _current;
	FlagType _previous;

	FlagType get_current()
	{
		return _current.load();
	}

	void set(FlagType state)
	{
		_current.fetch_or(state);
	}

	void clear(FlagType state)
	{
		_current.fetch_and(~state);
	}

	void clear_all(bool clear_changes = true)
	{
		_current.store(0);
		if(clear_changes) 
			_previous = 0;
	}

	void reset(FlagType state, bool clear_changes = false)
	{
		_current.store(state);
		if(clear_changes) 
		    _previous = state;
	}

	bool is_current(FlagType state) const
	{
		return (_current.load() & state) == state;
	}

	bool any_changed(FlagType state) const
	{
		return ((_current.load() ^ _previous) & state) != 0;
	}

	bool test_then_update_changes(FlagType state)
	{
		FlagType current = _current.load();
		bool rtn = ((current ^ _previous) & state) != 0;
		_previous = current & state | _previous & ~state;
		return rtn;
	}

	FlagType get_changes() const
	{
		return (_current.load() ^ _previous);
	}

	FlagType get_then_update_changes()
	{
		auto ret = get_changes();
		_previous = _current;
		return ret;
	}
};
