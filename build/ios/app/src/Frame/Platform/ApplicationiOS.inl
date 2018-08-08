#if defined(__APPLE_CC__)
#pragma once

//#include "iOS/iOSExampleMain.h"
void iOSGetClientSizeImpl(int32_t& width, int32_t& height);
void iOSKeyboardProcImpl(LORD::ui32 keyChar, bool isKeyDown);

namespace Examples
{
	Application* g_app = NULL;
	LORD::vector<ExampleFactory*>::type	g_exampleFactorys;
}

static void GetClientSizeImpl(int32_t& width, int32_t& height)
{
	iOSGetClientSizeImpl(width, height);
}

static void keyboardProcImpl(LORD::ui32 keyChar, bool isKeyDown)
{
	//iOSKeyboardProcImpl(keyChar, isKeyDown);
}

static void PFSMountImpl(LORD::String path, LORD::String internalPath)
{
	// PFS Mount
	std::wstring rootPath = s2ws(LORD::PathUtil::GetCurrentDir());
	
	PFS::CEnv::Mount(L"/root", rootPath, PFS::FST_NATIVE);
	PFS::CEnv::Mount(L"/internal_root", rootPath + L"/media/", PFS::FST_NATIVE);

	(void)path;
	(void)internalPath;

}
#endif