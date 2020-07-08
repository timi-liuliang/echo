#include <jni.h>
#include <string>
#include "App.h"

// init res directory
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initRes(JNIEnv *env, jclass clazz, jstring inResDir, jstring inUserDir)
{
    //Echo::String resDir = env->GetStringUTFChars(inResDir, NULL);
    //Echo::String userDir = env->GetStringUTFChars(inUserDir, NULL);

   // Echo::App::instance()->initRes(resDir, userDir);
}

// init engine
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initEngine(JNIEnv *env, jclass clazz, jint width, jint height)
{
    Echo::App::instance()->initEngine(width, height);
}

// tick
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_tick(JNIEnv *env, jclass clazz)
{
    Echo::App::instance()->tick();
}