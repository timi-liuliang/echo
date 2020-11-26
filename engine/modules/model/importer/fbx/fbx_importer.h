#pragma once

#include "engine/core/editor/importer.h"

#ifdef ECHO_EDITOR_MODE
#include "thirdparty/openfbx/ofbx.h"

namespace Echo
{
	class FbxImporter : public Importer
	{
		ECHO_CLASS(FbxImporter, Importer)

	public:
		FbxImporter();
		virtual ~FbxImporter();

		// name
		virtual const char* getName() override { return "FBX (*.fbx)"; }

		// import
		virtual void run(const char* targetFolder) override;

		// save
		void saveMeshs(ofbx::IScene* fbxScene, const String& fbxFile);

	private:
		String		m_targetFoler;
	};
}
#endif
