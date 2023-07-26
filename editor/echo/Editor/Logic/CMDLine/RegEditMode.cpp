#include "RegEditMode.h"
#include <string>
#include <QProcess>
#include <QSettings>
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/io/io.h>

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
