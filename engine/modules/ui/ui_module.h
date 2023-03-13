#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class UiModule : public Module
	{
		ECHO_SINGLETON_CLASS(UiModule, Module)

	public:
		UiModule();
		virtual ~UiModule();

		// instance
		static UiModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	
		// UiImage default material
		void setUiImageDefaultShader(const ResourcePath& path) { m_uiImageDefaultShader.setPath(path.getPath()); }
		const ResourcePath& getUiImageDefaultShader() { return m_uiImageDefaultShader; }

	protected:
		ResourcePath	m_uiImageDefaultShader = ResourcePath("Module://Ui/Transparent.shader", ".shader");
	};
}
