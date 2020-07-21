#include "android_build_settings.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"
#include <engine/core/io/IO.h>
#include <thirdparty/pugixml/pugixml.hpp>
#include "FreeImageHelper.h"
#include <engine/core/main/module.h>

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
		CLASS_BIND_METHOD(AndroidBuildSettings, getAppName, DEF_METHOD("getAppName"));
		CLASS_BIND_METHOD(AndroidBuildSettings, setAppName, DEF_METHOD("setAppName"));
        CLASS_BIND_METHOD(AndroidBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(AndroidBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));
		CLASS_BIND_METHOD(AndroidBuildSettings, getIconRoundRes, DEF_METHOD("getIconRoundRes"));
		CLASS_BIND_METHOD(AndroidBuildSettings, setIconRoundRes, DEF_METHOD("setIconRoundRes"));

		CLASS_REGISTER_PROPERTY(AndroidBuildSettings, "AppName", Variant::Type::String, "getAppName", "setAppName");
        CLASS_REGISTER_PROPERTY(AndroidBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
		CLASS_REGISTER_PROPERTY(AndroidBuildSettings, "IconRound", Variant::Type::ResourcePath, "getIconRoundRes", "setIconRoundRes");
    }

	ImagePtr AndroidBuildSettings::getPlatformThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "editor/echo/Editor/Modules/build/editor/icon/android.png");
	}

    void AndroidBuildSettings::setIconRes(const ResourcePath& path)
    {
        
    }

	void AndroidBuildSettings::setIconRoundRes(const ResourcePath& path)
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

			replaceIcon();
			//replaceLaunchImage();

			// overwrite config
			writeStringsXml();
			//writeCMakeList();

			writeModuleConfig();

			//cmake();

			//compile();
		}

		m_listener->onEnd();
    }

	String AndroidBuildSettings::getAppName() const
	{
		if (m_appName.empty())  return PathUtil::GetPureFilename(Engine::instance()->getConfig().m_projectFile, false);
		else                    return m_appName;
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

	void AndroidBuildSettings::replaceIcon()
	{
		// icon
		AppIconItem iconConfigs[] =
		{
			{72, "mipmap-hdpi"},
			{48, "mipmap-mdpi"},
			{96, "mipmap-xhdpi"},
			{144, "mipmap-xxhdpi"},
			{192, "mipmap-xxxhdpi"}
		};

		String iconFullPath = IO::instance()->convertResPathToFullPath(m_iconRes.getPath());
		if (PathUtil::IsFileExist(iconFullPath))
		{
			for (const AppIconItem& item : iconConfigs)
			{
				String outputPath = m_outputDir + StringUtil::Format("app/android/app/src/main/res/%s/ic_launcher.png", item.m_folder.c_str());
				PathUtil::DelPath(outputPath);

				rescaleIcon(iconFullPath.c_str(), outputPath.c_str(), item.m_size, item.m_size);
			}
		}

		String iconRoundFullPath = IO::instance()->convertResPathToFullPath(m_iconRoundRes.getPath());
		if (PathUtil::IsFileExist(iconRoundFullPath))
		{
			for (const AppIconItem& item : iconConfigs)
			{
				String outputPath = m_outputDir + StringUtil::Format("app/android/app/src/main/res/%s/ic_launcher_round.png", item.m_folder.c_str());
				PathUtil::DelPath(outputPath);

				rescaleIcon(iconRoundFullPath.c_str(), outputPath.c_str(), item.m_size, item.m_size);
			}
		}
	}

	void AndroidBuildSettings::writeStringsXml()
	{
		pugi::xml_document doc;
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		pugi::xml_node root_node = doc.append_child("resources");

		// app name
		pugi::xml_node app_name_node = root_node.append_child("string");
		app_name_node.append_attribute("name").set_value("app_name");
		app_name_node.append_child(pugi::node_pcdata).set_value(getAppName().c_str());

		// write file
		Echo::String savePath = m_outputDir + "app/android/app/src/main/res/values/strings.xml";
		doc.save_file(savePath.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	void AndroidBuildSettings::writeModuleConfig()
	{
		String  moduleSrc;

		// include
		writeLine(moduleSrc, "#include <engine/core/main/module.h>\n");

		// namespace
		writeLine(moduleSrc, "namespace Echo\n{");
		writeLine(moduleSrc, "\tvoid registerModules()");
		writeLine(moduleSrc, "\t{");
		vector<Module*>::type* allModules = Module::getAllModules();
		if (allModules)
		{
			for (Module* module : *allModules)
			{
				if (module->isEnable() && !module->isEditorOnly())
					writeLine(moduleSrc, StringUtil::Format("\t\tREGISTER_MODULE(%s)", module->getClassName().c_str()));
			}
		}

		// end namespace
		writeLine(moduleSrc, "\t}\n}\n");

		// Write to file
		String savePath = m_outputDir + "app/android/app/src/main/cpp/echo/ModuleConfig.cpp";
		FileHandleDataStream stream(savePath, DataStream::WRITE);
		if (!stream.fail())
		{
			stream.write(moduleSrc.data(), moduleSrc.size());
			stream.close();
		}
	}

	bool AndroidBuildSettings::rescaleIcon(const char* iFilePath, const char* oFilePath, ui32 targetWidth, ui32 targetHeight)
	{
		std::string ext = PathUtil::GetFileExt(iFilePath).c_str();
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename(iFilePath);
		int fiFlags = FreeImageHelper::instance()->MappingFlagsByFormat(fileFMT);

		// Load
		if (fileFMT != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fileFMT))
		{
			FIBITMAP* dip = FreeImage_Load(fileFMT, iFilePath, fiFlags);
			if (dip)
			{
				FIBITMAP* dipScaled = FreeImage_Rescale(dip, targetWidth, targetHeight, FILTER_BSPLINE);

				dipScaled = FreeImage_ConvertTo32Bits(dipScaled);

				FreeImage_Save(fileFMT, dipScaled, oFilePath, TARGA_DEFAULT);

				FreeImage_Unload(dipScaled);
				FreeImage_Unload(dip);

				return true;
			}
		}

		return false;
	}
}
