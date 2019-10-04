#include "ios_buildsettings.h"
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
        if(prepare())
        {
            copySrc();
            copyRes();
            
            cmake();
            
            compile();
        }
    }
}
