#include "android_build_settings.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
    AndroidBuildSettings::AndroidBuildSettings()
    {
    
    }

    AndroidBuildSettings::~AndroidBuildSettings()
    {
        
    }

    AndroidBuildSettings* AndroidBuildSettings::instance()
    {
        static AndroidBuildSettings* inst = EchoNew(AndroidBuildSettings);
        return inst;
    }

    void AndroidBuildSettings::bindMethods()
    {
        CLASS_BIND_METHOD(AndroidBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(AndroidBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(AndroidBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
    }

    void AndroidBuildSettings::setIconRes(const ResourcePath& path)
    {
        
    }

    void AndroidBuildSettings::setOutputDir(const String& outputDir)
    {
		m_outputDir = outputDir;
		PathUtil::FormatPath(m_outputDir, false);
    }

    void AndroidBuildSettings::build()
    {
		log("Build App for Android platform.");

		m_listener->onBegin();

		if (prepare())
		{
			copySrc();
			copyRes();

			//replaceIcon();
			//replaceLaunchImage();

			// overwrite config
			//writeInfoPlist();
			//writeCMakeList();

			//writeModuleConfig();

			//cmake();

			//compile();
		}

		m_listener->onEnd();
    }

    bool AndroidBuildSettings::prepare()
    {
		m_rootDir = PathUtil::GetCurrentDir() + "/../../../../";
		m_projectDir = Engine::instance()->getResPath();
        m_outputDir = m_outputDir.empty() ? PathUtil::GetCurrentDir() + "/build/android/" : m_outputDir;

		// delete output dir
		if (PathUtil::IsDirExist(m_outputDir))
		{
			PathUtil::DelPath(m_outputDir);
		}

		// create dir
		if (!PathUtil::IsDirExist(m_outputDir))
		{
			log("Create output directory : [%s]", m_outputDir.c_str());
			PathUtil::CreateDir(m_outputDir);
		}

        return true;
    }

    void AndroidBuildSettings::copySrc()
    {
		log("Copy Engine Source Code ...");

		// copy app
		PathUtil::CopyDir(m_rootDir + "app/android/", m_outputDir + "app/android/");

		// copy engine
		PathUtil::CopyDir(m_rootDir + "engine/", m_outputDir + "engine/");

		// copy thirdparty
		PathUtil::CopyDir(m_rootDir + "thirdparty/", m_outputDir + "thirdparty/");

		// copy CMakeLists.txt
		PathUtil::CopyFilePath(m_rootDir + "CMakeLists.txt", m_outputDir + "CMakeLists.txt");

		// copy build script
		//PathUtil::CopyFilePath(m_rootDir + "build/ios/cmake.sh", m_outputDir + "cmake.sh");
    }

    void AndroidBuildSettings::copyRes()
    {
		log("Convert Project File ...");

		// copy res
		PathUtil::DelPath(m_outputDir + "app/android/app/src/main/assets/res/");
		PathUtil::CopyDir(m_projectDir, m_outputDir + "app/android/app/src/main/assets/res/");

		// rename
		String projectFile = PathUtil::GetPureFilename(Engine::instance()->getConfig().m_projectFile);
		PathUtil::RenameFile(m_outputDir + "app/android/app/src/main/assets/res/" + projectFile, m_outputDir + "app/android/app/src/main/assets/res/app.echo");
    }

	String AndroidBuildSettings::getFinalResultPath()
	{
		String FinalResultPath = m_outputDir + "bin/app/";
		return PathUtil::IsDirExist(FinalResultPath) ? FinalResultPath : m_outputDir;
	}
}
