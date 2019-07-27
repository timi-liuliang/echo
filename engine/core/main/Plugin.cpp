#include "Engine.h"
#include "Plugin.h"
#include "PluginSettings.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/DataStream.h"
#include <nlohmann/json.hpp>
#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef ECHO_PLATFORM_WINDOWS
#define DYNLIB_HANDLE         HMODULE
#define DYNLIB_LOAD( a)       LoadLibraryEx( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#define DYNLIB_GETSYM( a, b)  GetProcAddress( a, b)
#define DYNLIB_UNLOAD( a)     FreeLibrary( a)
#else
#include <dlfcn.h>
#define DYNLIB_HANDLE                       void*
#define DYNLIB_LOAD( a)                     dlopen( a, RTLD_LOCAL|RTLD_LAZY)
#define DYNLIB_GETSYM( handle, symbolName)  dlsym( handle, symbolName)
#define DYNLIB_UNLOAD( a)                   dlclose( a)
#endif

namespace Echo
{
	struct PluginConfig 
	{
		i32			m_version = 0;
		String		m_name;
        String      m_pluginPath;
		String		m_libraryPath;
		StringArray	m_dependLibraryPaths;
		bool		m_loaded = false;

		PluginConfig(const String& pluginPath)
		{
		#ifdef ECHO_PLATFORM_WINDOWS
			String currentOS = "Windows";
		#else
			String currentOS = "Mac";
		#endif

			std::ifstream memReader(pluginPath.c_str());
			if (memReader.is_open())
			{
				m_pluginPath = PathUtil::GetFileDirPath(pluginPath);

				using namespace nlohmann;
				json j;
				memReader >> j;

				if (j.find("Version") != j.end())
				{
					m_version = j["Version"].get<i32>();
				}

				if (j.find("Name") != j.end())
				{
					m_name = j["Name"].get<std::string>().c_str();
				}

				nlohmann::json& library = j["Library"];
				for (i32 i = 0; i < library.size(); i++)
				{
					String os = library[i]["OS"].get<std::string>().c_str();
					if (os == currentOS)
					{
                        m_libraryPath = library[i]["Path"].get<std::string>().c_str();
                        m_libraryPath = StringUtil::Replace(m_libraryPath, "${PluginDir}", m_pluginPath);
                        m_libraryPath = StringUtil::Replace(m_libraryPath, "//", "/");
						m_dependLibraryPaths = StringUtil::Split( library[i]["DependLibraryPaths"].get<std::string>().c_str(), ";");
					}
				}

				m_loaded = true;
			}
		}

		// is loaded succeed
		bool isLoaded() const { return m_loaded; }
        
        // link depend library path
        void linkDependLibrary()
        {
        #ifdef ECHO_PLATFORM_WINDOWS
            for (String dllPath : m_dependLibraryPaths)
            {
                dllPath = StringUtil::Replace(dllPath, "${PluginDir}", m_pluginPath);
                dllPath = StringUtil::Replace(dllPath, "//", "/");
                ::SetDllDirectory(dllPath.c_str());
            }
        #endif
        }
	};

	// plugins
	static map<String, Plugin*>::type	g_plugins;

	Plugin::Plugin()
	{

	}

	Plugin::~Plugin()
	{

	}

	bool Plugin::load(const char* path)
	{
		m_handle = DYNLIB_LOAD(path);
		if (!m_handle)
		{
			EchoLogError("Load Plugin [%s] failed", path);
			return false;
		}

		return true;
	}

	void Plugin::unload()
	{
		DYNLIB_UNLOAD((DYNLIB_HANDLE)m_handle);
	}

	void* Plugin::getSymbol(const char* symbolName)
	{
		return (void*)DYNLIB_GETSYM((DYNLIB_HANDLE)m_handle, symbolName);
	}

	void Plugin::loadAllPlugins()
	{
        StringArray pluginPaths = StringUtil::Split(PluginSettings::instance()->getSearchPath(), ";");
        for(String& path : pluginPaths)
        {
            path = StringUtil::Replace(path, "${EchoDir}", PathUtil::GetCurrentDir());
            path = StringUtil::Replace(path, "${ProjectDir}", Engine::instance()->getResPath());
            path = StringUtil::Replace(path, "//", "/");
            PathUtil::FormatPath(path, false);
            
            loadPluginInPath( path);
        }
	}
    
    void Plugin::loadPluginInPath(const String& pluginDir)
    {
    #ifdef ECHO_EDITOR_MODE
        typedef bool(*LOAD_PLUGIN_FUN)();
        
        // get all plugins
        StringArray plugins;
        PathUtil::EnumFilesInDir( plugins, pluginDir, false, true, true);
        
        // iterate
        for (const String& pluginPath : plugins)
        {
            String ext = PathUtil::GetFileExt(pluginPath, true);
            if(ext==".plugin")
            {
                PluginConfig pluginCfg(pluginPath);
                if (pluginCfg.isLoaded())
                {
                    String name = pluginCfg.m_name;
                    String symbolName = StringUtil::Format("load%sPlugin", name.c_str());
                    
                    // link depend libray before load dll
                    pluginCfg.linkDependLibrary();
                    
                    Plugin* plugin = EchoNew(Plugin);
                    if (plugin->load(pluginCfg.m_libraryPath.c_str()))
                    {
                        LOAD_PLUGIN_FUN pFunc = (LOAD_PLUGIN_FUN)plugin->getSymbol(symbolName.c_str());
                        if (pFunc)
                            (*pFunc)();
                        else
                            EchoLogError("Can't find symbol %s in plugin [%s]", symbolName.c_str(), pluginPath.c_str());
                    }
                    
                    g_plugins[name] = plugin;
                }
            }
        }
    #endif
    }
}
