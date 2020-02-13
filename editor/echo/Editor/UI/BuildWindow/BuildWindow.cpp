#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFileDialog>
#include "BuildWindow.h"
#include "engine/core/util/PathUtil.h"
#include "MacHelper.h"
#include "ios_build_settings.h"
#include "mac_build_settings.h"
#include "android_build_settings.h"
#include "windows_build_settings.h"

namespace Studio
{
	BuildWindow::BuildWindow(QWidget* parent/*=0*/)
		: QDialog( parent)
	{
		setupUi( this);

        // hide window hwnd
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// set icon
        m_menuBar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

        // splitter stretch
        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 1);

        initPlatformList();

        QObject::connect(m_buildButton, SIGNAL(clicked()), this, SLOT(onBuild()));
        QObject::connect(m_showInExplorerButton, SIGNAL(clicked()), this, SLOT(onShowResultInExplorer()));
        QObject::connect(m_platformList, SIGNAL(currentRowChanged(int)), this, SLOT(onPlatformChanged()));
        QObject::connect(&m_cmdProcess, SIGNAL(readyRead()), this, SLOT(onReadMsgFromCmdProcess()));
        QObject::connect(&m_cmdProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcessError(QProcess::ProcessError)));
	}

	BuildWindow::~BuildWindow()
	{
	}

    void BuildWindow::initPlatformList()
    {
        m_platformList->setIconSize(QSize(28, 28));

        m_platformList->addItem(new QListWidgetItem(QIcon(":/icon/Icon/build/android.png"), "Android"));
        m_platformList->addItem(new QListWidgetItem(QIcon(":/icon/Icon/build/ios.png"),     "iOS"));
        m_platformList->addItem(new QListWidgetItem(QIcon(":/icon/Icon/build/mac.png"),     "Mac"));
        m_platformList->addItem(new QListWidgetItem(QIcon(":/icon/Icon/build/windows.png"), "Windows"));
    }

    void BuildWindow::onPlatformChanged()
    {
        m_targetPlatform = m_platformList->currentItem()->text().toStdString().c_str();
    }

    void BuildWindow::onShowResultInExplorer()
    {
        Echo::BuildSettings* buildSettings = getBuildSettings();
        if(buildSettings)
        {
            QString FinalResultPath = buildSettings->getFinalResultPath().c_str();
            if (!FinalResultPath.isEmpty())
            {
            #ifdef ECHO_PLATFORM_WINDOWS
                QDesktopServices::openUrl(FinalResultPath);
            #else
                QDesktopServices::openUrl(QUrl("file://" + FinalResultPath));
            #endif
            }
        }
    }

    void BuildWindow::onBuild()
    {
        QString outputDir = QFileDialog::getExistingDirectory(this, "Choose output directory");
        if(!outputDir.isEmpty())
        {
            Echo::BuildSettings* buildSettings = getBuildSettings();
            if(buildSettings)
            {
                buildSettings->setListener(this);
                buildSettings->setOutputDir(outputDir.toStdString().c_str());
                buildSettings->build();
            }
        }
    }

    Echo::BuildSettings* BuildWindow::getBuildSettings()
    {
        if(m_targetPlatform=="iOS")				return ECHO_DOWN_CAST<Echo::BuildSettings*>(Echo::Class::create(ECHO_CLASS_NAME(iOSBuildSettings)));
		else if(m_targetPlatform == "Windows")	return ECHO_DOWN_CAST<Echo::BuildSettings*>(Echo::Class::create(ECHO_CLASS_NAME(WindowsBuildSettings)));
        else									return nullptr;
    }

    void BuildWindow::log(const char* msg, bool isWrap)
    {
        static bool isNeedAddSpace = false;
        if(isWrap)
        {
            m_log->append( msg);
            isNeedAddSpace = true;
        }
        else
        {
            if(isNeedAddSpace)
            {
                m_log->insertPlainText("\n");
                isNeedAddSpace = false;
            }
            
            m_log->moveCursor(QTextCursor::End);
            m_log->insertPlainText( msg);
        }
    }

    void BuildWindow::onBegin()
    {
        m_showInExplorerButton->setEnabled(false);
    }

    void BuildWindow::onEnd()
    {
        m_showInExplorerButton->setEnabled(true);

        onShowResultInExplorer();
    }

    bool BuildWindow::onExecCmd(const char* cmd, const char* workingDir)
    {
        if(workingDir)
            m_cmdProcess.setWorkingDirectory(workingDir);

        m_cmdProcess.start(cmd);

        if(!m_cmdProcess.waitForStarted())
            return false;
        
        if(!m_cmdProcess.waitForFinished())
            return false;
        
        return true;
    }

    void BuildWindow::onReadMsgFromCmdProcess()
    {
        Echo::String msg = m_cmdProcess.readAllStandardOutput().toStdString().c_str();
        if (!msg.empty())
            log(msg.c_str(), false);
    }
    
    void BuildWindow::onProcessError(QProcess::ProcessError error)
    {
        switch(error)
        {
            case QProcess::FailedToStart:   log("build process failed to start.");  break;
            default:                        log("build process failed.");           break;
        }
    }
}
