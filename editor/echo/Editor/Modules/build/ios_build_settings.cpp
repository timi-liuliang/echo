#include "ios_build_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/main/Engine.h>
#include <thirdparty/pugixml/pugixml.hpp>
#include <QProcess>

namespace Echo
{
    iOSBuildSettings::LaunchImageItem::LaunchImageItem(i32 width, i32 height, i32 scale, iOSBuildSettings::DeviceType type)
        : m_width(width)
        , m_height(height)
        , m_scale(scale)
        , m_type(type)
    {}

    String iOSBuildSettings::LaunchImageItem::getPortraitPath() const
    {
        return "    ${MODULE_PATH}/frame/Platform/iOS/Launch/" + getPortraitName();
    }

    String iOSBuildSettings::LaunchImageItem::getLandscapePath() const
    {
        return "    ${MODULE_PATH}/frame/Platform/iOS/Launch/" + getLandscapeName();
    }

    String iOSBuildSettings::LaunchImageItem::getPortraitName() const
    {
        i32 width = std::min<i32>(m_width, m_height);
        i32 height = std::max<i32>(m_width, m_height);
        return StringUtil::Format("Default-Portrait-%dx%d@%dx.png", width, height, m_scale);
    }

    String iOSBuildSettings::LaunchImageItem::getLandscapeName() const
    {
        i32 width = std::max<i32>(m_width, m_height);
        i32 height = std::min<i32>(m_width, m_height);
        return StringUtil::Format("Default-Landscape-%dx%d@%dx.png", width, height, m_scale);
    }

    iOSBuildSettings::iOSBuildSettings()
    {
        // https://developer.apple.com/design/human-interface-guidelines/ios/icons-and-images/launch-screen/
        
        // iphone se
        m_launchImages.push_back(LaunchImageItem(320, 568, 2, DeviceType::iPhone));
        
        // iphone6s,iphone7,iphone8
        m_launchImages.push_back(LaunchImageItem(375, 667, 2, DeviceType::iPhone));
        
        // iphone6s plus, iphone7 plus, iphone8 plus
        m_launchImages.push_back(LaunchImageItem(414, 736, 3, DeviceType::iPhone));
        
        // iphoneX, iphoneXs 1125px × 2436px
        m_launchImages.push_back(LaunchImageItem(375, 812, 3, DeviceType::iPhone));
        
        // iphone XR
        m_launchImages.push_back(LaunchImageItem(414, 896, 2, DeviceType::iPhone));
        
        // iphone Xs Max
        m_launchImages.push_back(LaunchImageItem(414, 896, 3, DeviceType::iPhone));
        
        // ipad mini, ipad
        m_launchImages.push_back(LaunchImageItem(1024, 768, 2, DeviceType::iPad));
        
        // 10.5 iPad pro
        m_launchImages.push_back(LaunchImageItem(1112, 834, 2, DeviceType::iPad));
        
        // 11 iPad pro
        m_launchImages.push_back(LaunchImageItem(1194, 834, 2, DeviceType::iPad));
    
        // 12.9 iPad pro
        m_launchImages.push_back(LaunchImageItem(1366, 1024, 2, DeviceType::iPad));
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
        CLASS_BIND_METHOD(iOSBuildSettings, getAppName,         DEF_METHOD("getAppName"));
        CLASS_BIND_METHOD(iOSBuildSettings, setAppName,         DEF_METHOD("setAppName"));
        CLASS_BIND_METHOD(iOSBuildSettings, getIdentifier,      DEF_METHOD("getIdentifier"));
        CLASS_BIND_METHOD(iOSBuildSettings, setIdentifier,      DEF_METHOD("setIdentifier"));
        CLASS_BIND_METHOD(iOSBuildSettings, getVersion,         DEF_METHOD("getVersion"));
        CLASS_BIND_METHOD(iOSBuildSettings, setVersion,         DEF_METHOD("setVersion"));
        CLASS_BIND_METHOD(iOSBuildSettings, getIconRes,         DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(iOSBuildSettings, setIconRes,         DEF_METHOD("setIconRes"));
        CLASS_BIND_METHOD(iOSBuildSettings, isHiddenStatusBar,  DEF_METHOD("isHiddenStatusBar"));
        CLASS_BIND_METHOD(iOSBuildSettings, setHiddenStatusBar, DEF_METHOD("setHiddenStatusBar"));

        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "AppName",    Variant::Type::String,          "getAppName",       "setAppName");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Identifier", Variant::Type::String,          "getIdentifier",    "setIdentifier");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Version",    Variant::Type::String,          "getVersion",       "setVersion");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Icon",       Variant::Type::ResourcePath,    "getIconRes",       "setIconRes");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "HiddenStatusBar", Variant::Type::Bool,       "isHiddenStatusBar","setHiddenStatusBar");
        
