#pragma once

#include "engine/core/util/StringUtil.h"

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
        
        // load plugin by path
        static void loadPluginInPath(const String& pluginDir);

	private:
		String			m_path;
		void*			m_handle;
	};
}
