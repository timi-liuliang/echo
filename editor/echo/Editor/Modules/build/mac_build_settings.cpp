#include "mac_build_settings.h"
#include "engine/core/util/PathUtil.h"
#include <engine/core/main/Engine.h>
#include <engine/core/main/module.h>
#include <engine/core/io/stream/FileHandleDataStream.h>
#include <engine/core/io/IO.h>
#include <thirdparty/pugixml/pugixml.hpp>

//extern "C"
//{
//    #include <thirdparty/libicns/icns.h>
//    int icns_png_to_image(icns_size_t dataSize, icns_byte_t* dataPtr, icns_image_t* imageOut);
//}

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
        CLASS_BIND_METHOD(MacBuildSettings, getAppName,         DEF_METHOD("getAppName"));
        CLASS_BIND_METHOD(MacBuildSettings, setAppName,         DEF_METHOD("setAppName"));
        CLASS_BIND_METHOD(MacBuildSettings, getIdentifier,      DEF_METHOD("getIdentifier"));
        CLASS_BIND_METHOD(MacBuildSettings, setIdentifier,      DEF_METHOD("setIdentifier"));
        CLASS_BIND_METHOD(MacBuildSettings, getVersion,         DEF_METHOD("getVersion"));
        CLASS_BIND_METHOD(MacBuildSettings, setVersion,         DEF_METHOD("setVersion"));
        CLASS_BIND_METHOD(MacBuildSettings, getIconRes,         DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(MacBuildSettings, setIconRes,         DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(MacBuildSettings, "AppName",    Variant::Type::String,          "getAppName",       "setAppName");
        CLASS_REGISTER_PROPERTY(MacBuildSettings, "Identifier", Variant::Type::String,          "getIdentifier",    "setIdentifier");
        CLASS_REGISTER_PROPERTY(MacBuildSettings, "Version",    Variant::Type::String,          "getVersion",       "setVersion");
        CLASS_REGISTER_PROPERTY(MacBuildSettings, "Icon",       Variant::Type::ResourcePath,    "getIconRes",       "setIconRes");
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

            replaceIcon();
//            replaceLaunchImage();
//
//            // overwrite config
            writeInfoPlist();
            writeCMakeList();

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

    String MacBuildSettings::getIdentifier() const
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

    String MacBuildSettings::getProjectName() const
    {
        return PathUtil::GetPureFilename( Engine::instance()->getConfig().m_projectFile, false);
    }

    String MacBuildSettings::getAppName() const
    {
        if(m_appName.empty())   return getProjectName();
        else                    return m_appName;
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

	void MacBuildSettings::replaceIcon()
	{
//		String iconFullPath = IO::instance()->convertResPathToFullPath(m_iconRes.getPath());
//		if (PathUtil::IsFileExist(iconFullPath))
//		{
//            MemoryReader memReader(m_iconRes.getPath());
//            if (memReader.getSize())
//            {
//                icns_image_t* icnsImage = EchoNew(icns_image_t);
//                if (ICNS_STATUS_OK == icns_png_to_image(memReader.getSize(), memReader.getData<icns_byte_t*>(), icnsImage))
//                {
//                    icnsImage->pngFilename = iconFullPath.c_str();
//
//					icns_family_t* iconFamily = nullptr;
//                    if (ICNS_STATUS_OK == icns_create_family(&iconFamily))
//                    {
//                        // iterate all icon types
//						icns_element_t* iconElement = nullptr;
//                        vector<icns_type_t>::type  allTypes =
//                        {
//                            ICNS_TABLE_OF_CONTENTS,         // 0x544F4320  // "TOC "
//
//                            ICNS_ICON_VERSION,              // 0x69636E56  // "icnV"
//
//                            ICNS_256x256_2X_32BIT_ARGB_DATA,// 0x69633134 // "ic14"
//                            ICNS_128x128_2X_32BIT_ARGB_DATA,// 0x69633133 // "ic13"
//                            ICNS_32x32_2X_32BIT_ARGB_DATA,  // 0x69633132 // "ic12"
//                            ICNS_16x16_2X_32BIT_ARGB_DATA,  // 0x69633131 // "ic11"
//                            ICNS_512x512_2X_32BIT_ARGB_DATA,// 0x69633130 // "ic10"
//
//                            ICNS_1024x1024_32BIT_ARGB_DATA, // 0x69633130 // "ic10"
//
//                            ICNS_512x512_32BIT_ARGB_DATA,   // 0x69633039  // "ic09"
//                            ICNS_256x256_32BIT_ARGB_DATA,   // 0x69633038  // "ic08"
//                            ICNS_128x128_32BIT_ARGB_DATA,   // 0x69633037  // "ic07"
//
//                            ICNS_128X128_32BIT_DATA,        // 0x69743332  // "it32"
//                            ICNS_128X128_8BIT_MASK,         // 0x74386D6B  // "t8mk"
//
//                            ICNS_48x48_1BIT_DATA,           // 0x69636823  // "ich#"
//                            ICNS_48x48_4BIT_DATA,           // 0x69636834  // "ich4"
//                            ICNS_48x48_8BIT_DATA,           // 0x69636838  // "ich8"
//                            ICNS_48x48_32BIT_DATA,          // 0x69683332  // "ih32"
//                            ICNS_48x48_1BIT_MASK,           // 0x69636823  // "ich#"
//                            ICNS_48x48_8BIT_MASK,           // 0x68386D6B  // "h8mk"
//
//                            ICNS_32x32_1BIT_DATA,           // 0x49434E23  // "ICN#"
//                            ICNS_32x32_4BIT_DATA,           // 0x69636C34  // "icl4"
//                            ICNS_32x32_8BIT_DATA,           // 0x69636C38  // "icl8"
//                            ICNS_32x32_32BIT_DATA,          // 0x696C3332  // "il32"
//                            ICNS_32x32_1BIT_MASK,           // 0x49434E23  // "ICN#"
//                            ICNS_32x32_8BIT_MASK,           // 0x6C386D6B  // "l8mk"
//
//                            ICNS_16x16_1BIT_DATA,           // 0x69637323  // "ics#"
//                            ICNS_16x16_4BIT_DATA,           // 0x69637334  // "ics4"
//                            ICNS_16x16_8BIT_DATA,           // 0x69637338  // "ics8"
//                            ICNS_16x16_32BIT_DATA,          // 0x69733332  // "is32"
//                            ICNS_16x16_1BIT_MASK,           // 0x69637323  // "ics#"
//                            ICNS_16x16_8BIT_MASK,           // 0x73386D6B  // "s8mk"
//
//                            ICNS_16x12_1BIT_DATA,           // 0x69636D23  // "icm#"
//                            ICNS_16x12_4BIT_DATA,           // 0x69636D34  // "icm4"
//                            ICNS_16x12_1BIT_MASK,           // 0x69636D23  // "icm#"
//                            ICNS_16x12_8BIT_DATA,           // 0x69636D38  // "icm8"
//
//                            ICNS_32x32_1BIT_ICON,           // 0x49434F4E  // "ICON"
//
//                            ICNS_TILE_VARIANT,              // 0x74696C65  // "tile"
//                            ICNS_ROLLOVER_VARIANT,          // 0x6F766572  // "over"
//                            ICNS_DROP_VARIANT,              // 0x64726F70  // "drop"
//                            ICNS_OPEN_VARIANT,              // 0x6F70656E  // "open"
//                            ICNS_OPEN_DROP_VARIANT,         // 0x6F647270  // "odrp"
//                        };
//
//                        // all all types element to family
//                        for (size_t i=0; i<allTypes.size(); i++)
//                        {
//							if (ICNS_STATUS_OK == icns_new_element_from_image(icnsImage, allTypes[i], &iconElement))
//								icns_add_element_in_family(&iconFamily, iconElement);
//                        }
//
//                        icns_sint32_t count;
//                        icns_count_elements_in_family(iconFamily, &count);
//                        if (count)
//                        {
//							// write icon file
//							String outputPath = m_outputDir + StringUtil::Format("app/mac/resources/mac/App.icns");
//							FILE* fileHandle = fopen(outputPath.c_str(), "wb");
//							if (fileHandle)
//							{
//								icns_write_family_to_file(fileHandle, iconFamily);
//
//								fflush(fileHandle);
//								fclose(fileHandle);
//							}
//                        }
//                    }
//                }
//            }
//		}
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

    void MacBuildSettings::writeInfoPlist()
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
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("App.icns");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleIdentifier");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(getIdentifier().c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleInfoDictionaryVersion");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("6.0");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleVersion");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(m_version.c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleShortVersionString");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(m_version.c_str());
            
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("NSMainStoryboardFile");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("Main");
                
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("NSPrincipalClass");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("NSApplication");
                
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("NSHighResolutionCapable");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("True");
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundleName");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value(getAppName().c_str());
        
        root_dict.append_child("key").append_child(pugi::node_pcdata).set_value("CFBundlePackageType");
        root_dict.append_child("string").append_child(pugi::node_pcdata).set_value("APPL");
        
        Echo::String savePath = m_outputDir + "app/mac/resources/mac/Info.plist";
        doc.save_file(savePath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }

    void MacBuildSettings::writeCMakeList()
    {
        String  cmakeStr;
        
        // module
        String moduleName = StringUtil::Replace(getProjectName(), " ", "");
        writeLine( cmakeStr, StringUtil::Format("SET(MODULE_NAME %s)", moduleName.c_str()));
        
        // set module path
        writeLine( cmakeStr, "SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})");
        
        // include directories
        writeLine( cmakeStr, "INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})");
        writeLine( cmakeStr, "INCLUDE_DIRECtORIES(${ECHO_ROOT_PATH})");
        
        // link directories
        writeLine( cmakeStr, "LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})");
        writeLine( cmakeStr, "LINK_DIRECTORIES(${ECHO_LIB_PATH})");
        writeLine( cmakeStr, "LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/lib/macos/)");
        
        // AddFrameWork Macro
        writeLine( cmakeStr, "MACRO(ADD_FRAMEWORK fwname)");
        writeLine( cmakeStr, "    SET(FRAMEWORKS \"${FRAMEWORKS} -framework ${fwname}\")");
        writeLine( cmakeStr, "ENDMACRO(ADD_FRAMEWORK)");
        
        // Get all project files recursively
        writeLine( cmakeStr, "FILE(GLOB_RECURSE HEADER_FILES *.h *.inl)");
        writeLine( cmakeStr, "FILE(GLOB_RECURSE SOURCE_FILES *.cpp *.m *.mm)");
        writeLine( cmakeStr, "FILE(GLOB_RECURSE STORYBOARD_FILES *.storyboard)");
        
        writeLine( cmakeStr, "SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES} ${STORYBOARD_FILES})");
        
        // group source files
        writeLine( cmakeStr, "GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})");
        
        // mac platform resources
        writeLine( cmakeStr, "SET(MAC_RESOURCE_FILES");
        writeLine( cmakeStr, "    ${MODULE_PATH}/resources/mac/App.icns");
        writeLine( cmakeStr, "    ${MODULE_PATH}/resources/data");
        writeLine( cmakeStr, ")");
        writeLine( cmakeStr, "SET_SOURCE_FILES_PROPERTIES(${MAC_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)");
        writeLine( cmakeStr, "SOURCE_GROUP(\"Resources\" FILES ${MAC_RESOURCE_FILES})");
        
        // add framework
        writeLine( cmakeStr, "ADD_FRAMEWORK(AppKit)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(QuartzCore)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(Metal)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(MetalKit)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(OpenAL)");
        writeLine( cmakeStr, "ADD_FRAMEWORK(OpenCL)");
        
        // settings
        writeLine( cmakeStr, "SET(FRAMEWORKS \"${FRAMEWORKS} -ObjC\")");
        writeLine( cmakeStr, "SET(CMAKE_EXE_LINKER_FLAGS ${FRAMEWORKS})");
        writeLine( cmakeStr, "SET(CMAKE_OSX_ARCHITECTURES \"${ARCHS_STANDARD}\")");
        
        writeLine( cmakeStr, "ADD_EXECUTABLE(${MODULE_NAME} MACOSX_BUNDLE ${ALL_FILES} ${MAC_RESOURCE_FILES} CMakeLists.txt)");
        
        // link libraries
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} engine)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} pugixml physx spine recast lua freeimage freetype zlib box2d)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} Live2DCubismCore)");
        writeLine( cmakeStr, "TARGET_LINK_LIBRARIES(${MODULE_NAME} glslang spirv-cross)");
        
        // set target properties
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD \"c++14\")");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY \"1,2\")");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_COPY_PHASE_STRIP No)");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS \"--deep\")");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/Info.plist)");
        writeLine( cmakeStr, "SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES RESOURCE ${STORYBOARD_FILES})");
        
        // messages
        writeLine( cmakeStr, "MESSAGE(STATUS \"Configure Mac App success!\")");
        
        // write to file
        String savePath = m_outputDir + "app/mac/CMakeLists.txt";
        FileHandleDataStream stream(savePath, DataStream::WRITE);
        if(!stream.fail())
        {
            stream.write(cmakeStr.data(), cmakeStr.size());
            stream.close();
        }
    }
}
