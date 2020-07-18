package com.echo.app;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class GLESJniLib {

    // Load library
    static {
        System.loadLibrary("echojni");
    }

    // init
    public static native void initRes(String resDirectory, String userDirectory);
    public static native void initEngine(int width, int height);

    // event
    public static native void onTouch(int type, int buttonId, float x, float y);

    // tick
    public static native void tick();

    // install
    public static void install(Context context, AssetManager assets) {
        // https://imnotyourson.com/which-storage-directory-should-i-use-for-storing-on-android-6/
        String fromResDir = "res/";
        String toResDir = context.getFilesDir().getAbsolutePath() + "/res/";
        String toUserDir = context.getFilesDir().getAbsolutePath() + "/user/";

        copyFolder(assets, fromResDir, toResDir);
        GLESJniLib.initRes( toResDir, toUserDir);
    }

    // copy folder
    private static void copyFolder(AssetManager assets, String fromDir, String toDir) {
        try{
            String[] files = assets.list(formatPath(fromDir));
            for(String fileName : files){
                if (fileName.contains(".")){
                    copyFile( assets,fromDir + fileName, toDir + fileName);
                } else {
                    copyFolder( assets,fromDir + fileName + "/", toDir + fileName + "/");
                }
            }
        } catch (IOException e) {
            Log.e("Echo", "Failed to get asset file list.", e);
        }
    }

    // copy file
    private static void copyFile(AssetManager assets, String fromPath, String toPath) {
        try {
            // input stream
            InputStream in = assets.open(fromPath);

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

    // format path, remove last '/' (HUAWEI NEM-AL10)
    private static String formatPath(String fromDir){
        if(fromDir.length()>0 && fromDir.charAt(fromDir.length()-1) == '/') {
            return fromDir.substring(0, fromDir.length()-1);
        }

        return fromDir;
    }
}
