#include "windows_build_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>

namespace Echo
{
	WindowsBuildSettings::WindowsBuildSettings()
	{

	}

	WindowsBuildSettings::~WindowsBuildSettings()
	{

	}

	WindowsBuildSettings* WindowsBuildSettings::instance()
	{
		static WindowsBuildSettings* inst = EchoNew(WindowsBuildSettings);
		return inst;
	}

	void WindowsBuildSettings::bindMethods()
	{
		CLASS_BIND_METHOD(WindowsBuildSettings, getIconRes, DEF_METHOD("getIconRes"));
		CLASS_BIND_METHOD(WindowsBuildSettings, setIconRes, DEF_METHOD("setIconRes"));

		CLASS_REGISTER_PROPERTY(WindowsBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
	}

	void WindowsBuildSettings::setOutputDir(const String& outputDir)
	{
		m_outputDir = outputDir;
		PathUtil::FormatPath(m_outputDir, false);
	}

	bool WindowsBuildSettings::prepare()
	{
		m_rootDir = PathUtil::GetCurrentDir() + "/../../../../";
		m_projectDir = Engine::instance()->getResPath();
		m_outputDir = m_outputDir.empty() ? PathUtil::GetCurrentDir() + "/build/windows/" : m_outputDir;

		// create dir
		if (!PathUtil::IsDirExist(m_outputDir))
		{
			log("Create output directory : [%s]", m_outputDir.c_str());
			PathUtil::CreateDir(m_outputDir);
		}

		return true;
	}

	void WindowsBuildSettings::setIconRes(const ResourcePath& path)
	{

	}

	void WindowsBuildSettings::copySrc()
	{
		log("Copy Engine Source Code ...");

		// copy app
		PathUtil::CopyDir(m_rootDir + "app/windows/", m_outputDir + "app/windows/");

		// copy engine
		PathUtil::CopyDir(m_rootDir + "engine/", m_outputDir + "engine/");

		// copy thirdparty
		PathUtil::CopyDir(m_rootDir + "thirdparty/", m_outputDir + "thirdparty/");

		// copy CMakeLists.txt
		PathUtil::CopyFilePath(m_rootDir + "CMakeLists.txt", m_outputDir + "CMakeLists.txt");

		// copy build script
		PathUtil::CopyFilePath(m_rootDir + "build/windows/cmake.bat", m_outputDir + "cmake.bat");
	}

	void WindowsBuildSettings::copyRes()
	{
		log("Convert Project File ...");

		// copy release used dlls
		PathUtil::CopyDir(m_rootDir + "bin/app/Win64/Release/", m_outputDir + "bin/app/win64/Release/");
		PathUtil::CopyDir(m_rootDir + "bin/app/Win64/Debug/", m_outputDir + "bin/app/win64/Debug/");

		// copy res
		PathUtil::CopyDir(m_projectDir, m_outputDir + "bin/app/win64/Release/data/");

		// rename
		String projectFile = PathUtil::GetPureFilename(Engine::instance()->getConfig().m_projectFile);
		PathUtil::RenameFile(m_outputDir + "bin/app/win64/Release/data/" + projectFile, m_outputDir + "bin/app/win64/Release/data/app.echo");
	}

	void WindowsBuildSettings::cmake()
	{

	}

	void WindowsBuildSettings::compile()
	{

	}

	void WindowsBuildSettings::build()
	{
		log("Build App for Windows x64 platform.");

		m_listener->onBegin();

		if (prepare())
		{
			copySrc();
			copyRes();

			cmake();

			compile();
		}

		m_listener->onEnd();
	}
}
