#include <jni.h>
#include <string>
#include "Log.h"
#include <engine/core/main/Engine.h>
#include <engine/core/render/gles/GLES.h>

// jstring to String
Echo::String jstring2string(JNIEnv *env, jstring jStr)
{
    const char *cstr = env->GetStringUTFChars(jStr, NULL);
    Echo::String str = std::string(cstr);
    env->ReleaseStringUTFChars(jStr, str.c_str());
    return str;
}

// init
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_init(JNIEnv *env, jclass clazz, jint width, jint height, jstring inResDir, jstring inUserDir)
{
    Echo::String resDir = jstring2string(env, inResDir);
    Echo::String userDir = jstring2string(env, inUserDir);

    // Log
    Echo::Log::instance()->addOutput(EchoNew(Echo::GameLog("Game")));

    // Initialize
    Echo::initRender(0, width, height);
    Echo::initEngine( resDir + "app.echo", true);
}

// tick
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_tick(JNIEnv *env, jclass clazz)
{
    Echo::Engine::instance()->tick(0.1f);
}