package com.echo.app;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class GLESJniLib {

    // Load library
    static {
        System.loadLibrary("echojni");
    }

    // Init
    public static native void init(int width, int height, String resDirectory, String userDirectory);

    // Tick
    public static native void tick();
}
