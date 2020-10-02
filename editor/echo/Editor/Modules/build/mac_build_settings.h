#pragma once

#include "build_settings.h"

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
        
        // build
        virtual void build() override;

		// get name
		virtual const char* getPlatformName() const override { return "Mac"; }

		// platform thumbnail
		virtual ImagePtr getPlatformThumbnail() const override;
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
    private:
        // output directory
        bool prepare();
        
    private:
        ResourcePath            m_iconRes;
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        String                  m_solutionDir;
    };
}
