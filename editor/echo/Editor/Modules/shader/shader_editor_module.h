#pragma once

#include <engine/core/main/module.h>

namespace Echo
{
    class ShaderEditorModule : public Module
    {
		ECHO_SINGLETON_CLASS(ShaderEditorModule, Module)

    public:
		ShaderEditorModule();
        virtual ~ShaderEditorModule();

		// instance
		static ShaderEditorModule* instance();

        // register all types of the module
        virtual void registerTypes() override;

		// always enable this module
		virtual void setEnable(bool isEnable) override { m_isEnable = true; }

		// is for editor
		virtual bool isEditorOnly() override { return true; }
    };
}
