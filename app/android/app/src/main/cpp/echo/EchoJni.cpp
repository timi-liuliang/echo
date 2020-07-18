#include <jni.h>
#include <string>
#include "App.h"
#include <engine/core/input/input.h>
#include <engine/core/main/Engine.h>

// init res directory https://stackoverflow.com/questions/54433385/art-sigsegv-fault-when-entering-jni-function-in-c-code-within-android-studio
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initRes(JNIEnv *env, jclass clazz, jstring inResDir, jstring inUserDir)
{
    Echo::String resDir = env->GetStringUTFChars(inResDir, NULL);
    Echo::String userDir = env->GetStringUTFChars(inUserDir, NULL);

    Echo::App::instance()->initRes(resDir, userDir);
}

// init engine
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_initEngine(JNIEnv *env, jclass clazz, jint width, jint height)
{
    Echo::App::instance()->initEngine(width, height);
}

// on touch
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_onTouch(JNIEnv *env, jclass clazz, int type, int buttonId, jfloat x, jfloat y)
{
    // MotionEvent.ACTION_DOWN 0
    // MotionEvent.ACTION_UP   1
    // MotionEvent.ACTION_MOVE 2
    switch(type)
    {
        case 0: Echo::Input::instance()->notifyMouseButtonDown(buttonId, Echo::Vector2(x, y));  break;
        case 1: Echo::Input::instance()->notifyMouseButtonUp(buttonId, Echo::Vector2(x, y));    break;
        case 2: Echo::Input::instance()->notifyMouseMove(buttonId, Echo::Vector2(x, y));        break;
        default: break;
    }
}

// tick
extern "C" JNIEXPORT void JNICALL Java_com_echo_app_GLESJniLib_tick(JNIEnv *env, jclass clazz)
{
    Echo::App::instance()->tick();
}