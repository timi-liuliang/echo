#include "gltf_importer.h"
#include "gltf_loader.h"
#include "engine/core/editor/editor.h"
#include "engine/core/util/PathUtil.h"

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
		for(size_t i =0; i < loader.m_meshes.size(); i++)
		{
			Gltf::MeshInfo& meshInfo = loader.m_meshes[i];
			for (size_t j=0; j < meshInfo.m_primitives.size(); j++)
			{
				MeshPtr mesh = meshInfo.m_primitives[j].m_mesh;
				if (mesh)
				{
					if (!meshInfo.m_name.empty())
					{
						mesh->setPath(m_targetFoler + "/" + meshInfo.m_name  +".mesh");
						mesh->save();
					}
					else
					{
						String meshName = PathUtil::GetPureFilename(loader.m_path.getPath(), false);
						if (i != 0 || j != 0)
							meshName += StringUtil::Format("%d_%d", i, j);

						mesh->setPath(m_targetFoler + "/" + meshName + ".mesh");
						mesh->save();
					}
				}
			}
		}
	}
}
#endif
