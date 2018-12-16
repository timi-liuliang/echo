#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * 命令行解析 2012-8-16 帝林
	 */
	class CMDLine
	{
	public:
		// 解析主入口
		static bool Parser(int argc, char* argv[]);
	};

	/**
	 * EditorMode
	 */
	class EditorMode
	{
	public:
		// exec command
		bool exec(int argc, char* argv[]);
	};

	// EditOpen
	class EditOpenMode
	{
	public:
		// exec command
		bool exec(int argc, char* argv[]);
	};

	/**
	 * GameMode
	 */
	class GameMode
	{
	public:
		// exec command
		bool exec(int argc, char* argv[]);
	};
}