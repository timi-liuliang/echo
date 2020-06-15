package com.echo.app;

import android.support.v7.app.AppCompatActivity;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends NativeActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Hide toolbar
        setImmersiveSticky();
    }

    protected void onResume() {
        super.onResume();
        setImmersiveSticky();
    }

    protected void onPause() {
        super.onPause();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    // Out popup window, you will call it from C/C++ code later
    void setImmersiveSticky(){
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        );
    }
}
