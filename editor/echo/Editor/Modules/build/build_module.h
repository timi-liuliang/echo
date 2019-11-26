#pragma once

#include <engine/core/main/module.h>

namespace Echo
{
    class BuildModule : public Module
    {
		ECHO_SINGLETON_CLASS(BuildModule, Module)

    public:
        BuildModule();
        virtual ~BuildModule();

		// instance
		static BuildModule* instance();

        // register all types of the module
        virtual void registerTypes() override;

		// always enable this module
		virtual void setEnable(bool isEnable) override { m_isEnable = true; }
    };
}
