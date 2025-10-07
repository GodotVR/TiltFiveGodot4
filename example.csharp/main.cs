using Godot;
using System;

public partial class main : Node3D
{
	TextureRect cameraView;
	Image cameraImage;
	ImageTexture cameraTexture;
	T5ImageCaptureCS imageCapture;
	bool isCapturing = false;
	Vector2I currentImageSize = Vector2I.Zero;

	public override void _Ready() {
		cameraView = GetNode<TextureRect>("ScreenUI/CameraView");
	}

	public override void _Process(double delta)
	{
		if (imageCapture != null && isCapturing)
		{
			if (imageCapture.acquireBuffer())
			{
				byte[] imageData = imageCapture.getImageData();
				Vector2I imageSize = imageCapture.getImageSize();

				if(cameraImage == null || imageSize != currentImageSize)
				{
					cameraImage = Image.CreateFromData(imageSize.X, imageSize.Y, false, Image.Format.R8, imageData);
					cameraTexture = ImageTexture.CreateFromImage(cameraImage);
					cameraView.Texture = cameraTexture;
					currentImageSize = imageSize;
				}
				else
				{
					cameraImage.SetData(imageSize.X, imageSize.Y, false, Image.Format.R8, imageData);
					cameraTexture.Update(cameraImage);
				}
				imageCapture.releaseBuffer();
			}
		}
	}

	public override void _Input(InputEvent evt)
	{
		if (evt.IsActionPressed("toggle_camera") && imageCapture != null)
		{
			if (!isCapturing && imageCapture.startCapture())
			{
				isCapturing = true;
				cameraView.Visible = true;
			}
			else if (isCapturing)
			{
				imageCapture.stopCapture();
				isCapturing = false;
				cameraView.Visible = false;
			}
		}
	}

	private void _on_t_5_manager_xr_rig_was_added(SubViewport rig)
	{
		var rigImageCapture = rig.GetNode<T5ImageCaptureCS>("Origin/T5ImageCapture");
		if(imageCapture == null) {
			imageCapture = rigImageCapture;
		}
	}

	private void _on_t_5_manager_xr_rig_will_be_removed(SubViewport rig)
	{
		var rigImageCapture = rig.GetNode<T5ImageCaptureCS>("Origin/T5ImageCapture");
		if(imageCapture != null && imageCapture == rigImageCapture) {
			if(isCapturing)
			{
				imageCapture.stopCapture();
				isCapturing = false;
				cameraView.Visible = false;
			}
			imageCapture = null;
		}
	}
}
