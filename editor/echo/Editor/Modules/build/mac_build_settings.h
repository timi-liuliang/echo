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
        
        // set output directory
        virtual void setOutputDir(const String& outputDir) override;
        
        // get final result path
        virtual String getFinalResultPath() override;
        
        // identifier
        void setIdentifier(const String& identifier) { m_identifier = identifier; }
        String getIdentifier() const;
        
        // version
        void setVersion(const String& version) { m_version = version; }
        const String& getVersion() const { return m_version; }
        
        // app name
        void setAppName(const String& appName) { m_appName = appName; }
        String getAppName() const;
        
    private:
        // output directory
        bool prepare();
        
        // copy
        void copySrc();
        void copyRes();

		// replace
		void replaceIcon();
        
        // write
        void writeModuleConfig();
        void writeInfoPlist();
        void writeCMakeList();
        
    private:
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        String                  m_solutionDir;
        String                  m_appName;
        ResourcePath            m_iconRes = ResourcePath("Res://icon.png", ".png");
        String                  m_identifier;
        String                  m_version = "1.0.0";
    };
}
