#pragma once
#include <Logging.h>
#include <T5Math.h>
#include <T5Service.h>
#include <TaskSystem.h>
#include <memory>

namespace T5Integration {

using TaskSystem::Scheduler;

class ObjectRegistry {
protected:
	ObjectRegistry();
	virtual ~ObjectRegistry() = default;

public:
	static T5Service::Ptr service();
	static T5Math::Ptr math();
	static Logger::Ptr logger();
	static Scheduler::Ptr scheduler();

protected:
	virtual T5Service::Ptr get_service() = 0;
	virtual T5Math::Ptr get_math() = 0;
	virtual Logger::Ptr get_logger();
	virtual Scheduler::Ptr get_scheduler();

	static ObjectRegistry* _instance;

	Logger::Ptr::weak_type _logger;
	Scheduler::Ptr::weak_type _scheduler;
};
} //namespace T5Integration