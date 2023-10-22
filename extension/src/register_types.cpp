#include <register_types.h>
#include <TiltFiveXRInterface.h>
#include <T5Origin3D.h>
#include <T5Camera3D.h>
#include <T5Controller3D.h>
#include <T5Gameboard.h>
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/xr_server.hpp>

using namespace godot;

void initialize_tiltfive_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<TiltFiveXRInterface>();
	ClassDB::register_class<T5Origin3D>();
	ClassDB::register_class<T5Camera3D>();
	ClassDB::register_class<T5Node3D>();
	ClassDB::register_class<T5Controller3D>();
	ClassDB::register_class<T5Gameboard>();


}

void uninitialize_tiltfive_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C"
{

	// Initialization.

	GDExtensionBool GDE_EXPORT tiltfive_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_tiltfive_types);
		init_obj.register_terminator(uninitialize_tiltfive_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
