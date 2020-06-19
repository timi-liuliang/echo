package com.echo.app;

public class GLESJniLib {

    // load library
    static {
        System.loadLibrary("echojni");
    }

    // init
    public static native void init(int width, int height);

    // tick
    public static native void tick();
}
