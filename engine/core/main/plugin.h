#pragma once

#include "engine/core/util/StringUtil.h"

#ifdef ECHO_PLATFORM_WINDOWS
	#define DYNLIB_HANDLE         HMODULE
#else
	#define DYNLIB_HANDLE		  void*
#endif

namespace Echo
{
	class Plugin
	{
	public:
		Plugin();
		~Plugin();

		// load/unload
		bool load(const char* path);
		void unload();

		// get symbol
		void* getSymbol(const char* symbolName);

	public:
		// load all plugins
		static void loadAllPlugins();

	private:
		String			m_path;
		DYNLIB_HANDLE	m_handle;
	};
}
