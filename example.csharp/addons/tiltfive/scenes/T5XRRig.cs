using Godot;
using System;

public partial class T5XRRig : SubViewport
{
	T5OriginCS origin;
	T5CameraCS camera;
	T5ControllerCS wand;

	public string GlassesID { get; set; } 
	public T5Def.GameboardType GameboardType { get; set; }
	public Aabb GameboardSize { get; set; }
	public T5OriginCS Origin {  get { return origin; } }
	public T5CameraCS Camera{ get { return camera; } }
	public T5ControllerCS Wand { get { return wand; } }
	
	// Returns the friendly name of the glasses defined in the Tilt Five control panel
	public string GlassesName { 
		get {
			var t5Interface = GetNode<T5Interface>("/root/T5Interface");
			return t5Interface?.GetGlassesName(GlassesID) ?? "";
		}
	}

	// Called when the node enters the scene tree for the first time.
	public override void _EnterTree()
	{
		base._EnterTree();

		origin = GetNode<T5OriginCS>("Origin");
		camera = GetNode<T5CameraCS>("Origin/Camera");
		wand = GetNode<T5ControllerCS>("Origin/Wand_1");
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		if(wand != null) wand.Visible = wand.getHasTrackingData();
	}
}
