#pragma once

class QSplashScreen;

namespace Echo
{
	/**
	 * 命令行解析 2012-8-16 帝林
	 */
	class CMDLine
	{
	public:
		// 解析主入口
		static bool Parser(int argc, char* argv[], QSplashScreen* splash);

		// 释放
		static void Release();
	};
}