        // Ui interface orientation
        CLASS_BIND_METHOD(iOSBuildSettings, isUIInterfaceOrientationPortrait,           DEF_METHOD("isUIInterfaceOrientationPortrait"));
        CLASS_BIND_METHOD(iOSBuildSettings, setUIInterfaceOrientationPortrait,          DEF_METHOD("setUIInterfaceOrientationPortrait"));
        CLASS_BIND_METHOD(iOSBuildSettings, isUIInterfaceOrientationPortraitUpsideDown, DEF_METHOD("isUIInterfaceOrientationPortraitUpsideDown"));
        CLASS_BIND_METHOD(iOSBuildSettings, setUIInterfaceOrientationPortraitUpsideDown,DEF_METHOD("setUIInterfaceOrientationPortraitUpsideDown"));
        CLASS_BIND_METHOD(iOSBuildSettings, isUIInterfaceOrientationLandscapeLeft,      DEF_METHOD("isUIInterfaceOrientationLandscapeLeft"));
        CLASS_BIND_METHOD(iOSBuildSettings, setUIInterfaceOrientationLandscapeLeft,     DEF_METHOD("setUIInterfaceOrientationLandscapeLeft"));
        CLASS_BIND_METHOD(iOSBuildSettings, isUIInterfaceOrientationLandscapeRight,     DEF_METHOD("isUIInterfaceOrientationLandscapeRight"));
        CLASS_BIND_METHOD(iOSBuildSettings, setUIInterfaceOrientationLandscapeRight,    DEF_METHOD("setUIInterfaceOrientationLandscapeRight"));
        
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "Portrait",           Variant::Type::Bool, "isUIInterfaceOrientationPortrait",            "setUIInterfaceOrientationPortrait");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "PortraitUpsideDown", Variant::Type::Bool, "isUIInterfaceOrientationPortraitUpsideDown",  "setUIInterfaceOrientationPortraitUpsideDown");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "LandscapeLeft",      Variant::Type::Bool, "isUIInterfaceOrientationLandscapeLeft",       "setUIInterfaceOrientationLandscapeLeft");
        CLASS_REGISTER_PROPERTY(iOSBuildSettings, "LandscapeRight",     Variant::Type::Bool, "isUIInterfaceOrientationLandscapeRight",      "setUIInterfaceOrientationLandscapeRight");
        
        CLASS_REGISTER_PROPERTY_HINT(iOSBuildSettings, "Portrait",           PropertyHintType::Category, "UIInterfaceOrientation");
        CLASS_REGISTER_PROPERTY_HINT(iOSBuildSettings, "PortraitUpsideDown", PropertyHintType::Category, "UIInterfaceOrientation");
        CLASS_REGISTER_PROPERTY_HINT(iOSBuildSettings, "LandscapeLeft",      PropertyHintType::Category, "UIInterfaceOrientation");
        CLASS_REGISTER_PROPERTY_HINT(iOSBuildSettings, "LandscapeRight",     PropertyHintType::Category, "UIInterfaceOrientation");
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

    void iOSBuildSettings::writeUIInterfaceOrientationInfo(void* parent)
    {
        pugi::xml_node* root_dict = (pugi::xml_node*)(parent);
        
        // https://developer.apple.com/documentation/uikit/uiinterfaceorientation?language=objc
        root_dict->append_child("key").append_child(pugi::node_pcdata).set_value("UISupportedInterfaceOrientations");
        pugi::xml_node orient_node = root_dict->append_child("array");
        
        if(m_uiInterfaceOrientationPortrait)
            orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationPortrait");
        
        if(m_uiInterfaceOrientationPortraitUpsideDown)
            orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationPortraitUpsideDown");
        
        if(m_uiInterfaceOrientationLandscapeLeft)
            orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationLandscapeLeft");
        
        if(m_uiInterfaceOrientationLandscapeRight)
            orient_node.append_child("string").append_child(pugi::node_pcdata).set_value("UIInterfaceOrientationLandscapeRight");
    }

    void iOSBuildSettings::writeLaunchImageInfo(void* parent)
    {
        pugi::xml_node* root_dict = (pugi::xml_node*)(parent);
        
        // iphone
        root_dict->append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImages~iphone");
        pugi::xml_node iphone_node = root_dict->append_child("array");
        
        // ipad
        root_dict->append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImages~ipad");
        pugi::xml_node ipad_node = root_dict->append_child("array");
        
        for(const LaunchImageItem& image : m_launchImages)
        {
            i32 minDimension = std::min<i32>(image.m_width, image.m_height);
            i32 maxDimension = std::max<i32>(image.m_width, image.m_height);
            pugi::xml_node dict_parent = image.m_type == DeviceType::iPhone ? iphone_node : ipad_node;
            
            if(m_uiInterfaceOrientationPortrait || m_uiInterfaceOrientationPortraitUpsideDown)
            {
                pugi::xml_node dict_node = dict_parent.append_child("dict");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageMinimumOSVersion");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value("8.0");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageName");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value(image.getPortraitName().c_str());
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageOrientation");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value("Portrait");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageSize");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value(StringUtil::Format("{%d, %d}", minDimension, maxDimension).c_str());
            }
            
            if(m_uiInterfaceOrientationLandscapeLeft || m_uiInterfaceOrientationLandscapeRight)
            {
                pugi::xml_node dict_node = dict_parent.append_child("dict");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageMinimumOSVersion");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value("8.0");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageName");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value(image.getLandscapeName().c_str());
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageOrientation");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value("Landscape");
                
                dict_node.append_child("key").append_child(pugi::node_pcdata).set_value("UILaunchImageSize");
                dict_node.append_child("string").append_child(pugi::node_pcdata).set_value(StringUtil::Format("{%d, %d}", maxDimension, minDimension).c_str());
            }
        }
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
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleVersion");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(m_version.c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleShortVersionString");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(m_version.c_str());
        
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
        
        if(m_hiddenStatusBar)
        {
            root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("UIStatusBarHidden");
            root_dict.append_child("true");
            
            root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("UIViewControllerBasedStatusBarAppearance");
            root_dict.append_child("false");
        }
        
        writeUIInterfaceOrientationInfo(&root_dict);
        writeLaunchImageInfo(&root_dict);

        Echo::String savePath = m_outputDir + "app/ios/frame/Platform/iOS/Info.plist";
        doc.save_file(savePath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }

    static void writeLine(String& str, const String& line)
    {
        str += line + "\n";
    }

    void iOSBuildSettings::writeCMakeList()
    {
        String  cmakeStr;
        
        // module
        String moduleName = StringUtil::Replace(getAppName(), " ", "");
        writeLine( cmakeStr, StringUtil::Format("SET(MODULE_NAME %s)", moduleName.c_str()));
        
        // set module path
        writeLine( cmakeStr, "SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})");
        
        // include directories
        writeLine( cmakeStr, "INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})");
        writeLine( cmakeStr, "INCLUDE_DIRECtORIES(${ECHO_ROOT_PATH})");
        
        // link directories
        writeLine( cmakeStr, "LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})");
        writeLine( cmakeStr, "LINK_DIRECTORIES(${ECHO_LIB_PATH})");
        writeLine( cmakeStr, "LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/lib/ios/)");
        
        // AddFrameWork Macro
        writeLine( cmakeStr, "MACRO(ADD_FRAMEWORK fwname)");
        writeLine( cmakeStr, "    SET(FRAMEWORKS \"${FRAMEWORKS} -framework ${fwname}\")");
        writeLine( cmakeStr, "ENDMACRO(ADD_FRAMEWORK)");
        
        // Get all project files recursively
        writeLine( cmakeStr, "FILE(GLOB_RECURSE HEADER_FILES *.h *.inl)");
        writeLine( cmakeStr, "FILE(GLOB_RECURSE SOURCE_FILES *.cpp *.m *.mm)");
        
        writeLine( cmakeStr, "SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})");
        
        // group source files
        writeLine( cmakeStr, "GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})");
        
        // iOS platform resources
        writeLine( cmakeStr, "SET(IOS_RESOURCE_FILES");
        
        // icon
        writeLine( cmakeStr, "    ${MODULE_PATH}/frame/Platform/iOS/Icon/Icon.png");
        
        // launch images
        for(const LaunchImageItem& image : m_launchImages)
        {
            if(m_uiInterfaceOrientationPortrait || m_uiInterfaceOrientationPortraitUpsideDown)
                writeLine( cmakeStr, image.getPortraitPath());
            
            if(m_uiInterfaceOrientationLandscapeLeft || m_uiInterfaceOrientationLandscapeRight)
                writeLine(cmakeStr, image.getLandscapePath());
        }
        
        // data
        writeLine( cmakeStr, "    ${MODULE_PATH}/resources/data");
        writeLine( cmakeStr, ")");
        writeLine( cmakeStr, "SET_SOURCE_FILES_PROPERTIES(${IOS_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)");
        
        // add framework
        writeLine( cmakeStr, "ADD_FRAMEWORK(MediaPlayer)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(AudioToolbox)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreGraphics)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(QuartzCore)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(UIKit)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(GLKit)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(OpenGLES)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(OpenAL)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CFNetwork)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(MobileCoreServices)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(SystemConfiguration)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(AVFoundation)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(AdSupport)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreFoundation)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreTelephony)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreText)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(iAd)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(StoreKit)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreData)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreMedia)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(CoreMotion)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(Security)");
        
        // settings
        writeLine( cmakeStr, "SET(FRAMEWORKS \"${FRAMEWORKS} -ObjC\")");
        writeLine( cmakeStr, "SET(CMAKE_EXE_LINKER_FLAGS ${FRAMEWORKS})");
        writeLine( cmakeStr, "SET(CMAKE_OSX_ARCHITECTURES \"${ARCHS_STANDARD}\")");
        
        writeLine( cmakeStr, "ADD_EXECUTABLE(${MODULE_NAME} MACOSX_BUNDLE ${HEADER_FILES} ${SOURCE_FILES} ${IOS_RESOURCE_FILES} CMakeLists.txt)");
        
        // link libraries
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} engine)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} pugixml physx spine recast lua freeimage freetype zlib box2d)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} Live2DCubismCore)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} glslang spirv-cross)");
        
        // set target properties
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${MODULE_PATH}/Frame/Platform/iOS/Info.plist)");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD \"c++14\")");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY \"1,2\")");
        
        // messages
        writeLine( cmakeStr, "MESSAGE(STATUS \"Configure iOS App success!\")");
        
        // write to file
        String savePath = m_outputDir + "app/ios/CMakeLists.txt";
        FileHandleDataStream stream(savePath, DataStream::WRITE);
        if(!stream.fail())
        {
            stream.write(cmakeStr.data(), cmakeStr.size());
            stream.close();
        }
    }
}
