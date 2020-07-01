package com.echo.app;

public class GLESJniLib {

    // load library
    static {
        System.loadLibrary("echojni");
    }

    // init
    public static native void init(int width, int height, String resDirectory, String userDirectory);

    // tick
    public static native void tick();

    // install
    public static void install(String resDir) {

    }
}
