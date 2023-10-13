#pragma once
#include <atomic>
#include <type_traits>

template<typename T>
class StateFlags
{
	static_assert(std::is_integral_v<T>);

	public:

	using FlagType = T;

	FlagType get_current()
	{
		return _current.load();
	}

	FlagType get_changes() const
	{
		return (_current.load() ^ _previous);
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

	bool is_any_current(FlagType state) const
	{
		return (_current.load() & state) != 0;
	}

	bool is_not_current(FlagType state) const
	{
		return (_current.load() & state) != state;
	}

	bool any_changed(FlagType state) const
	{
		return ((_current.load() ^ _previous) & state) != 0;
	}

	bool became_set(FlagType state) const {
		return (_current.load() & state) == state && (_previous & state) != state;
	}

	bool became_clear(FlagType state) const {
		return (_current.load() & state) != state && (_previous & state) == state;
	}

	void reset_changes()
	{
		_previous = _current;
	}

	private:
	std::atomic<FlagType> _current;
	FlagType _previous;
};
