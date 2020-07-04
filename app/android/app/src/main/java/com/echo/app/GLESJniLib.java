package com.echo.app;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class GLESJniLib {

    // Load library
    static {
        System.loadLibrary("echojni");
    }

    //  set dirs
    public static native void setDirs(String resDirectory, String userDirectory);

    // Init
    public static native void init(int width, int height);

    // Tick
    public static native void tick();
}
