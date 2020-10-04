#include "build_module.h"
#include "ios_build_settings.h"
#include "mac_build_settings.h"
#include "android_build_settings.h"
#include "windows_build_settings.h"
#include "wechat_build_settings.h"
#include "was_build_settings.h"
#include "html5_build_settings.h"

namespace Echo
{
	DECLARE_MODULE(BuildModule)

    BuildModule::BuildModule()
    {
    }

    BuildModule::~BuildModule()
    {
        EchoSafeDeleteInstance(iOSBuildSettings);
        EchoSafeDeleteInstance(AndroidBuildSettings);
        EchoSafeDeleteInstance(MacBuildSettings);
        EchoSafeDeleteInstance(WindowsBuildSettings);
        EchoSafeDeleteInstance(WeChatBuildSettings);
        EchoSafeDeleteInstance(WebAssemblyBuildSettings);
        EchoSafeDeleteInstance(Html5BuildSettings);
    }

	BuildModule* BuildModule::instance()
	{
		static BuildModule* inst = EchoNew(BuildModule);
		return inst;
	}

	void BuildModule::bindMethods()
	{

	}

    void BuildModule::registerTypes()
    {
        Class::registerType<BuildSettings>();
        Class::registerType<iOSBuildSettings>();
        Class::registerType<MacBuildSettings>();
        Class::registerType<AndroidBuildSettings>();
        Class::registerType<WindowsBuildSettings>();
        Class::registerType<WeChatBuildSettings>();
        Class::registerType<WebAssemblyBuildSettings>();
        Class::registerType<Html5BuildSettings>();
    }
}
