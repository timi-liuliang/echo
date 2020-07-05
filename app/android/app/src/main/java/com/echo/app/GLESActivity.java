package com.echo.app;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;

public class GLESActivity extends Activity {

    GLESView m_view;

    // on Create
    @Override protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);

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
