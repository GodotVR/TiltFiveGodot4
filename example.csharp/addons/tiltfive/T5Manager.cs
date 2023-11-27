using Godot;
using System;

[GlobalClass]
public partial class T5Manager : Node, T5ManagerInterface
{
	[Signal]
	public delegate void XRRigWasAddedEventHandler(T5XRRig rig);

	[Signal]
	public delegate void XRRigWillBeRemovedEventHandler(T5XRRig rig);

	[Export]
	public PackedScene xrRigScene;

	[Export]
	public Node3D startLocation;

	Node3D rigs;

	T5Interface t5Interface;

	public override void _EnterTree()
	{
		base._EnterTree();
		
		if (xrRigScene == null)
		{
			xrRigScene = (PackedScene)ResourceLoader.Load("res://addons/tiltfive/scenes/T5XRRig.tscn");
		}

		t5Interface = GetNode<T5Interface>("/root/T5Interface");
		if (t5Interface != null)
		{
			t5Interface.Manager = this;
		}
	}

	public override void _ExitTree()
	{
		if(t5Interface != null)
		{
			t5Interface.Manager = null;
		}
		base._ExitTree();
	}

	public override void _Ready()
	{
		rigs = new Node3D();
		rigs.Name = "T5XRRigs";
		GetTree().Root.CallDeferred(MethodName.AddChild, rigs);
	}

	public void ServiceStarted()
	{
	}

	public void ServiceStopped()
	{
	}

	public void ServiceUnavailable()
	{
		GD.PrintErr("Tilt Five Service is unavailable");
	}

	public void ServiceIncorrectVersion()
	{
		GD.PrintErr("Tilt Five Service version is incompatible");
	}

	public bool ShouldUseGlasses(string glassesID)
	{
		return true;
	}

	public string GetUIDisplayName(string glassesID)
	{
		return T5ProjectSettings.DefaultDisplayName;
	}

	public T5XRRig CreateXRRig(string glassesID)
	{
		var newRig = xrRigScene.Instantiate<T5XRRig>();
		newRig.Name = glassesID;
		rigs.AddChild(newRig);
		if(startLocation != null)
		{
			var origin = newRig.Origin;
			origin.Transform = startLocation.Transform;
			if(startLocation.IsClass("T5Gameboard")) { 
				var contentScale = startLocation.Get("content_scale").As<float>();
				origin.Set("gameboard_scale", contentScale);
			}
		}
		EmitSignal(SignalName.XRRigWasAdded, newRig);
		return newRig;
	}

	public void ReleaseXRRig(T5XRRig xrRig)
	{
		EmitSignal(SignalName.XRRigWillBeRemoved, xrRig);
		rigs.RemoveChild(xrRig);
		xrRig.QueueFree();
	}

	public void SetGameboardType(T5XRRig rig, T5Def.GameboardType gameboard_type)
	{
	}
}
