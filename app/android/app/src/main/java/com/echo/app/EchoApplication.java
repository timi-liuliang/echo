package com.echo.app;

import android.app.Application;
import android.util.Log;

public class EchoApplication extends Application {

    public void onCreate() {
        super.onCreate();
        Log.w("native-activity", "onCreate");
    }
}
