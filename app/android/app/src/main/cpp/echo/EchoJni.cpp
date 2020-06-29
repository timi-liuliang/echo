#include <jni.h>
#include <string>
#include "Log.h"
#include <engine/core/main/Engine.h>
#include <engine/core/render/gles/GLES.h>

// init
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_init(JNIEnv *env, jclass clazz, jint width, jint height)
{
    // Log
    Echo::Log::instance()->addOutput(EchoNew(Echo::GameLog("Game")));

    Echo::initRender(0, width, height);
    Echo::initEngine( "", true);
}

// tick
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_tick(JNIEnv *env, jclass clazz)
{
    Echo::Engine::instance()->tick(0.1f);
}