#include "VsGenMode.h"
#include <string>
#include <QProcess>
#include <QSettings>
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/io/io.h>

static const char* g_batchGotAdmin = R"(@echo off
:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------    

)";

static const char* g_runArguments = R"(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LocalDebuggerCommandArguments>${Args}</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LocalDebuggerCommandArguments>${Args}</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>
)";

Echo::String g_gameMainSourceCode = R"(#include <engine/core/main/module.h>

namespace Echo
{
	void registerGameModules()
	{
		// REGISTER_MODULE(YourGameModuleName)
	}
}
)";

Echo::String g_gameCMakeListsTxt = R"(
# set module name
SET(MODULE_NAME ${ECHO_GAME_NAME})

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES ${ECHO_GAME_SOURCE_PATH}/*.h)
FILE( GLOB_RECURSE SOURCE_FILES ${ECHO_GAME_SOURCE_PATH}/*.cpp)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${ECHO_GAME_SOURCE_PATH})

# add macro
IF(ECHO_GAME_SOURCE)
	ADD_DEFINITIONS(-DECHO_GAME_SOURCE)
ENDIF()

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# link libraries
TARGET_LINK_LIBRARIES(${MODULE_NAME} )

# no cmd window
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "game")

# log
MESSAGE(STATUS "Configure success!")

)";

Echo::String g_gameRootCMakeListsTxt = R"(
# Debug message
MESSAGE(STATUS "Add subdirectory [${CMAKE_CURRENT_SOURCE_DIR}]")

# Set this directory as ECHO_GAME_ROOT_PATH
SET(ECHO_GAME_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Add game library
ADD_SUBDIRECTORY("${ECHO_GAME_NAME}")

# Add thirdparty libraries


# Add custom editor


)";

namespace Echo
{
	bool VsGenMode::exec(int argc, char* argv[])
	{
		using namespace Echo;

		Echo::String type = argv[1];
		if (type == "vs")
		{
			Echo::String project = argv[2];
			Echo::PathUtil::FormatPath(project);

			Echo::String editor = argv[0];
			Echo::PathUtil::FormatPath(editor);

			Echo::String projectPath = Echo::PathUtil::GetFileDirPath(project);
			Echo::String projectName = Echo::PathUtil::GetPureFilename(project, false);
			Echo::String projectSrcPath = projectPath + "Source/";
			Echo::String projectGameSrcPath = projectSrcPath + projectName + "/";
			Echo::String buildPath = projectPath + "Build/";
			Echo::String binaryPath = projectPath + "Bin/";
			Echo::String vsVersion = "-G\"Visual Studio 17 2022\" -A x64";
			Echo::String batFile = buildPath + "cmake.bat";
			Echo::String vsSolutionFile = projectPath + projectName + ".sln";

			Echo::String engineEditExePath = Echo::PathUtil::GetFileDirPath(editor);
			Echo::String enginePath = PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(engineEditExePath))));

			// Root CMakeLists.txt
			writeCMakeLists(projectSrcPath + "CMakeLists.txt", g_gameRootCMakeListsTxt);

			// Game source
			writeGameMain(projectGameSrcPath + "GameMain.cpp");
			writeCMakeLists(projectGameSrcPath + "CMakeLists.txt", g_gameCMakeListsTxt);
			writeCMakeBatFile(projectName.c_str(), projectSrcPath.c_str(), batFile.c_str(), enginePath.c_str(), vsSolutionFile.c_str());

#ifdef ECHO_PLATFORM_WINDOWS
			Echo::String cmd = StringUtil::Format("%s", batFile.c_str());

			QProcess process;
			process.start("cmd.exe", QStringList() << "/c" << batFile.c_str());
			process.waitForFinished();

			Echo::String output(process.readAllStandardOutput());
			printf(output.c_str());

			Echo::String userFilePath = buildPath + "editor/echo/echo.vcxproj.user";
			Echo::String args = "open " + project;
			writeVcxprojUser(userFilePath, args);
#endif

			return true;
		}

		return false;
	}

	void VsGenMode::writeCMakeBatFile(const char* projectName, const char* projectSrcPath, const char* batFile, const char* enginePath, const char* vsSolutionFile)
	{
		using namespace Echo;

		String batSrc;

		StringUtil::WriteLine(batSrc, g_batchGotAdmin);

		StringUtil::WriteLine(batSrc, ":: start in current directory ");
		StringUtil::WriteLine(batSrc, "cd /D \"%~dp0\"\n");

		//-DECHO_GAME=TRUE -DECHO_GAME_NAME=moon -DECHO_GAME_SOURCE=D:/github/myprojects/moon/Source
		StringUtil::WriteLine(batSrc, StringUtil::Format("call cmake.exe -DECHO_GAME_SOURCE=TRUE -DECHO_GAME_NAME=\"%s\" -DECHO_GAME_SOURCE_PATH=\"%s\" %s\n", projectName, projectSrcPath, enginePath).c_str());

		StringUtil::WriteLine(batSrc, ":: make link");
		StringUtil::WriteLine(batSrc, "cd /D \"%~dp0\"");
		StringUtil::WriteLine(batSrc, "cd ../\n");
		StringUtil::WriteLine(batSrc, Echo::StringUtil::Format("if not exist .\\%s.sln mklink .\\%s.sln .\\Build\\echo.sln\n", projectName, projectName));

		StringUtil::WriteLine(batSrc, ":: delay close");
		StringUtil::WriteLine(batSrc, "timeout /t 10");

		IO::instance()->saveStringToFile(batFile, batSrc);
	}

	void VsGenMode::writeVcxprojUser(const Echo::String& userFilePath, const Echo::String& args)
	{
		Echo::String userSrc = Echo::StringUtil::Replace(g_runArguments, "${Args}", args.c_str());
		IO::instance()->saveStringToFile(userFilePath, userSrc);
	}

	void VsGenMode::writeGameMain(const Echo::String& gameMainCppFilePath)
	{
		if (!Echo::PathUtil::IsFileExist(gameMainCppFilePath))
			IO::instance()->saveStringToFile(gameMainCppFilePath, g_gameMainSourceCode);
	}

	void VsGenMode::writeCMakeLists(const Echo::String& cmakeListsFilePath, const Echo::String& content)
	{
		if (!Echo::PathUtil::IsFileExist(cmakeListsFilePath))
			IO::instance()->saveStringToFile(cmakeListsFilePath, content);
	}
}
