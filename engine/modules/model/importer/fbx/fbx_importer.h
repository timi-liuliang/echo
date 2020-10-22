#pragma once

#include "engine/core/editor/importer.h"
#include "fbx_loader.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class FbxImporter : public Importer
	{
		ECHO_CLASS(FbxImporter, Importer)

	public:
		FbxImporter();
		virtual ~FbxImporter();

		// name
		virtual const char* getName() override { return "FBX"; }

		// import
		virtual void run(const char* targetFolder) override;

		// save
		void saveMeshs(Fbx::Loader& loader);

	private:
		String		m_fbxFile;
		String		m_targetFoler;
	};
}
#endif
