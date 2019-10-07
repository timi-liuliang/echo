#pragma once

#include "build_settings.h"

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
        // output directory
        bool prepare();
        
        // copy
        void copySrc();
        void copyRes();
        
        // cmake
        void cmake();
        
        // compile
        void compile();
        
    private:
        String                  m_outputDir;
        ResourcePath            m_iconRes;
    };
}
