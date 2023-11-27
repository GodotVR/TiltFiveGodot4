using Godot;
using System;

public static class T5Def {

	public enum GameboardType
	{
		Unknown = 1,
		LE = 2,
		XE = 3,
		XE_Raised = 4
	}

	// Buttons
	public static StringName  WAND_BUTTON_A		= "button_a";
	public static StringName  WAND_BUTTON_B		= "button_b";
	public static StringName  WAND_BUTTON_X		= "button_x";
	public static StringName  WAND_BUTTON_Y		= "button_y";
	public static StringName  WAND_BUTTON_1		= "button_1";
	public static StringName  WAND_BUTTON_2		= "button_2";
	public static StringName  WAND_BUTTON_STICK	= "button_3";
	public static StringName  WAND_BUTTON_T5	    = "button_t5";
	public static StringName  WAND_BUTTON_TRIGGER = "trigger_click";
	// Axis
	public static StringName  WAND_ANALOG_STICK	= "stick";
	public static StringName  WAND_ANALOG_TRIGGER = "trigger";

}
