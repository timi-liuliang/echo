#pragma once

#include "buildsettings.h"

namespace Echo
{
    class iOSBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(iOSBuildSettings, BuildSettings)
        
    public:
        iOSBuildSettings();
        virtual ~iOSBuildSettings();
        
        // instance
        static iOSBuildSettings* instance();
        
        // build
        virtual void build() override;
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
    private:
        ResourcePath            m_iconRes;
    };
}
