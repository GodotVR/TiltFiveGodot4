#include <cassert>
#include <ObjectRegistry.h>

namespace T5Integration {

	ObjectRegistry* ObjectRegistry::_instance = nullptr;

	ObjectRegistry::ObjectRegistry() {
		_instance = this;
	}

	T5Service::Ptr ObjectRegistry::service() {
		assert(_instance);
		return _instance->get_service();
	}

	T5Math::Ptr ObjectRegistry::math() {
		assert(_instance);
		return _instance->get_math();
	}

	Logger::Ptr ObjectRegistry::logger() {
		assert(_instance);
		return _instance->get_logger();
	}

	Scheduler::Ptr ObjectRegistry::scheduler() {
		assert(_instance);
		return _instance->get_scheduler();
	}

	Logger::Ptr ObjectRegistry::get_logger() {
		Logger::Ptr logger;
		if (_logger.expired())
		{
			logger = std::make_shared<DefaultLogger>();
			_logger = logger;
		}
		else {
			logger = _logger.lock();
		}
		return logger;
	}

	Scheduler::Ptr ObjectRegistry::get_scheduler() {
		Scheduler::Ptr scheduler;
		if (_scheduler.expired()) {
			scheduler = std::make_shared<Scheduler>();
			_scheduler = scheduler;
		}
		else {
			scheduler = _scheduler.lock();
		}
		return scheduler;
	}
}