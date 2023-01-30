#pragma once

#include "engine/core/base/object.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
	// implement by application or dll
	void registerModules();

#ifdef ECHO_GAME_SOURCE
	void registerGameModules();
#endif

	class Module : public Object
	{
		ECHO_VIRTUAL_CLASS(Module, Object);

	public:
		virtual ~Module() {}

		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// Res path
		const String& getResPath();
		void setResPath(const String& resPath) { m_resPath = resPath; }

        // register all types of this module
		virtual void registerTypes() {}

        // update this module
		virtual void update(float elapsedTime) {}

		// enable
		virtual void setEnable(bool isEnable) { m_isEnable = isEnable; }
		bool isEnable() const { return m_isEnable; }

		// is for editor
		virtual bool isEditorOnly() { return false; }

	public:
		// get all modules
		static vector<Module*>::type* getAllModules();

        // add module by type
        template<typename T> static void addModule(const char* name, const char* moduleFile);
        
        // add module
		static void addModule(Module* module);

		// register all module types
		static void registerAllTypes();

		// get current register module name
		static const char* getCurrentRegisterModuleName();

		// update all modules every frame(ms)
		static void updateAll(float elapsedTime);
        
        // clear all
        static void clear();

	protected:
		String			m_name;
		String			m_resPath;
		bool			m_isEnable = true;
	};
    
    // add module by type
    template<typename T> void Module::addModule(const char* name, const char* moduleFile)
    {
		Class::registerType<T>();

		Echo::Module* module = ECHO_DOWN_CAST<Module*>(Echo::Class::create(name));
        module->setName(name);

		String resPath = moduleFile;
		PathUtil::FormatPath(resPath);
		resPath = PathUtil::GetFileDirPath(resPath) + "res/";
		module->setResPath(resPath);
        
        Echo::Module::addModule(module);
    }
}

#define DECLARE_MODULE(T, F) \
void LoadModule_##T() \
{ \
	Echo::Module::addModule<T>(#T, F); \
}

#define REGISTER_MODULE(T) \
extern void LoadModule_##T(); \
LoadModule_##T(); \
    