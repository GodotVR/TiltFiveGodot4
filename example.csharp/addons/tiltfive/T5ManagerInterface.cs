// Interface for all T5Managers. Should not be used directly.
//
// Classes derived from T5ManagerBase implement these voidtions
// to customize the process of connecting the XR rigs in the scene
// to the Tilt Five glasses hardware that is found.
//
// These voidtions must be overridden 
//
// create_glasses_scene
// release_glasses_scene
// get_glasses_scene_viewport
// get_glasses_scene_origin
// get_glasses_scene_camera
// get_glasses_scene_wand
//
// The derived node should be persistent.

public interface T5ManagerInterface
{

    // Invoked by T5Interface when the Tilt Five service has started
    public void ServiceStarted();

    // Invoked by T5Interface when the Tilt Five service has stopped
    public void ServiceStopped();

    // Invoked by T5Interface when the Tilt Five service is not available
    // The driver might not be installed
    public void ServiceUnavailable();

    // Invoked by T5Interface when the Tilt Five service installed does
    // not meet the minimum version requirements
    public void ServiceIncorrectVersion();

    // Invoked by the T5Interface to find out if the glasses should be used in
    // game
    public bool ShouldUseGlasses(string glassesID);

    // Invoked by the T5Interface to get the XR rig scene to be associated with 
    // tilt five glasses. This scene should contain a SubViewport -> T5Origin -> Camera3D and T5Controller3D(s)
    public T5XRRig CreateXRRig(string glassesID);

    // Invoked by the T5Interface if the Tilt Five glasses become unavailable
    public void ReleaseXRRig(T5XRRig xrRig);

    // Invoked by the T5Interface to set the gameboard type the Tilt Five glasses detected
    public void SetGameboardType(T5XRRig rig, T5Def.GameboardType gameboard_type);
}

