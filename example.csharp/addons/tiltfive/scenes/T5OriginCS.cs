using Godot;
using System;

public partial class T5OriginCS : Node3D
{
	public float GameboardScale 
	{
		get
		{
			return Get("gameboard_scale").As<float>();
		}	
		set
		{
			Set("gameboard_scale", value);
		}
	}
}
