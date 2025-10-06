using Godot;
using System;


public static class T5ProjectSettings
{
	static bool isInitialized = false;

	static void DefineProjectSetting(String name, Variant.Type setting_type, PropertyHint hint, String hintString, Variant defaultValue)
	{
		if(!ProjectSettings.HasSetting(name))
		{
			ProjectSettings.SetSetting(name, defaultValue);
		}

		var propertyInfo = new Godot.Collections.Dictionary();

		propertyInfo["name"] = name;
		propertyInfo["type"] = (int)setting_type;
		propertyInfo["hint"] = (int)hint;
		propertyInfo["hint_string"] = hintString;

		ProjectSettings.AddPropertyInfo(propertyInfo);
		ProjectSettings.SetAsBasic(name, true);
		ProjectSettings.SetInitialValue(name, defaultValue);
	}

	public static void setup_properties() {
		if (!isInitialized) {
			DefineProjectSetting("xr/tilt_five/trigger_click_threshhold", Variant.Type.Float, PropertyHint.Range, "0,1,0.01", 0.3);
			DefineProjectSetting("xr/tilt_five/debug_logging", Variant.Type.Bool, PropertyHint.None, "", false);

			isInitialized = true;
		}
	}

	public static String ApplicationID
	{
		get {
			var app_id = ProjectSettings.GetSettingWithOverride("application/config/name").AsString();
			if (app_id == null || app_id == "")
				return "tiltfive.godot.game";
			return  app_id;
		}
	}

	public static String ApplicationVersion
	{
		get {
			var version = ProjectSettings.GetSettingWithOverride("application/config/version").AsString();
			if (version == null || version == "")
				return "unknown";
			return  version;
		}
	}

	public static float TriggerClickThreshhold
	{
		get { setup_properties(); return (float)ProjectSettings.GetSettingWithOverride("xr/tilt_five/trigger_click_threshhold").AsDouble(); }
	}

	public static bool IsDebugLogging
	{
		get { setup_properties(); return ProjectSettings.GetSettingWithOverride("xr/tilt_five/debug_logging").AsBool(); }
	}
}

