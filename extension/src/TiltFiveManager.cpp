//© Copyright 2014-2022, Juan Linietsky, Ariel Manzur and the Godot community (CC-BY 3.0)
#include "TiltFiveManager.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

TiltFiveManager::TiltFiveManager()
{
    count = 0;
}

TiltFiveManager::~TiltFiveManager()
{
}

void TiltFiveManager::_bind_methods()
{
    //ClassDB::bind_method(D_METHOD("add", "value"), &TiltFiveManager::add, DEFVAL(1));
    //ClassDB::bind_method(D_METHOD("reset"), &TiltFiveManager::reset);
    //ClassDB::bind_method(D_METHOD("get_total"), &TiltFiveManager::get_total);
}