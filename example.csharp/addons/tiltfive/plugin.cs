#if TOOLS
using Godot;
using System;

[Tool]
public partial class plugin : EditorPlugin
{
	static bool isInitialized = false;

	private void Setup()
	{
		if (!isInitialized)
		{
			AddAutoloadSingleton("T5Interface", "res://addons/tiltfive/T5Interface.cs");
			isInitialized = true;
		}

	}

	public override void _EnterTree()
	{
		T5ProjectSettings.setup_properties();
		Setup();

	}

	public override void _EnablePlugin()
	{
		base._EnablePlugin();
		Setup();
	}

	public override void _DisablePlugin()
	{
		base._EnablePlugin();
		if(isInitialized)
		{
			RemoveAutoloadSingleton("T5Interface");
			isInitialized = false;
		}
	}
}
#endif
