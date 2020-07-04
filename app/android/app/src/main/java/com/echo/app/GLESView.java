package com.echo.app;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.util.Log;

import java.io.File;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

public class GLESView extends GLSurfaceView {

    // constructor
    public GLESView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, stencil);
    }

    // init
    private void init(boolean translucent, int depth, int stencil){

        /* By default, GLSurfaceView() creates a RGB_565 opaque surface.
        *  If we want a translucent one, we should change the surface's
        *  format here, using PixelFormat.TRANSLUCENT for GL Surfaces.
        *  is interpreted as any 32-bit surface with alpha by SurfaceFlinger
        */
        if(translucent){
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }

        /* Setup the context factory for 2.0 rendering
         * See ContextFactory class definition below
         */
        setEGLContextFactory(new ContextFactory());

        /* We need to choose an EGLConfig that matches the format of our
         * surface exactly. This is going to be done in our custom config
         * chooser. See ConfigChooser class definition below
         */
        setEGLConfigChooser(translucent ? new ConfigChooser( 8, 8, 8, 8, depth, stencil) : new ConfigChooser(5, 6, 5, 0, depth, stencil));

        /* Set the renderer responsible for frame rendering */
        setRenderer( new Renderer());
    }

    /**
     * Custom context factory
     */
    private static class ContextFactory implements GLSurfaceView.EGLContextFactory {

        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

        // Create Context
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig){
            checkEglError("Before eglCreate Context", egl);
            int[] attribList = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attribList);
            checkEglError("After eglCreateContext", egl);

            return context;
        }

        // Destroy context
        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context){
            egl.eglDestroyContext(display, context);
        }

        // check EglError
        private static void checkEglError(String prompt, EGL10 egl){
            int error;
            while((error = egl.eglGetError()) != EGL10.EGL_SUCCESS){
                Log.e("GLESView", String.format("%s : EGL error: 0x%x", prompt, error));
            }
        }
    }

    /**
     * Custom Config Chooser
     */
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {

        // Subclasses can adjust these values
        protected int m_redSize;
        protected int m_greenSize;
        protected int m_blueSize;
        protected int m_alphaSize;
        protected int m_depthSize;
        protected int m_stencilSize;
        private int[] m_value = new int[1];

        // Constructor
        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
            m_redSize = r;
            m_greenSize = g;
            m_blueSize = b;
            m_alphaSize = a;
            m_depthSize = depth;
            m_stencilSize = stencil;
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

            int EGL_OPENGL_ES2_BIT = 4;
            int[] configAttribs = {
                EGL10.EGL_RED_SIZE, 4,
                EGL10.EGL_GREEN_SIZE, 4,
                EGL10.EGL_BLUE_SIZE, 4,
                EGL10.EGL_RENDERABLE_TYPE,
                EGL_OPENGL_ES2_BIT,
                EGL10.EGL_NONE
            };

            // Get the number of minimally matching EGL configurations
            int[] numConfig = new int[1];
            egl.eglChooseConfig(display, configAttribs, null, 0, numConfig);

            // Check num configs
            int numConfigs = numConfig[0];
            if(numConfigs <=0){
                throw new IllegalArgumentException("No configs match configSpec");
            }

            // Allocate then read the array of minimally matching EGL configs
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, configAttribs, configs, numConfigs, numConfig);

            return chooseConifgInternal(egl, display, configs);
        }

        public EGLConfig chooseConifgInternal(EGL10 egl, EGLDisplay display, EGLConfig[] configs) {

            for(EGLConfig config : configs) {
                int d = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (d < m_depthSize || s < m_stencilSize)
                    continue;

                // We want an *exact* match for red/green/blue/alpha
                int r = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0);

                if (r == m_redSize && g == m_greenSize && b == m_blueSize && a == m_alphaSize)
                    return config;
            }

            return null;
        }

        // Find attrib value
        private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue) {
            if (egl.eglGetConfigAttrib(display, config, attribute, m_value)) {
                return m_value[0];
            }
            return defaultValue;
        }
    }

    /**
     * Custom Renderer
     */
    private static class Renderer implements GLSurfaceView.Renderer {

        // On surface changed
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLESJniLib.init( width, height);
        }

        // On surface created
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        }

        // On draw frame
        public void onDrawFrame(GL10 gl) {
            GLESJniLib.tick();
        }
    }
}
