#pragma once

#include <QProcess>
#include "ui_BuildWindow.h"
#include <engine/core/util/StringUtil.h>
#include "build_settings.h"

namespace Studio
{
	class BuildWindow : public QDialog, public Ui_BuildWindow, public Echo::BuildListener
	{
        Q_OBJECT

    public:
		BuildWindow(QWidget* parent=0);
		~BuildWindow();

    public:
        // log
        virtual void log(const char* msg, bool isWrap=true) override;

        // on begin|finished
        virtual void onBegin() override;
        virtual void onEnd() override;

        // on exec cmd
        virtual bool onExecCmd(const char* cmd, const char* workingDir) override;

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

        // receive msg from other process
        void onReadMsgFromCmdProcess();
        
        // on process error
        void onProcessError(QProcess::ProcessError error);

    private:
        Echo::String    m_targetPlatform = "Android";
        QProcess		m_cmdProcess;
	};
}
