#include "gltf_importer.h"
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	GltfImporter::GltfImporter()
	{
	}

	GltfImporter::~GltfImporter()
	{

	}

	void GltfImporter::bindMethods()
	{

	}

	void GltfImporter::run(const char* targetFolder)
	{
		if (m_gltfFile.empty())
		{
			m_gltfFile = EditorApi.selectAFile("Import GLTF", "*.gltf");
			if (!m_gltfFile.empty())
			{
				int a = 10;
			}
		}
	}
}
#endif