#include "ios_buildsettings.h"

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

    void iOSBuildSettings::setIconRes(const ResourcePath& path)
    {
        
    }

    void iOSBuildSettings::build()
    {
        int  a = 10;
    }
}
