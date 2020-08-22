#include "gltf_importer.h"
#include "gltf_loader.h"
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
			if (IO::instance()->convertFullPathToResPath(targetFolder, m_targetFoler))
			{
				m_gltfFile = EditorApi.selectAFile("Import GLTF", "*.gltf");
				if (!m_gltfFile.empty())
				{
					Gltf::Loader loader;
					if (loader.load(m_gltfFile))
					{
						// save meshes
						saveMeshs(loader);
					}
				}
			}
		}
	}

	void GltfImporter::saveMeshs(Gltf::Loader& loader)
	{
		for (Gltf::MeshInfo& meshInfo : loader.m_meshes)
		{
			for (Gltf::Primitive& primitiveInfo : meshInfo.m_primitives)
			{
				MeshPtr mesh = primitiveInfo.m_mesh;
				if (mesh)
				{
					mesh->setPath(m_targetFoler + "/" + meshInfo.m_name + ".mesh");
					mesh->save();
				}
			}
		}
	}
}
#endif
