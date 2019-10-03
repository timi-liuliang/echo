#include "build_module.h"
#include "ios_buildsettings.h"
#include "mac_buildsettings.h"
#include "android_buildsettings.h"
#include "windows_buildsettings.h"

namespace Echo
{
    BuildModule::BuildModule()
    {
    }

    BuildModule::~BuildModule()
    {

    }

    void BuildModule::registerTypes()
    {
        Class::registerType<BuildSettings>();
        Class::registerType<iOSBuildSettings>();
        Class::registerType<MacBuildSettings>();
        Class::registerType<AndroidBuildSettings>();
        Class::registerType<WindowsBuildSettings>();
    }
}
