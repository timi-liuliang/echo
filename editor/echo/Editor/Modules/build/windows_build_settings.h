#pragma once

#include "build_settings.h"

namespace Echo
{
    class WindowsBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(WindowsBuildSettings, BuildSettings)
        
    public:
        WindowsBuildSettings();
        virtual ~WindowsBuildSettings();
        
        // instance
        static WindowsBuildSettings* instance();
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
    private:
        ResourcePath            m_iconRes;
    };
}
