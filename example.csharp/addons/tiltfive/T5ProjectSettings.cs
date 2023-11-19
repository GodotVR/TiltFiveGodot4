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
			DefineProjectSetting("xr/tilt_five/application_id", Variant.Type.String, PropertyHint.None, "", "my.game.com");
			DefineProjectSetting("xr/tilt_five/application_version", Variant.Type.String, PropertyHint.None, "", "0.1.0");
			DefineProjectSetting("xr/tilt_five/default_display_name", Variant.Type.String, PropertyHint.None, "", "Game: Player One");
			DefineProjectSetting("xr/tilt_five/trigger_click_threshhold", Variant.Type.Float, PropertyHint.Range, "0,1,0.01", 0.3);

			isInitialized = true;
		}
	}

	public static String ApplicationID
	{
		get { setup_properties();  return ProjectSettings.GetSettingWithOverride("xr/tilt_five/application_id").AsString(); }
	}

	public static String ApplicationVersion
	{
		get { setup_properties(); return ProjectSettings.GetSettingWithOverride("xr/tilt_five/application_version").AsString(); }
	}

	public static String DefaultDisplayName
	{
		get { setup_properties(); return ProjectSettings.GetSettingWithOverride("xr/tilt_five/default_display_name").AsString(); }
	}

	public static float TriggerClickThreshhold
	{
		get { setup_properties(); return (float)ProjectSettings.GetSettingWithOverride("xr/tilt_five/trigger_click_threshhold").AsDouble(); }
	}
}

