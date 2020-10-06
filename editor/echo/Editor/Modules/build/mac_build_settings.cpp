#include "mac_build_settings.h"
#include "engine/core/util/PathUtil.h"
#include <engine/core/main/Engine.h>
#include <engine/core/main/module.h>
#include <engine/core/io/stream/FileHandleDataStream.h>

namespace Echo
{
    MacBuildSettings::MacBuildSettings()
    {
    
    }

    MacBuildSettings::~MacBuildSettings()
    {
        
    }

    MacBuildSettings* MacBuildSettings::instance()
    {
        static MacBuildSettings* inst = EchoNew(MacBuildSettings);
        return inst;
    }

    void MacBuildSettings::bindMethods()
    {
        CLASS_BIND_METHOD(MacBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(MacBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(MacBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
    }

    void MacBuildSettings::setOutputDir(const String& outputDir)
    {
        m_outputDir = outputDir;
        PathUtil::FormatPath(m_outputDir, false);
    }

    void MacBuildSettings::build()
    {
        log("Build App for Mac platform.");

        m_listener->onBegin();

        if(prepare())
        {
            copySrc();
            copyRes();
//
//            replaceIcon();
//            replaceLaunchImage();
//
//            // overwrite config
//            writeInfoPlist();
//            writeCMakeList();
//
            writeModuleConfig();

            //cmake();

            //compile();
        }

        m_listener->onEnd();
    }

	ImagePtr MacBuildSettings::getPlatformThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "editor/echo/Editor/Modules/build/editor/icon/mac.png");
	}

    void MacBuildSettings::setIconRes(const ResourcePath& path)
    {
        
    }

    String MacBuildSettings::getFinalResultPath()
    {
        String finalResultPath = m_outputDir + "bin/app/";
        return PathUtil::IsDirExist(finalResultPath) ? finalResultPath : m_outputDir;
    }

    bool MacBuildSettings::prepare()
    {
        m_rootDir   = Engine::instance()->getRootPath();
        m_projectDir = Engine::instance()->getResPath();
        m_outputDir = m_outputDir.empty() ? PathUtil::GetCurrentDir() + "/build/mac/" : m_outputDir;
        m_solutionDir = m_outputDir + "xcode/";
        
        // delete output dir
        if(PathUtil::IsDirExist(m_outputDir))
        {
            PathUtil::DelPath(m_outputDir);
        }

        // create dir
        if(!PathUtil::IsDirExist(m_outputDir))
        {
            log("Create output directory : [%s]", m_outputDir.c_str());
            PathUtil::CreateDir(m_outputDir);
        }
        
        return true;
    }

    void MacBuildSettings::copySrc()
    {
        log("Copy Engine Source Code ...");

        // copy app
        PathUtil::CopyDir( m_rootDir + "app/mac/", m_outputDir + "app/mac/");

        // copy engine
        PathUtil::CopyDir( m_rootDir + "engine/", m_outputDir + "engine/");

        // copy thirdparty
        PathUtil::CopyDir( m_rootDir + "thirdparty/", m_outputDir + "thirdparty/");

        // copy CMakeLists.txt
        PathUtil::CopyFilePath( m_rootDir + "CMakeLists.txt", m_outputDir + "CMakeLists.txt");
        
        // copy build script
        PathUtil::CopyFilePath( m_rootDir + "tool/build/mac/cmake.sh", m_outputDir + "cmake.sh");
    }

    void MacBuildSettings::copyRes()
    {
        log("Convert Project File ...");

        // copy res
        PathUtil::CopyDir( m_projectDir, m_outputDir + "app/mac/resources/data/");
        packageRes(m_outputDir + "app/mac/resources/data/");

        // rename
        String projectFile = PathUtil::GetPureFilename( Engine::instance()->getConfig().m_projectFile);
        PathUtil::RenameFile(m_outputDir + "app/mac/resources/data/" + projectFile, m_outputDir + "app/mac/resources/data/app.echo");
    }

    void MacBuildSettings::writeModuleConfig()
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
        String savePath = m_outputDir + "app/mac/config/ModuleConfig.cpp";
        FileHandleDataStream stream(savePath, DataStream::WRITE);
        if (!stream.fail())
        {
            stream.write(moduleSrc.data(), moduleSrc.size());
            stream.close();
        }
    }
}
