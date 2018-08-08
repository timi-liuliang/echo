#pragma once

namespace Examples
{
	Application* g_app = NULL;
	LORD::vector<ExampleFactory*>::type	g_exampleFactorys;
}

static void GetClientSizeImpl(int32_t& width, int32_t& height)
{

}

static void keyboardProcImpl(LORD::ui32 keyChar, bool isKeyDown)
{

}

static void PFSMountImpl(LORD::String path, LORD::String internalPath)
{
	// PFS Mount
	std::wstring rootPath         = s2ws(path.c_str());
	std::wstring internalRootPath = s2ws(internalPath.c_str());

	PFS::CEnv::Mount(L"/root", rootPath, PFS::FST_NATIVE);
	PFS::CEnv::Mount(L"/internal_root", internalRootPath, PFS::FST_RUNZIP);
}