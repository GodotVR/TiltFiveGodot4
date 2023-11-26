using Godot;
using System;

public partial class AutoScale : Node3D
{
	T5OriginCS origin;
	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		base._Ready();

		var parent = GetParent();
		while(parent != null && !parent.IsClass("T5Origin3D"))
			parent = parent.GetParent();

		if(parent != null)
		{

			origin = parent as T5OriginCS;
			float scale = origin.GameboardScale;
			Scale = new Vector3(scale, scale, scale);
			origin.Connect("gameboard_scale_changed", Callable.From<float>(OnGameboardScaleChanged));
		}
	}

	public override void _ExitTree()
	{
		if(origin != null)
			origin.Disconnect("gameboard_scale_changed", Callable.From<float>(OnGameboardScaleChanged));
		origin = null;
		base._ExitTree();
	}

	public void OnGameboardScaleChanged(float scale)
	{
		Scale = new Vector3(scale, scale, scale);
	}
}
