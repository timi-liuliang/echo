package com.echo.app;

import android.app.Activity;
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
        String fromResDir = "res/";
        String toResDir = Environment.getDataDirectory().getAbsolutePath() + "/res/";

        copyFolder(fromResDir, toResDir);
    }

    // copy folder
    private void copyFolder(String fromDir, String toDir) {
        try{
            String[] files = getAssets().list(fromDir);
            for(String fileName : files){
                if (fileName.contains(".")){
                    copyFile(fromDir + fileName, toDir + fileName);
                } else {
                    copyFolder(fromDir + fileName + "/", toDir + fileName + "/");
                }
            }
        } catch (IOException e) {
            Log.e("Echo", "Failed to get asset file list.", e);
        }
    }

    // copy file
    private void copyFile(String fromPath, String toPath) {
        try {
            // input stream
            InputStream in = getAssets().open(fromPath);

            // output stream
            File toFile = new File(toPath);
            toFile.getParentFile().mkdirs();
            toFile.createNewFile();
            OutputStream out = new FileOutputStream(toFile, false);

            byte[] buffer = new byte[1024];
            int bytes;
            while ((bytes = in.read(buffer)) != -1) {
                out.write(buffer, 0, bytes);
            }

            in.close();
            out.flush();
            out.close();
        } catch (IOException e){
            Log.e("Echo", "Failed to copy file when install.", e);
        }
    }
}
