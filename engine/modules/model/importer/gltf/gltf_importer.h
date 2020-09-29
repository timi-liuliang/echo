#pragma once

#include "engine/core/editor/importer.h"
#include "gltf_loader.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class GltfImporter : public Importer
	{
		ECHO_CLASS(GltfImporter, Importer)

	public:
		GltfImporter();
		virtual ~GltfImporter();

		// name
		virtual const char* getName() override { return "GLTF"; }

		// import
		virtual void run(const char* targetFolder) override;

		// save
		void saveMeshs(Gltf::Loader& loader);

	private:
		String		m_gltfFile;
		String		m_targetFoler;
	};
}
#endif
