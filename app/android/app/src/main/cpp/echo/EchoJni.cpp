#include <jni.h>
#include <string>
#include "App.h"

// jstring to String
Echo::String jstring2string(JNIEnv *env, jstring jStr)
{
    const char *cstr = env->GetStringUTFChars(jStr, NULL);
    Echo::String str = std::string(cstr);
    env->ReleaseStringUTFChars(jStr, str.c_str());
    return str;
}

// init
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initRes(JNIEnv *env, jclass clazz, jstring inResDir, jstring inUserDir)
{
    Echo::String resDir = jstring2string(env, inResDir);
    Echo::String userDir = jstring2string(env, inUserDir);

    Echo::App::instance()->initRes(resDir, userDir);
}


// init
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initEngine(JNIEnv *env, jclass clazz, jint width, jint height)
{
    Echo::App::instance()->initEngine(width, height);
}

// tick
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_tick(JNIEnv *env, jclass clazz)
{
    Echo::App::instance()->tick();
}