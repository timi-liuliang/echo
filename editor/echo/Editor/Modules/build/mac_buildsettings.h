#pragma once

#include "buildsettings.h"

namespace Echo
{
    class MacBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(MacBuildSettings, BuildSettings)
        
    public:
        MacBuildSettings();
        virtual ~MacBuildSettings();
        
        // instance
        static MacBuildSettings* instance();
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
    private:
        ResourcePath            m_iconRes;
    };
}
