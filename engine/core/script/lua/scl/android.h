//
// Unlike iOS's Core Foundation library's great support for C/C++ development
// Android has a very poor support for C/C++
// So, here is the expedient for android's C/C++ development.
//
#pragma once

#ifdef __ANDROID__

#if defined(__cplusplus)
struct _JavaVM;
typedef _JavaVM JavaVM;
struct _JNIEnv;
typedef _JNIEnv JNIEnv;
class _jobject;
typedef _jobject *jobject;
#else
struct JNIInvokeInterface;
typedef const struct JNIInvokeInterface* JavaVM;
struct JNINativeInterface;
typedef const struct JNINativeInterface* JNIEnv;
struct _jobject;
typedef void* jobject;
#endif

 struct AAssetManager;
 struct AAsset;

#ifdef __cplusplus
extern "C" {
namespace scl {
#endif

//JavaVM
void	set_android_javavm(JavaVM* vm);
JavaVM*	get_android_javavm();

//android main activity
void	set_android_activity(jobject o);
jobject get_android_activity();

//android enviroment and main activity
//void	set_android(JavaVM* vm, jobject activity);

//android enviroment
//void	set_android_env(JNIEnv* evn);
JNIEnv*	attach_android_env();	// NOTE!!! before call get_android_env, you must call set_android in JNI interface function!!!
void	detach_android_env();
//android path
void 	get_android_files_dir(char* out, int len);
void	get_android_external_files_dir(char* out, int len);

void 	get_android_cache_dir(char* out, int len);
void	get_android_external_cache_dir(char* out, int len);

void 	get_android_package_path(char* out, int len);
void	get_android_package_name(char* out, int len);

//android asset manager
struct AAssetManager* get_android_asset_manager(); // NOTE!!! before call get_android_asset_manager, you must call set_android() in JNI interface function first!!!

#ifdef __cplusplus

class android_file
{
public:
	android_file();
	~android_file();

	bool		open		(const char* const filename);
	const void*	get_buffer	() const;
	int			size		() const;
	int			read		(void* buf, int count);
	int			seek		(int offset, int origin = 1); //origin = SEEK_SET(0)/SEEK_CUR(1)/SEE_END(2)
	void		close		();

private:
	AAsset* m_file;
};

#endif

#ifdef __cplusplus
} //namespace scl
} //extern "C" 
#endif

#endif

