package com.echo.app;

import android.app.Activity;
import android.os.Bundle;

public class GLESActivity extends Activity {

    GLESView m_view;

    // on Create
    @Override protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);

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
}
