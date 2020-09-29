#pragma once

#include "build_settings.h"

namespace Echo
{
	class WebAssemblyBuildSettings : public BuildSettings
	{
		ECHO_SINGLETON_CLASS(WebAssemblyBuildSettings, BuildSettings)

	public:
		WebAssemblyBuildSettings();
		virtual ~WebAssemblyBuildSettings();

		// instance
		static WebAssemblyBuildSettings* instance();

		// get name
		virtual const char* getPlatformName() const override { return "WebAssembly"; }

		// platform thumbnail
		virtual ImagePtr getPlatformThumbnail() const override;

		// build
		virtual void build() override;

		// set
		virtual void setOutputDir(const String& outputDir) override;

		// get final result path
		virtual String getFinalResultPath() override;

		// icon res path
		void setIconRes(const ResourcePath& path);
		const ResourcePath& getIconRes() { return m_iconRes; }

	private:
		// output directory
		bool prepare();

		// copy
		void copySrc();
		void copyRes();

		// cmake
		void cmake();

		// compile
		void compile();

		// write config
		void writeModuleConfig();

	private:
		String                  m_rootDir;
		String                  m_projectDir;
		String                  m_outputDir;
		ResourcePath            m_iconRes;
	};
}
