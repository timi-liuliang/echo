#pragma once

#include "build_settings.h"

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

		// get name
		virtual char* getPlatformName() const { return "Android"; }

		// platform thumbnail
        virtual ImagePtr getPlatformThumbnail() const override;

        // set output directory
        virtual void setOutputDir(const String& outputDir) override;

        // build
        virtual void build() override;

		// app name
		void setAppName(const String& appName) { m_appName = appName; }
		String getAppName() const;

		// get final result path
		virtual String getFinalResultPath() override;
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }

    private:
        // output directory
        bool prepare();

        // copy
        void copySrc();
        void copyRes();

    private:
        // write settings
        void writeStringsXml();

        // write config
        void writeModuleConfig();
        
    private:
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        ResourcePath            m_iconRes;
        String                  m_appName;
    };
}
