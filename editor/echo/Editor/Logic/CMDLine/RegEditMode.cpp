#include "RegEditMode.h"
#include <string>
#include <QProcess>
#include <QSettings>
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/io/io.h>

#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace Echo
{
	bool RegEditMode::exec(int argc, char* argv[])
	{
		Echo::String type = argv[1];
		if (type != "regedit")
			return false;

#ifdef ECHO_PLATFORM_WINDOWS
		Echo::String buildToolFilePath = argv[0];
		Echo::PathUtil::FormatPath(buildToolFilePath);

		Echo::String editorFilePath = Echo::PathUtil::GetFileDirPath(buildToolFilePath) + "Echo.exe";
		Echo::String regRootPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes";
		Echo::String regShellPath = regRootPath + "\\Echo.ProjectFile\\shell";

		setDefaultValue((regRootPath + "\\.echo").c_str(), "Echo.ProjectFile");
		setDefaultValue((regRootPath + "\\Echo.ProjectFile").c_str(), "Echo Engine Project File");
		setDefaultValue((regRootPath + "\\Echo.ProjectFile\\DefaultIcon").c_str(), editorFilePath.c_str());

		addRightMenu(regShellPath, "open", "Open", editorFilePath.c_str(), (editorFilePath + " open %1").c_str());
		addRightMenu(regShellPath, "rungenproj", "Generate Visual Studio project files", editorFilePath.c_str(), (Echo::String(argv[0]) + " vs %1").c_str());
#endif

		return true;
	}

	void RegEditMode::setDefaultValue(const String& regPath, const String& value)
	{
		QSettings regItem(regPath.c_str(), QSettings::NativeFormat);
		regItem.setValue("Default", value.c_str());
	}

	void RegEditMode::addRightMenu(const String& shellPath, const String& name, const String& desc, const String& icon, const String& command)
	{
		QSettings regGenProj((shellPath + "\\" + name).c_str(), QSettings::NativeFormat);
		regGenProj.setValue("Default", desc.c_str());
		regGenProj.setValue("Icon", icon.c_str());

		QSettings regGenProjCommand((shellPath + "\\" + name + "\\command").c_str(), QSettings::NativeFormat);
		regGenProjCommand.setValue("Default", command.c_str());
	}

	void RegEditMode::check(const char* echoExe)
	{
	#if defined(ECHO_PLATFORM_WINDOWS) && !defined(ECHO_GAME_SOURCE)
		QSettings regIcon("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Echo.ProjectFile\\shell\\rungenproj\\command", QSettings::NativeFormat);
		Echo::String currentEdit = regIcon.value("Default").toString().toStdString().c_str();
		if (!Echo::StringUtil::StartWith(currentEdit, echoExe))
		{
			HINSTANCE result = ShellExecute(NULL, "runas", echoExe, "regedit", NULL, SW_SHOWNORMAL);
			assert(result != 0);
		}
	#endif
	}
}
