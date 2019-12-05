#pragma once

#include <engine/core/main/module.h>

namespace Echo
{
    class EditorSettingsModule : public Module
    {
		ECHO_SINGLETON_CLASS(EditorSettingsModule, Module)

    public:
		EditorSettingsModule();
        virtual ~EditorSettingsModule();

		// instance
		static EditorSettingsModule* instance();

        // register all types of the module
        virtual void registerTypes() override;

		// always enable this module
		virtual void setEnable(bool isEnable) override { m_isEnable = true; }

		// is for editor
		virtual bool isEditorOnly() override { return true; }
    };
}
