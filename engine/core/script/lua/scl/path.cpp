//  Created by caolei on 2016/3/9.

#include "scl/path.h"

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/file.h"

#if defined(SCL_APPLE)
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFUtilities.h>
#endif

#if defined(SCL_WIN)
#include <Windows.h>
#endif

#include <stdlib.h>
#include <string.h>

namespace scl {

void get_application_path(char* out, const int outlen)
{
#if defined(SCL_APPLE_IOS)
	CFBundleRef mainBundle	= CFBundleGetMainBundle();
	CFURLRef url			= CFBundleCopyBundleURL(mainBundle);
	CFStringRef path		= CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	int pathlen				= static_cast<int>(CFStringGetLength(path));
	if (outlen - 1 <= pathlen)
	{
		CFRelease(path);
		CFRelease(url);
		return;
	}
	CFStringGetCString(path, out, outlen, kCFStringEncodingUTF8);
	strcat(out, "/");
	
	CFRelease(path);
	CFRelease(url);
#endif

#if defined(SCL_WIN)
	::GetModuleFileNameA(NULL, out, outlen);
	pstring s(out, outlen);
	extract_path(s);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE_MAC)
	readlink("/proc/self/exe", out, outlen);
#endif

#if defined(SCL_ANDROID)
	//TODO	android app is a entire apk, so there is no real path for .bin/.exe.
	//		so we leave it empty
#endif
}

void get_home_path(char* out, const int outlen)
{
#if defined(SCL_APPLE_IOS)
    CFURLRef    url    = CFCopyHomeDirectoryURL();
    CFStringRef path    = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    CFStringGetCString(path, out, outlen, kCFStringEncodingUTF8);
   	CFRelease(path);
	CFRelease(url);
#endif
    
#if defined(SCL_LINUX) || defined(SCL_APPLE_MAC)
	char* env = getenv("HOME");
	if (NULL == env)
		return;
	if (outlen - 1 <= strlen(env))
		return;
	::strcpy(out, env);
	::strcat(out, "/");
#endif

#if defined(SCL_WIN)
	int r = GetCurrentDirectoryA(outlen, out);
	if (r == 0)
	{
		assertf(0, "errno = %d", GetLastError());
	}
#endif

#if defined(SCL_ANDROID)
	assert(0);
#endif

}
	
#if defined(SCL_APPLE)
#define SCL_DOC_DIR "/Documents/"
#endif
	
void get_document_path(char* out, const int outlen)
{
#if defined(SCL_APPLE)
	char*	env		= getenv("HOME");
	if (NULL == env)
		return;
	int		slen	= static_cast<int>(strlen(env));
	if (outlen <= slen + sizeof(SCL_DOC_DIR))
		return;
	::strcpy(out, env);
	::strcat(out, SCL_DOC_DIR);
#endif

#if defined(SCL_WIN) || defined(SCL_LINUX)
	get_home_path(out, outlen);
#endif

#if defined(SCL_ANDROID)
	assert(0);
#endif
}
	
void get_application_path_filename	(char* out, const int outlen, const char* const filename)
{
	get_application_path(out, outlen);
	size_t pathlen = strlen(out);
	size_t leftlen = static_cast<size_t>(outlen) - strlen(out);
	if (pathlen == 0)
		return;
	if (leftlen <= strlen(filename))
	{
		memset(out, 0, pathlen);
		return;
	}
#ifdef SCL_WIN
#pragma warning(push)
#pragma warning(disable:4996)
#endif
	strcat(out, filename);
#ifdef SCL_WIN
#pragma warning(pop)
#endif
}
	
void get_document_path_filename	(char* out, const int outlen, const char* const filename)
{
	get_document_path(out, outlen);
	size_t pathlen = strlen(out);
	size_t leftlen = static_cast<size_t>(outlen) - strlen(out);
	if (pathlen == 0)
		return;
	if (leftlen <= strlen(filename))
	{
		memset(out, 0, pathlen);
		return;
	}
#ifdef SCL_WIN
#pragma warning(push)
#pragma warning(disable:4996)
#endif
	strcat(out, filename);
#ifdef SCL_WIN
#pragma warning(pop)
#endif
}


} // namepspace scl
