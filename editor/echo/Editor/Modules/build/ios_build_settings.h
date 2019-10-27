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
        
        // set output directory
        virtual void setOutputDir(const String& outputDir) override;
        
        // build
        virtual void build() override;
        
        // get final result path
        virtual String getFinalResultPath() override;
        
        // icon res path
        void setIconRes(const ResourcePath& path);
        const ResourcePath& getIconRes() { return m_iconRes; }
        
        // app name
        void setAppName(const String& appName) { m_appName = appName; }
        String getAppName() const;

        // identifier
        void setIdentifier(const String& identifier) { m_identifier = identifier; }
        String getIdentifier() const;
        
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
        
        // cmake
        void cmake();
        
        // compile
        void compile();
       
    private:
        // write
        void writeInfoPlist();
        void writeUIInterfaceOrientationInfo(void* parent);
        
    private:
        void writeCMakeList();
        
    private:
        String                  m_rootDir;
        String                  m_projectDir;
        String                  m_outputDir;
        String                  m_solutionDir;
        ResourcePath            m_iconRes;
        String                  m_appName;
        String                  m_identifier;
        bool                    m_uiInterfaceOrientationPortrait = true;
        bool                    m_uiInterfaceOrientationPortraitUpsideDown = true;
        bool                    m_uiInterfaceOrientationLandscapeLeft = true;
        bool                    m_uiInterfaceOrientationLandscapeRight = true;
    };
}
