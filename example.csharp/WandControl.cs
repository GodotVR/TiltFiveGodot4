using Godot;
using System;
using System.Collections.Generic;
using System.Threading;

public partial class WandControl : T5ControllerCS
{
	static Dictionary<StringName, NodePath> buttons = new()
	{
		{"button_a", "Controls/A" },
		{"button_b", "Controls/B" },
		{"button_x", "Controls/X" },
		{"button_y", "Controls/Y" },
		{"button_1", "Controls/One" },
		{"button_2", "Controls/Two" },
		{"button_3", "Controls/Three" },
		{"button_t5", "Controls/T5" },
		{ "trigger_click", "Controls/TriggerClick" }

	};

	[Export]
	public Material unselected;

	[Export]
	public Material selected;

	Vector3 triggerPos;
	Vector3 stickPos;

	public override void _EnterTree()
	{
		base._EnterTree();
		
		Connect("button_pressed", Callable.From<StringName>(OnButtonPressed));
		Connect("button_released", Callable.From<StringName>(OnButtonReleased));
	}

	public override void _ExitTree()
	{
		Disconnect("button_pressed", Callable.From<StringName>(OnButtonPressed));
		Disconnect("button_released", Callable.From<StringName>(OnButtonReleased));

		base._ExitTree();
	}

	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		base._Ready();
		
		foreach (var entry in buttons)
		{
			var node = GetNodeOrNull<MeshInstance3D>(entry.Value);
			if (node != null)
				node.MaterialOverride = unselected;
		}
		var triggerNode = GetNodeOrNull<MeshInstance3D>("Controls/Trigger");
		if (triggerNode != null)
		{
			triggerNode.MaterialOverride = unselected;
			triggerPos = triggerNode.Transform.Origin;
		}
		var stickNode = GetNodeOrNull<MeshInstance3D>("Controls/Three");
		if (stickNode != null)
		{
			stickNode.MaterialOverride = unselected;
			stickPos = stickNode.Transform.Origin;
		}
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		base._Process(delta);
		
		var triggerNode = GetNodeOrNull<MeshInstance3D>("Controls/Trigger");
		if(triggerNode != null)
		{
			var triggerValue = GetFloat("trigger") * 0.03f;
			var transform = triggerNode.Transform;
			transform.Origin = triggerPos + new Vector3(0, 0, triggerValue);
			triggerNode.Transform = transform;
		}

		var stickNode = GetNodeOrNull<MeshInstance3D>("Controls/Three");
		if (stickNode != null)
		{
			var stickValue = GetVector2("stick") * 0.03f;
			var transform = stickNode.Transform;
			transform.Origin = stickPos + new Vector3(stickValue.X, 0, -stickValue.Y);
			stickNode.Transform = transform;
		}
	}

	public void OnButtonPressed(StringName name)
	{
		if(buttons.TryGetValue(name, out var button))
		{
			var node = GetNodeOrNull<MeshInstance3D>(button);
			if (node != null) node.MaterialOverride = selected;
		}
		if(name == "button_1") {
			TriggerHapticPulse(1,100);
		}
		else if(name == "button_2") {
			TriggerHapticPulse(1,50);
		}
	}

	public void OnButtonReleased(StringName name)
	{
		if (buttons.TryGetValue(name, out var button))
		{
			var node = GetNodeOrNull<MeshInstance3D>(button);
			if (node != null) node.MaterialOverride = unselected;
		}
	}
}
