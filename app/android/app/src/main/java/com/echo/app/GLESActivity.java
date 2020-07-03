package com.echo.app;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;

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
        // Internal storage
        String resDir = Environment.getDataDirectory().getAbsolutePath() + "/res/";

        try{
            String[] files = getAssets().list("");
            for(String fileName : files){
                InputStream in = getAssets().open(fileName);
                OutputStream out = new FileOutputStream(new File(resDir, fileName));

                copyFile(in, out);

                in.close();
                out.flush();
                out.close();
            }
        } catch (IOException e) {
            //Log.e("tag", "Failed to get asset file list.", e);
        }
    }

    // Copy file
    private void copyFile(InputStream in, OutputStream out) {
        try{
            byte[] buffer = new byte[1024];
            int bytes;
            while ((bytes = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytes);
            }
        } catch (IOException e){
            //Log.e("tag", "Failed to get asset file list.", e);
        }
    }
}
