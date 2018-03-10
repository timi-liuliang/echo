#pragma once

#include "Engine/Core.h"

namespace Echo
{
	/**
	 * EngineConsole 引擎控制台 
	 */
	class EngineConsole
	{
	public:
		// 执行指令
		void exec( const String& cmd, String& output);

	private:
		// 解析
		void parse( const StringArray& argv, String& output);

		//渲染队列相关处理
		void parseRenderQueue( const StringArray& argv, String& output);

		// 获取变量
		void parseGet( const StringArray& argv, String& output);
	};
}