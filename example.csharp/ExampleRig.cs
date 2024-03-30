using Godot;
using System;

public partial class ExampleRig : T5XRRig
{
	Node3D pivot;

	public override void _Ready()
	{
		pivot = GetNode<Node3D>("Origin/Pivot");
		var label = GetNode<Label3D>("Origin/Pivot/GlassesName");

		var timer = new Timer();
		timer.Autostart = true;
		timer.WaitTime = 1.0;
		AddChild(timer);
		timer.Timeout += () => label.Text = GlassesName;
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		var pos = Camera.GlobalPosition;
		if(pivot.GlobalPosition.DistanceTo(pos) < 0.01) return;
		pivot.LookAt(new Vector3(pos.X, pivot.GlobalPosition.Y, pos.Z), Vector3.Up, true);
	}
}
