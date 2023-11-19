using Godot;
using System;

public partial class T5NodeCS : Node3D
{
	public StringName Tracker {  
		get { 
			return Get("tracker").As<StringName>(); 
		} 
		set {
			Set("tracker", value);
		}
	}
	
	public bool getIsActive()
	{
		return Call("get_is_active").AsBool();
	}

	public bool getHasTrackingData()
	{
		return Call("get_has_tracking_data").AsBool();
	}

	public XRPose getPose() 
	{
		return Call("get_pose").As<XRPose>();
	}
}
