#include "windows_build_settings.h"

namespace Echo
{
    WindowsBuildSettings::WindowsBuildSettings()
    {
    
    }

    WindowsBuildSettings::~WindowsBuildSettings()
    {
        
    }

    WindowsBuildSettings* WindowsBuildSettings::instance()
    {
        static WindowsBuildSettings* inst = EchoNew(WindowsBuildSettings);
        return inst;
    }

    void WindowsBuildSettings::bindMethods()
    {
        CLASS_BIND_METHOD(WindowsBuildSettings, getIconRes,   DEF_METHOD("getIconRes"));
        CLASS_BIND_METHOD(WindowsBuildSettings, setIconRes,   DEF_METHOD("setIconRes"));

        CLASS_REGISTER_PROPERTY(WindowsBuildSettings, "Icon", Variant::Type::ResourcePath, "getIconRes", "setIconRes");
    }

    void WindowsBuildSettings::setIconRes(const ResourcePath& path)
    {
        
    }
}
