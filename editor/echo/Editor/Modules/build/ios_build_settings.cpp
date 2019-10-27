#include "ios_build_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>
#include <thirdparty/pugixml/pugixml.hpp>
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
        CLASS_BIND_METHOD(iOSBuildSettings, getAppName,   DEF_METHOD("getAppName"));
        CLASS_BIND_METHOD(iOSBuildSettings, setAppName,   DEF_METHOD("setAppName"));
        CLASS_BIND_METHOD(iOSBuildSettings, getIdentifier,DEF_METHOD("getIdentifier"));
        CLASS_BIND_METHOD(iOSBuildSettings, setIdentifier,DEF_METHOD("setIdentifier"));
        CLASS_BIND_METHOD(iOSBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(iOSBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "AppName", Variant::Type::String, "getAppName", "setAppName");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Identifier", Variant::Type::String, "getIdentifier", "setIdentifier");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
    }
    
    void iOSBuildSettings::setOutputDir(const String& outputDir)
    {
        m_outputDir = outputDir;
        PathUtil::FormatPath(m_outputDir, false);
    }

    bool iOSBuildSettings::prepare()
    {
        m_rootDir   = PathUtil::GetCurrentDir() + "/../../../../";
        m_projectDir = Engine::instance()->getResPath();
        m_outputDir = m_outputDir.empty() ? PathUtil::GetCurrentDir() + "/build/ios/" : m_outputDir;
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
        
        // copy build script
        PathUtil::CopyFilePath( m_rootDir + "build/ios/cmake.sh", m_outputDir + "cmake.sh");
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
        String workingDir = m_outputDir + "build/ios/";
        String cmakeCmd = "sh cmake.sh";
        if(!m_listener->onExecCmd(cmakeCmd.c_str(), workingDir.c_str()))
        {
            log("exec command [%s] failed.", cmakeCmd.c_str());
        }
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
            
            // overwrite config
            writeInfoPlist();
            writeCMakeList();

            //cmake();

            //compile();
        }

        m_listener->onEnd();
    }

    String iOSBuildSettings::getAppName() const
    {
        if(m_appName.empty())   return PathUtil::GetPureFilename( Engine::instance()->getConfig().m_projectFile, false);
        else                    return m_appName;
    }

    String iOSBuildSettings::getIdentifier() const
    {
        if(m_identifier.empty())
        {
            String appName = PathUtil::GetPureFilename( Engine::instance()->getConfig().m_projectFile, false);
            String identifier = "com.echo." + appName;
            identifier = StringUtil::Replace( identifier, ' ', '_');
            
            return identifier;
        }
        else
        {
            return m_identifier;
        }
    }

    String iOSBuildSettings::getFinalResultPath()
    {
        String FinalResultPath = m_outputDir + "bin/app/";
        return PathUtil::IsDirExist(FinalResultPath) ? FinalResultPath : m_outputDir;
    }

    void iOSBuildSettings::writeInfoPlist()
    {
        pugi::xml_document doc;
        pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
        dec.append_attribute("version") = "1.0";
        dec.append_attribute("encoding") = "utf-8";

        pugi::xml_node root_node= doc.append_child("plist" );
        root_node.append_attribute("version").set_value("1.0");
        
        pugi::xml_node root_dict = root_node.append_child("dict");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleDevelopmentRegion");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("English");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleExecutable");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("$(EXECUTABLE_NAME)");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleGetInfoString");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleIconFile");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("Icon.png");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleIdentifier");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(getIdentifier().c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleInfoDictionaryVersion");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("6.0");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleName");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(getAppName().c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundlePackageType");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("APPL");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("UISupportedInterfaceOrientations");
        pugi::xml_node orient_node = root_dict.append_child("array");
        orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationLandscapeRight");
        orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationLandscapeLeft");

        Echo::String savePath = m_outputDir + "app/ios/frame/Platform/iOS/Info.plist";
        doc.save_file(savePath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }

    void iOSBuildSettings::writeCMakeList()
    {
    
    }
}
