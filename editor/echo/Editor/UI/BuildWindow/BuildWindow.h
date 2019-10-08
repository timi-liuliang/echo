#pragma once

#include <QProcess>
#include "ui_BuildWindow.h"
#include <engine/core/util/StringUtil.h>
#include "build_log.h"
#include "build_settings.h"

namespace Studio
{
	class BuildWindow : public QMainWindow, public Ui_BuildWindow, public Echo::BuildLog
	{
        Q_OBJECT
    
    public:
		BuildWindow(QWidget* parent=0);
		~BuildWindow();
        
    public:
        // log
        virtual void log(const char* msg) override;
        
        // on begin|finished
        virtual void onBegin() override;
        virtual void onEnd() override;

	private:
        // init target platform list
        void initPlatformList();
        
        // get build settings
        Echo::BuildSettings* getBuildSettings();
        
    private slots:
        // on build
        void onBuild();
        
        // switch target platform
        void onPlatformChanged();
        
        // show result
        void onShowResultInExplorer();
        
    private:
        Echo::String    m_targetPlatform = "Android";
	};
}
