#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
	/**
	 * Command Parser 2012-8-16 Liang
	 */
	class CMDLine
	{
	public:
		// Parse
		static bool Parser(int argc, char* argv[]);
	};

	/**
	 * Generate Visual Studio project files
	 */
	class VsGenMode
	{
	public:
		// Exec command
		bool exec(int argc, char* argv[]);

		// Write cmake.bat
		void writeCMakeBatFile(const char* projectName, const char* projectSrcPath, const char* batFile, const char* enginePath, const char* vsSolutionFile);

		// Write echo.vcxproj.user
		void writeVcxprojUser(const Echo::String& userFilePath, const Echo::String& args);

		// Write GameMain.cpp
		void writeGameMain(const Echo::String& gameMainCppFilePath);

		// Wirte CMakeLists.txt
		void writeCMakeLists(const Echo::String& cmakeListsFilePath, const Echo::String& content);
	};

	/**
	 * Link echo file
	 */
	class RegEditMode
	{
	public:
		// Exec command
		bool exec(int argc, char* argv[]);
	};
}