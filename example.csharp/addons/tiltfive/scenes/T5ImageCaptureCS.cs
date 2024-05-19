using Godot;
using System;

public partial class T5ImageCaptureCS : Node3D
{
	public bool startCapture()
	{
		return Call("start_capture").AsBool();
	}

	public void stopCapture()
	{
		Call("stop_capture");
	}

	public bool acquireBuffer()
	{
		return Call("acquire_buffer").AsBool();
	}

	public void releaseBuffer()
	{
		Call("release_buffer");
	}

	public byte[] getImageData()
	{
		return Call("get_image_data").As<byte[]>();
	}

	public Transform3D getCameraTransform()
	{
		return Call("get_camera_transform").As<Transform3D>();
	}

	public Vector2I getImageSize()
	{
		return Call("get_image_size").As<Vector2I>();
	}

	public int getImageStride()
	{
		return Call("get_image_stride").As<int>();
	}

	public int getFrameIlluminationMode()
	{
		return Call("get_frame_illumination_mode").As<int>();
	}
}
