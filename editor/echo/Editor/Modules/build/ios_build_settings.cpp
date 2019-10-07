#include "ios_build_settings.h"
#include <engine/core/util/PathUtil.h>

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
    
    }

    void iOSBuildSettings::copyRes()
    {
        
    }

    void iOSBuildSettings::cmake()
    {
        
    }

    void iOSBuildSettings::compile()
    {
        
    }

    void iOSBuildSettings::build()
    {
        log("Build App for iOS platform.");
        
        if(prepare())
        {
            copySrc();
            copyRes();
            
            cmake();
            
            compile();
        }
    }
}
