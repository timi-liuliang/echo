#pragma once

#include "build_settings.h"

namespace Echo
{
    class iOSBuildSettings : public BuildSettings
    {
        ECHO_SINGLETON_CLASS(iOSBuildSettings, BuildSettings)
        
    public:
        // Device type
        enum DeviceType
        {
            iPhone,
            iPad,
        };
        
        // icon
        struct AppIconItem
        {
            i32     m_size;
            
            AppIconItem(i32 size);
        };
        typedef vector<AppIconItem>::type AppIconArray;
        
        // launch image
        struct LaunchImageItem
        {
            i32         m_width;
            i32         m_height;
            i32         m_scale;
            DeviceType  m_type;
            
            LaunchImageItem(i32 width, i32 height, i32 scale, DeviceType type);
            
            // get path for cmake
            String getPortraitPath() const;
            String getLandscapePath() const;
            
            // get name
            String getPortraitName() const;
            String getLandscapeName() const;
        };
        typedef vector<LaunchImageItem>::type LaunchImageArray;
        
    public:
        iOSBuildSettings();
        virtual ~iOSBuildSettings();
        
        // instance
        static iOSBuildSettings* instance();

		// get name
		virtual const char* getPlatformName() const override { return "iOS"; }

		// platform thumbnail
		virtual ImagePtr getPlatformThumbnail() const override;
        
        // set output directory
        virtual void setOutputDir(const String& outputDir) override;
        
        // build
        virtual void build() override;
        
        // get final result path
        virtual String getFinalResultPath() override;
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
        // project name
        String getProjectName() const;
        
        // app name
        void setAppName(const String& appName) { m_appName = appName; }
        String getAppName() const;

        // identifier
        void setIdentifier(const String& identifier) { m_identifier = identifier; }
        String getIdentifier() const;
        
        // version
        void setVersion(const String& version) { m_version = version; }
        const String& getVersion() const { return m_version; }
        
        // status bar
        void setHiddenStatusBar(bool hiddenStatusBar) { m_hiddenStatusBar = hiddenStatusBar; }
        bool isHiddenStatusBar() const { return m_hiddenStatusBar; }
        
    public:
        // ui interface orientation portrait
        bool isUIInterfaceOrientationPortrait() const { return m_uiInterfaceOrientationPortrait; }
        void setUIInterfaceOrientationPortrait( bool value) { m_uiInterfaceOrientationPortrait = value;}
        
        // ui interface orientation upside down
        bool isUIInterfaceOrientationPortraitUpsideDown() const { return m_uiInterfaceOrientationPortraitUpsideDown; }
        void setUIInterfaceOrientationPortraitUpsideDown( bool value) { m_uiInterfaceOrientationPortraitUpsideDown = value;}
        
        // ui interface orientation landscape left
        bool isUIInterfaceOrientationLandscapeLeft() const { return m_uiInterfaceOrientationLandscapeLeft; }
        void setUIInterfaceOrientationLandscapeLeft( bool value) { m_uiInterfaceOrientationLandscapeLeft = value;}
        
        // ui interface orientation landscape right
        bool isUIInterfaceOrientationLandscapeRight() const { return m_uiInterfaceOrientationLandscapeRight; }
        void setUIInterfaceOrientationLandscapeRight( bool value) { m_uiInterfaceOrientationLandscapeRight = value;}
        
    private:
        // output directory
        bool prepare();
        
        // copy
        void copySrc();
        void copyRes();
        
        // replace
        void replaceIcon();
        void replaceLaunchImage();
        
        // cmake
        void cmake();
        
        // compile
        void compile();
        
    private:
        // conifg icon and launchimage
        void configAppIcons();
        void configLaunchImages();
       
    private:
        // write
        void writeInfoPlist();
        void writeUIInterfaceOrientationInfo(void* parent);
        void writeLaunchImageInfo(void* parent);

		// write config
		void writeModuleConfig();
        
    private:
        void writeCMakeList();
        
        // utils function
        bool rescaleIcon( const char* iFilePath, const char* oFilePath, ui32 targetWidth, ui32 targetHeight);
        
    private:
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        String                  m_solutionDir;
        ResourcePath            m_iconRes  = ResourcePath("Res://icon.png", ".png");
        String                  m_appName;
        String                  m_identifier;
        String                  m_version = "1.0.0";
        bool                    m_uiInterfaceOrientationPortrait = true;
        bool                    m_uiInterfaceOrientationPortraitUpsideDown = true;
        bool                    m_uiInterfaceOrientationLandscapeLeft = true;
        bool                    m_uiInterfaceOrientationLandscapeRight = true;
        AppIconArray            m_appIcons;
        LaunchImageArray        m_launchImages;
        bool                    m_hiddenStatusBar = true;
    };
}
