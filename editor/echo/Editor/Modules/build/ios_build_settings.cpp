#include "ios_build_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>
#include <QProcess>

namespace Echo
{
    iOSBuildSettings::iOSBuildSettings()
    {

    }

    iOSBuildSettings::~iOSBuildSettings()
    {

    }

    iOSBuildSettings* iOSBuildSettings::instance()
    {
        static iOSBuildSettings* inst = EchoNew(iOSBuildSettings);
        return inst;
    }

    void iOSBuildSettings::bindMethods()
    {
        CLASS_BIND_METHOD(iOSBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(iOSBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
    }

    bool iOSBuildSettings::prepare()
    {
        m_rootDir   = PathUtil::GetCurrentDir() + "/../../../../";
        m_projectDir = Engine::instance()->getResPath();
        m_outputDir = PathUtil::GetCurrentDir() + "/build/ios/";

        // create dir
        if(!PathUtil::IsDirExist(m_outputDir))
        {
            log("Create output directory : [%s]", m_outputDir.c_str());
            PathUtil::CreateDir(m_outputDir);
        }

        return true;
    }

    void iOSBuildSettings::setIconRes(const ResourcePath& path)
    {

    }

    void iOSBuildSettings::copySrc()
    {
        log("Copy Engine Source Code ...");

        // copy app
        PathUtil::CopyDir( m_rootDir + "app/ios/", m_outputDir + "app/ios/");

        // copy engine
        PathUtil::CopyDir( m_rootDir + "engine/", m_outputDir + "engine/");

        // copy thirdparty
        PathUtil::CopyDir( m_rootDir + "thirdparty/", m_outputDir + "thirdparty/");

        // copy CMakeLists.txt
        PathUtil::CopyFilePath( m_rootDir + "CMakeLists.txt", m_outputDir + "CMakeLists.txt");
    }

    void iOSBuildSettings::copyRes()
    {
        log("Convert Project File ...");

        // copy res
        PathUtil::CopyDir( m_projectDir, m_outputDir + "app/ios/resources/data/");

        // rename
        String projectFile = PathUtil::GetPureFilename( Engine::instance()->getConfig().m_projectFile);
        PathUtil::RenameFile(m_outputDir + "app/ios/resources/data/" + projectFile, m_outputDir + "app/ios/resources/data/app.echo");
    }

    void iOSBuildSettings::cmake()
    {
        String solutionDir = m_outputDir + "solution/";
        String cmakeCmd = "cmake -DCMAKE_BUILD_TYPE=RELEASE -DECHO_EDITOR_MODE=FALSE -DECHO_BUILD_PLATFORM_IOS=TRUE -DECHO_BUILD_PLATFORM_MAC=FALSE -G\"Xcode\" ../";

        m_listener->onExecCmd(cmakeCmd.c_str(), solutionDir.c_str());
    }

    void iOSBuildSettings::compile()
    {

    }

    void iOSBuildSettings::build()
    {
        log("Build App for iOS platform.");

        m_listener->onBegin();

        if(prepare())
        {
            copySrc();
            copyRes();

            cmake();

            compile();
        }

        m_listener->onEnd();
    }

    String iOSBuildSettings::getFinalResultPath()
    {
        String FinalResultPath = m_outputDir + "bin/app/";
        return PathUtil::IsDirExist(FinalResultPath) ? FinalResultPath : m_outputDir;
    }
}
