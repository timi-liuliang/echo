#include <QApplication>
#include <QDesktopWidget>
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
		: QMainWindow( parent)
	{
		setupUi( this);

#ifdef ECHO_PLATFORM_WINDOWS
        // hide window hwnd
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
        // set title bar color
        macChangeTitleBarColor(winId(), 66.f/255.f, 66.f/255.f, 66.f/255.f);
#endif

		// set icon
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");
        
        // splitter stretch
        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 1);
        
        initPlatformList();
        
        QObject::connect(m_buildButton, SIGNAL(clicked()), this, SLOT(onBuild()));
        QObject::connect(m_platformList, SIGNAL(currentRowChanged(int)), this, SLOT(onPlatformChanged()));
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

    void BuildWindow::onBuild()
    {
        if(m_targetPlatform=="iOS")
        {
            Echo::BuildSettings* buildSettings = ECHO_DOWN_CAST<Echo::BuildSettings*>(Echo::Class::create(ECHO_CLASS_NAME(iOSBuildSettings)));
            if(buildSettings)
            {
                buildSettings->setLog(this);
                buildSettings->build();
            }
        }
    }

    void BuildWindow::log(const char* msg)
    {
        m_log->append( msg);
    }
}
