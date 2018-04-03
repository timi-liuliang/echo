#include "android.h"

#ifdef __ANDROID__

#include <string.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/log.h>
//#define mylog(s, ...) __android_log_print(ANDROID_LOG_INFO, "scl", s, ##__VA_ARGS__)
#define mylog(s, ...) 

#include "scl/thread.h"

namespace scl {

//static JNIEnv*			g_android_env			= 0;
static JavaVM*			g_android_javavm		= 0;
static jobject			g_android_activity		= 0;
//static jclass			g_android_activity_class	= 0;
static AAssetManager*		g_android_asset_manager	= 0;

//void set_android(JavaVM* vm, jobject activity)
//{
//	set_android_javavm	(vm);
//	set_android_env		(env);
//	set_android_activity(activity);
//}

//void set_android_env(JNIEnv* env)	
//{ 
//	if (NULL != g_android_env)
//		return;
//	g_android_env = env; 
//}

class AndroidEnv
{
public:
	JNIEnv* p;
	bool detach;
	AndroidEnv() : p(NULL), detach(false)
	{
		if (NULL == g_android_javavm)
			return;
		JavaVM* vm = g_android_javavm;
		
		jint r = vm->GetEnv((void**)&p, JNI_VERSION_1_4);
		if (r == JNI_EDETACHED)
		{
			detach = true;
			r = vm->AttachCurrentThread(&p, NULL);
			if (r != 0)
				p = NULL;
		}
		else if (r == JNI_EVERSION)
			p = NULL;
	}
	~AndroidEnv()
	{
		if (NULL != p && detach && NULL != g_android_javavm)
			g_android_javavm->DetachCurrentThread();
	}
};

//JNIEnv* attach_android_env()				
//{ 
//	if (NULL == g_android_javavm)
//		return NULL;
//	JNIEnv* env = NULL;
//	g_android_javavm->AttachCurrentThread(&env, NULL);
//	return env;
//}

void set_android_javavm(JavaVM* vm)
{
	g_android_javavm = vm;

	//JNIEnv* env = NULL;
	//g_android_javavm->AttachCurrentThread(&env, NULL);

	//JNIEnv* env =  attach_android_env();;
	//mylog("scl lord game : set_android_javavm, get env = %x", (uint)env);
}

JavaVM* get_android_javavm()
{
	return g_android_javavm;
}


//void detach_android_env()
//{
//	if (NULL == g_android_javavm)
//		return;
//	g_android_javavm->DetachCurrentThread();
//}


void set_android_activity(jobject o)
{
	if (NULL != g_android_activity)
		return;
	AndroidEnv env;
	//JNIEnv* env = attach_android_env();
	if (NULL == env.p)
		return;
	//jclass cls = env.p->GetObjectClass(obj);
	//g_android_class = (jclass)env.p->NewGlobalRef(cls);
	g_android_activity = env.p->NewGlobalRef(o);
	mylog("scl lord game : set_android_activity, activity = %x", (uint)g_android_activity);
	//detach_android_env();
	//mylog("scl lord game : set_android_activity, over", (uint)g_android_activity);
}

jobject get_android_activity()
{
	return g_android_activity;
}

/*
void set_android_asset_manager(jobject o, JNIEnv* _env)
{
	JNIEnv* e = _env;
	if (NULL == e)
		e = g_android_env;
	g_asset_manager = AAssetManager_fromJava(e, o);
}
*/
jobject _call_class_func(jobject obj, const char* const funcname, const char* const funcsig)
{
	AndroidEnv _e;
	JNIEnv* env = _e.p;
	if (NULL == env)
		return NULL;

	jclass clazz 	= env->GetObjectClass	(obj);
	jmethodID func 	= env->GetMethodID	(clazz, funcname, funcsig);
	jobject ret	= env->CallObjectMethod	(obj, func);
	//env->CallVoidMethod(obj, func);

	return ret;
}
jobject _call_activity_func(const char* const funcname, const char* const funcsig)
{
	_call_class_func(g_android_activity, funcname, funcsig);
}

struct AAssetManager* get_android_asset_manager()
{
	AndroidEnv _e;
	JNIEnv* env = _e.p;
	mylog("scl lord game get_android_asset_manager begin: env = %x, activity = %x, assetmanager = %x", 
		(unsigned int)env, 
		(unsigned int)g_android_activity, 
		(unsigned int)g_android_asset_manager);

