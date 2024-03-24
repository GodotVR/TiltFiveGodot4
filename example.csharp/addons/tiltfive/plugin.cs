#if TOOLS
using Godot;
using Godot.Collections;
using System;

[Tool]
public partial class plugin : EditorPlugin
{
	static bool isInitialized = false;

	AndroidExportPlugin _androidExportPlugin;

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
		_androidExportPlugin = new AndroidExportPlugin();
		AddExportPlugin(_androidExportPlugin);
		Setup();
	}

	public override void _ExitTree()
	{
		RemoveExportPlugin(_androidExportPlugin);
		_androidExportPlugin = null;
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

[Tool]
public partial class AndroidExportPlugin : EditorExportPlugin
{
	static string _plugin_name = "gdtiltfive";

	public override bool _SupportsPlatform(EditorExportPlatform platform) {
		if(platform is EditorExportPlatformAndroid) {
			return true;
		}
		return false;
	}

	public override string[] _GetAndroidLibraries(EditorExportPlatform platform, bool debug) {
		if(platform is EditorExportPlatformAndroid) {
			if(debug) {
				return new string[] {"tiltfive/bin/android/" + _plugin_name + "-debug.aar"};
			} else {
				return new string[] {"tiltfive/bin/android/" + _plugin_name + "-release.aar"};
			}
		}
		return null;
	}

	public override string _GetName() {
		return _plugin_name;
	}
}
#endif
