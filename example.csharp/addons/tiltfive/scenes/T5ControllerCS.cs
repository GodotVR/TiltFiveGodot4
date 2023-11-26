using Godot;
using System;

[Tool]
public partial class T5ControllerCS : T5NodeCS
{
	
	public void TriggerHapticPulse(float amplitude, int duration) 
	{
		Call("trigger_haptic_pulse", amplitude, duration);
	}

	public bool IsButtonPressed(StringName name)
	{
		return Call("is_button_pressed", name).AsBool();
	}

	public Variant GetInput(StringName name)
	{
		return Call("get_input", name);
	}

	public float GetFloat(StringName name)
	{
		return Call("get_float", name).As<float>();
	}

	public Vector2 GetVector2(StringName name)
	{
		return Call("get_vector2", name).As<Vector2>();
	}
}