	if (NULL == g_android_asset_manager)
	{
		if (NULL == env || NULL == g_android_activity)
			return NULL;
		
		jobject obj				= _call_activity_func("getAssets", "()Landroid/content/res/AssetManager;");
		if (NULL == obj)
		{
			mylog("scl get_android_asset_manager obj = NULL");
		}
		else
		{
			mylog("scl get_android_asset_manager obj = %x", (uint)obj);
		}
		g_android_asset_manager = AAssetManager_fromJava(env, obj);		
	}
	mylog("scl lord game get_android_asset_manager end: env = %x, activity = %x, assetmanager = %x", 
		(unsigned int)env, 
		(unsigned int)g_android_activity, 
		(unsigned int)g_android_asset_manager);
	return g_android_asset_manager;
}


char* jstrcpy(char* dest, jstring str, const int destlen)
{
	AndroidEnv _e;
	JNIEnv* env = _e.p;
	if (NULL == env)
		return NULL;
	const char* cs = env->GetStringUTFChars(str, NULL);
	const int cslen = strlen(cs);
	strncpy(dest, cs, cslen > destlen - 1 ? destlen - 1 : cslen);
	env->ReleaseStringUTFChars(str, cs);
	return dest;
}


void get_android_files_dir(char* out, int len)
{
	jobject obj = _call_activity_func("getFilesDir", "()Ljava/io/File;");
	if (NULL == obj)
		return;
	jstring str = (jstring)_call_class_func(obj, "getAbsolutePath", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	jstrcpy(out, str, len);
}

void	get_android_external_files_dir(char* out, int len)
{
	AndroidEnv _e;
	int tid = scl::thread::self();
	//jclass contextClass = g_android_env->FindClass("android/content/Context");
	//if (NULL == contextClass)
	//	return;
	//jmethodID methodID = g_android_env->GetMethodID(contextClass, "getExternalCache");
	mylog("scl get external files begin... thread id = %d", tid);

	JNIEnv* env 			= _e.p;
	if (NULL == env)
		return;

	//jclass clazz 	= env->GetObjectClass	(g_android_activity);
	//jclass clazz 	= env->FindClass("android/app/NativeActivity");
	jclass clazz 	= env->FindClass("android/content/Context");
	mylog("scl clazz = %x", (uint)clazz);
	if (NULL == clazz)
		return;
	jmethodID func 	= env->GetMethodID		(clazz, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
	mylog("scl func= %x", (uint)func);
	if (NULL == func)
		return;
	mylog("scl env = %x, clazz = %x, func = %x", (uint)env, (uint)clazz, (uint)func);
	jobject obj		= env->CallObjectMethod(g_android_activity, func, NULL);
	if (NULL == obj)
		return;
	mylog("scl external dir jobject = %x", (uint)obj);

	jstring str = (jstring)_call_class_func(obj, "getAbsolutePath", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	mylog("scl external dir jstring = %x", (uint)str);
	jstrcpy(out, str, len);
}

void 	get_android_cache_dir(char* out, int len)
{
	jobject obj = _call_activity_func("getCacheDir", "()Ljava/io/File;");
	if (NULL == obj)
		return;
	jstring str = (jstring)_call_class_func(obj, "getAbsolutePath", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	jstrcpy(out, str, len);
}

void	get_android_external_cache_dir(char* out, int len)
{
	jobject obj = _call_activity_func("getExternalCacheDir", "()Ljava/io/File;");
	if (NULL == obj)
		return;
	jstring str = (jstring)_call_class_func(obj, "getAbsolutePath", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	jstrcpy(out, str, len);
}

void get_android_package_path(char* out, int len)
{
	jstring str = (jstring)_call_activity_func("getPackageResourcePath", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	jstrcpy(out, str, len);
}

void get_android_package_name(char* out, int len)
{
	jstring str = (jstring)_call_activity_func("getPackageName", "()Ljava/lang/String;");
	if (NULL == str)
		return;
	jstrcpy(out, str, len);
}

android_file::android_file() :
	m_file(NULL)
{

}

android_file::~android_file()
{
	close();
}

bool android_file::open(const char* const filename)
{
	mylog("scl lord game android_file::open begin");
	AAssetManager* am = get_android_asset_manager();
	if (NULL == am)
		return false;
	mylog("scl lord game android_file::open asset manager  = %x", (uint)am);
	m_file = AAssetManager_open(am, filename, AASSET_MODE_UNKNOWN);
	mylog("scl lord game android_file::open finished, m_file  = %x", (uint)m_file);
	return m_file != NULL;
}

const void* android_file::get_buffer() const
{
	if (NULL == m_file)
		return NULL;
	return AAsset_getBuffer(m_file);
}

int android_file::size() const
{
	if (NULL == m_file)
		return NULL;
	return AAsset_getLength(m_file);
}

int android_file::read(void* buf, int count)
{
	return AAsset_read(m_file, buf, count);
}

int android_file::seek(int offset, int origin)
{
	return AAsset_seek(m_file, offset, origin);
}

void android_file::close()
{
	if (NULL != m_file)
	{
		AAsset_close(m_file);
		m_file = NULL;
	}
}

} //namespace scl

#endif

