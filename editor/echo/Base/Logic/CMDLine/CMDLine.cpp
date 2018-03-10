#include "CMDLine.h"
#include <string>
#include <QDir>
#include <QSplashScreen>
#include "Studio.h"
#include "EchoEngine.h"

namespace Echo
{
	Studio::AStudio* g_astudio = NULL;

	// 解析主入口
	bool CMDLine::Parser(int argc, char* argv[], QSplashScreen* splash)
	{
		if ( argc > 1 )
		{	
			StringArray sargv;
			for( int i=1; i<argc; i++)
				sargv.push_back( argv[i]);

			//命令行编辑器
			g_astudio = new Studio::AStudio(" ");
			g_astudio->setAppPath(QDir::currentPath().toStdString().c_str());

			return true;
		}
		else
		{
			// 基础编辑器
			TIME_PROFILE
			(
				g_astudio = new Studio::AStudio;
				g_astudio->setAppPath(QDir::currentPath().toStdString().c_str());
			)

			// 加载配置
			TIME_PROFILE
			(
				g_astudio->getConfigMgr()->loadCfgFile();
			)
			
			TIME_PROFILE
			(
				g_astudio->Start();
			)

			// 显示主窗口
			TIME_PROFILE
			(
				ThreadSleepByMilliSecond(1000);
				g_astudio->getProjectWindow()->show();
				splash->finish(g_astudio->getProjectWindow());
			)
		}

		return false;
	}

	// 释放
	void CMDLine::Release()
	{
		delete g_astudio;
	}
}
