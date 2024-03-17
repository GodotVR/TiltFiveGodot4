using Godot;
using System;
using System.Collections.Generic;

public partial class T5Interface : Node
{
	enum ServiceEventType
	{
		E_SERVICE_STOPPED = 1,
		E_SERVICE_RUNNING = 2,
		E_SERVICE_T5_UNAVAILABLE = 3,
		E_SERVICE_T5_INCOMPATIBLE_VERSION = 4
	}

	enum GlassesEventType
	{
		E_GLASSES_ADDED = 1,
		E_GLASSES_LOST = 2,
		E_GLASSES_AVAILABLE = 3,
		E_GLASSES_UNAVAILABLE = 4,
		E_GLASSES_RESERVED = 5,
		E_GLASSES_DROPPED = 6,
		E_GLASSES_TRACKING = 7,
		E_GLASSES_NOT_TRACKING = 8,
		E_GLASSES_STOPPED_ON_ERROR = 9
	}

	// State of a set of glasses. 
	class XRRigState {
		public bool available = false;
		public bool attemptingToReserve = false;
		public bool reserved = false;

		public T5XRRig rig;
		public T5Def.GameboardType gameboardType;

		public bool CanAttemptToReserve {  get { return available && !attemptingToReserve && !reserved; } }
	}

	XRInterface xrInterface;
	Dictionary<String, XRRigState> glassesDictionary = new();

	public T5ManagerInterface Manager { get; set; }

	public override void _EnterTree()
	{
		base._EnterTree();
		CreateXRInterface();
	}

	public override void _ExitTree()
	{
		DestroyXRInterface();
		base._ExitTree();
	}

	public override void _Notification(int what)
	{
		if (what == NotificationApplicationPaused)
		{
			DestroyXRInterface();
		}
		else if (what == NotificationApplicationResumed)
		{
			CreateXRInterface();

			if(!xrInterface.IsInitialized())
			{
				xrInterface.Initialize();
			}
		}
	}

	private void CreateXRInterface() 
	{
		if (xrInterface != null)
		{
			return;
		}
		xrInterface = ClassDB.Instantiate("TiltFiveXRInterface").As<XRInterface>();

		xrInterface.Set("application_id", T5ProjectSettings.ApplicationID);
		xrInterface.Set("application_version", T5ProjectSettings.ApplicationVersion);
		xrInterface.Set("trigger_click_threshold", T5ProjectSettings.TriggerClickThreshhold);
		xrInterface.Set("debug_logging", T5ProjectSettings.IsDebugLogging);

		XRServer.AddInterface(xrInterface as XRInterface);

		xrInterface.Connect("glasses_event", Callable.From<String, int>(_OnGlassesEvent));
		xrInterface.Connect("service_event", Callable.From<int>(_OnServiceEvent));
	}

	private void DestroyXRInterface() 
	{
		if (xrInterface == null)
		{
			return;
		}
		xrInterface.Disconnect("service_event", Callable.From<int>(_OnServiceEvent));
		xrInterface.Disconnect("glasses_event", Callable.From<String, int>(_OnGlassesEvent));

		if (xrInterface.Get("is_initialized").AsBool())
		{
			xrInterface.Call("uninitialize");
		}
		XRServer.RemoveInterface(xrInterface as XRInterface);
		xrInterface = null;
		glassesDictionary.Clear();
	}

	public override void _Ready()
	{
		base._Ready();

		if(Manager == null)
		{
			GD.PrintErr("T5Interface does not have a manager set.");
			return;
		}

		if(!xrInterface.IsInitialized())
		{
			xrInterface.Initialize();
		}
	}

	void StartDisplay(string glassesID, T5XRRig xrRig) {
		
		xrInterface.Call("start_display", glassesID, xrRig, xrRig.Origin);
		xrRig.Camera.Tracker = $"/user/{glassesID}/head";
		xrRig.Wand.Tracker = $"/user/{glassesID}/wand_1";
	}

	void ProcessGlasses()
	{
		foreach(var entry in glassesDictionary)
		{
			if(entry.Value.CanAttemptToReserve && Manager.ShouldUseGlasses(entry.Key))
			{
				entry.Value.attemptingToReserve = true;
				xrInterface.Call("reserve_glasses", entry.Key, Manager.GetUIDisplayName(entry.Key));
			}
		}
	}

	void _OnGlassesEvent(String glassesID, int eventNum)
	{
		XRRigState xrRigState;
		if(!glassesDictionary.TryGetValue(glassesID, out xrRigState))
		{
			xrRigState = new XRRigState();
			glassesDictionary.Add(glassesID, xrRigState);
		}

		switch ((GlassesEventType)eventNum)
		{
			case GlassesEventType.E_GLASSES_AVAILABLE:
			{ 
				xrRigState.available = true;
				ProcessGlasses();
				break;
			}
			case GlassesEventType.E_GLASSES_UNAVAILABLE:
			{ 
				xrRigState.available = false;
				if(xrRigState.attemptingToReserve)
				{
					xrRigState.attemptingToReserve = false;
					ProcessGlasses();
				}
				break;
			}
			case GlassesEventType.E_GLASSES_RESERVED:
			{ 
				xrRigState.reserved = true;
				xrRigState.attemptingToReserve = false;

				var xrRig = Manager.CreateXRRig(glassesID);

				// instance our scene
				if(xrRig != null) 
				{ 
					xrRig.GlassesID = glassesID;
					xrRigState.rig = xrRig;
					StartDisplay(glassesID, xrRig);
				}
				else
				{
					xrInterface.Call("release_glasses", glassesID);
				}
				break;
			}
			case GlassesEventType.E_GLASSES_DROPPED:
			{ 
				xrRigState.reserved = false;
				xrRigState.attemptingToReserve = false;

				var xrRig = xrRigState.rig;

				if(xrRig != null) { 
					xrInterface.Call("stop_display", glassesID);
					xrRigState.rig = null;
						Manager.ReleaseXRRig(xrRig);

				}
				break;
			}
			case GlassesEventType.E_GLASSES_TRACKING:
			{
				var gbt = xrInterface.Call("get_gameboard_type", glassesID).As<T5Def.GameboardType>();
				if(xrRigState.gameboardType != gbt)
				{
					xrRigState.gameboardType = gbt;
					if(xrRigState.rig != null)
					{
						xrRigState.rig.GameboardType = gbt;
						xrRigState.rig.GameboardSize = xrInterface.Call("get_gameboard_extents", (int)gbt).AsAabb();
						Manager.SetGameboardType(xrRigState.rig, gbt);
					}
				}
				break;
			}
			// This is the only set of events that needs to be handled
			default: break;
		}

	}

	void _OnServiceEvent(int eventNum)
	{
		switch((ServiceEventType)eventNum)
		{
			case ServiceEventType.E_SERVICE_STOPPED:
			{
				Manager.ServiceStopped();
				break;
			}
			case ServiceEventType.E_SERVICE_RUNNING:
			{
				Manager.ServiceStarted();
				break;
			}
			case ServiceEventType.E_SERVICE_T5_UNAVAILABLE:
			{
				Manager.ServiceUnavailable();
				break;
			}
			case ServiceEventType.E_SERVICE_T5_INCOMPATIBLE_VERSION:
			{
				Manager.ServiceIncorrectVersion();
				break;
			}
		}
	}


}
