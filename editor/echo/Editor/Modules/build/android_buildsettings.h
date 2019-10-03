#pragma once

#include "buildsettings.h"

namespace Echo
{
    class AndroidBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(AndroidBuildSettings, BuildSettings)
        
    public:
        AndroidBuildSettings();
        virtual ~AndroidBuildSettings();
        
        // instance
        static AndroidBuildSettings* instance();
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
    private:
        ResourcePath            m_iconRes;
    };
}
