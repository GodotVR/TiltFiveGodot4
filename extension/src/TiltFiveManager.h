#ifndef TILT_FIVE_CLASS_H
#define TILT_FIVE_CLASS_H

// We don't need windows.h in this plugin but many others do and it throws up on itself all the time
// So best to include it and make sure CI warns us when we use something Microsoft took for their own goals....
#ifdef WIN32
#include <windows.h>
#endif

#include <godot_cpp/classes/node.hpp>

using namespace godot;

class TiltFiveManager : public Node
{
    GDCLASS(TiltFiveManager, Node);

    int count;

protected:
    static void _bind_methods();

public:
    TiltFiveManager();
    ~TiltFiveManager();
};

#endif // TILT_FIVE_CLASS_H