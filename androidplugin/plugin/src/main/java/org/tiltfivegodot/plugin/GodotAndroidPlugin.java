package org.tiltfivegodot.plugin;

import com.tiltfive.client.TiltFiveClient;
import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;
import org.godotengine.godot.plugin.UsedByGodot;
import android.util.Log;

public class GodotAndroidPlugin extends GodotPlugin {

    private TiltFiveClient tiltFiveClient;
    private long platformContext = 0;

    public GodotAndroidPlugin(Godot godot) {
        super(godot);
    }

    @Override
    public String getPluginName() {
        return BuildConfig.GODOT_PLUGIN_NAME;
    }

    @Override
    public void onGodotSetupCompleted() {
        super.onGodotSetupCompleted();

        tiltFiveClient = new TiltFiveClient(getActivity().getApplicationContext());
        platformContext = tiltFiveClient.newPlatformContext();    
    }

    @UsedByGodot
    private String getPlatformContextString() {
		return Long.toString(platformContext);
    }

	// Using this from gdscript will cause a crash
    @UsedByGodot
    private long getPlatformContext() {
        return platformContext;
    }
}