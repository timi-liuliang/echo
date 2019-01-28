#include "Plugin.h"

#ifdef ECHO_PLATFORM_WINDOWS
#define DYNLIB_HANDLE         HMODULE
#define DYNLIB_LOAD( a)       LoadLibraryEx( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#define DYNLIB_GETSYM( a, b)  GetProcAddress( a, b)
#define DYNLIB_UNLOAD( a)     FreeLibrary( a)
#endif

namespace Echo
{
	Plugin::Plugin()
	{

	}

	Plugin::~Plugin()
	{

	}

	void Plugin::load(const char* path)
	{
		m_handle = DYNLIB_LOAD(path);
	}

	void Plugin::unload()
	{
		DYNLIB_UNLOAD(m_handle);
	}

	void* Plugin::getSymbol(const char* symbolName)
	{
		return (void*)DYNLIB_GETSYM(m_handle, symbolName);
	}

	void Plugin::loadAllPlugins()
	{
#if defined(ECHO_PLATFORM_WINDOWS) && defined(ECHO_EDITOR_MODE)

#endif
	}
}