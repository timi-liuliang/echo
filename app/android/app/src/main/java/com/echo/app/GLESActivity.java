package com.echo.app;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Window;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;

import static android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN;

public class GLESActivity extends Activity {

    GLESView m_view;

    // on Create
    @Override protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);

        // https://stackoverflow.com/questions/26543268/android-making-a-fullscreen-application
        getWindow().setFlags(FLAG_FULLSCREEN, FLAG_FULLSCREEN);

        install();

        m_view = new GLESView(getApplication(), false, 0, 0);
        setContentView(m_view);
    }

    // on Pause
    @Override protected void onPause(){
        super.onPause();
        m_view.onPause();
    }

    // on Resume
    @Override protected void onResume() {
        super.onResume();
        m_view.onResume();
    }

    // install
    public void install() {
        GLESJniLib.install(getApplicationContext(), getAssets());
    }
}
