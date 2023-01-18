#include "CMDLine.h"
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

namespace Echo
{
	bool CMDLine::Parser(int argc, char* argv[])
	{
		if ( argc > 1 )
		{	
			StringArray sargv;
			for( int i=1; i<argc; i++)
				sargv.emplace_back( argv[i]);

			if (sargv[0] == "vs")
			{
				VsGenMode vsGenMode;
				vsGenMode.exec(argc, argv);
			}
			else if (sargv[0] == "regedit")
			{
				RegEditMode regEditMode;
				regEditMode.exec(argc, argv);
			}

			return true;
		}
		else
		{
		}

		return false;
	}

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
			Echo::String buildPath = projectPath + "Build/";
			Echo::String binaryPath = projectPath + "Bin/";
			Echo::String vsVersion = "-G\"Visual Studio 17 2022\" -A x64";
			Echo::String batFile = buildPath + "cmake.bat";
			Echo::String vsSolutionFile = projectPath + projectName + ".sln";

			Echo::String engineEditExePath = Echo::PathUtil::GetFileDirPath(editor);
			Echo::String enginePath = PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(PathUtil::GetParentPath(engineEditExePath))));

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

			Echo::String gameMainCppFilePath = projectSrcPath + "GameMain.cpp";
			writeGameMain(gameMainCppFilePath);
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
		{
			IO::instance()->saveStringToFile(gameMainCppFilePath, g_gameMainSourceCode);
		}
	}

	bool RegEditMode::exec(int argc, char* argv[])
	{
		Echo::String type = argv[1];
		if (type != "regedit")
			return false;

#ifdef ECHO_PLATFORM_WINDOWS
		Echo::String buildToolFilePath = argv[0];
		Echo::PathUtil::FormatPath(buildToolFilePath);

		Echo::String editorFilePath = Echo::PathUtil::GetFileDirPath(buildToolFilePath) + "Echo.exe";

		QSettings regOpenIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Open", QSettings::NativeFormat);
		regOpenIcon.setValue("Icon", editorFilePath.c_str());

		QSettings regOpen("HKEY_CLASSES_ROOT\\.echo\\shell\\Open\\command", QSettings::NativeFormat);
		regOpen.setValue("Default", (editorFilePath + " open %1").c_str());

		QSettings regIcon("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files", QSettings::NativeFormat);
		regIcon.setValue("Icon", editorFilePath.c_str());

		QSettings regGv("HKEY_CLASSES_ROOT\\.echo\\shell\\Generate Visual Studio Files\\command", QSettings::NativeFormat);
		regGv.setValue("Default", (Echo::String(argv[0]) + " vs %1").c_str());
#endif

		return true;
	}
}
