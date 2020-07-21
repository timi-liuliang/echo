#pragma once

#include "build_settings.h"

namespace Echo
{
    class AndroidBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(AndroidBuildSettings, BuildSettings)

    public:
		struct AppIconItem
		{
			i32     m_size;
			String  m_folder;
		};
        
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

		void setIconRoundRes(const ResourcePath& path);
		const ResourcePath& getIconRoundRes() { return m_iconRoundRes; }

    private:
        // output directory
        bool prepare();

        // copy
        void copySrc();
        void copyRes();

		// replace
		void replaceIcon();

    private:
        // write settings
        void writeStringsXml();

        // write config
        void writeModuleConfig();

    private:
		// utils function
		bool rescaleIcon(const char* iFilePath, const char* oFilePath, ui32 targetWidth, ui32 targetHeight);
        
    private:
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        String                  m_appName;
        ResourcePath            m_iconRes = ResourcePath("Res://icon.png", ".png");
        ResourcePath            m_iconRoundRes = ResourcePath("Res://icon.png", ".png");
    };
}
