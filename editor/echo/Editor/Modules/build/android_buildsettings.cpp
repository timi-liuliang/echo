#include "android_buildsettings.h"

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
}
