#pragma once

#include <QProcess>
#include "ui_BuildWindow.h"
#include <engine/core/util/StringUtil.h>
#include "build_log.h"

namespace Studio
{
	class BuildWindow : public QMainWindow, public Ui_BuildWindow, public Echo::BuildLog
	{
        Q_OBJECT
    
    public:
		BuildWindow(QWidget* parent=0);
		~BuildWindow();
        
        // log
        virtual void log(const char* msg) override;

	private:
        // init target platform list
        void initPlatformList();
        
    private slots:
        // on build
        void onBuild();
        
        // switch target platform
        void onPlatformChanged();
        
    private:
        Echo::String    m_targetPlatform = "Android";
	};
}
