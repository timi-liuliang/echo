#include "fbx_importer.h"
#include "engine/core/editor/editor.h"
#include "engine/core/util/PathUtil.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	FbxImporter::FbxImporter()
	{
	}

	FbxImporter::~FbxImporter()
	{

	}

	void FbxImporter::bindMethods()
	{

	}

	void FbxImporter::run(const char* targetFolder)
	{
		if (m_fbxFile.empty())
		{
			if (IO::instance()->convertFullPathToResPath(targetFolder, m_targetFoler))
			{
				m_fbxFile = EditorApi.selectAFile("Import Fbx", "*.fbx");
				if (!m_fbxFile.empty())
				{
					Fbx::Loader loader;
					if (loader.load(m_fbxFile))
					{
						// save meshes
						saveMeshs(loader);
					}
				}
			}
		}
	}

	void FbxImporter::saveMeshs(Fbx::Loader& loader)
	{
	}
}
#endif
