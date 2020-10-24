#include "fbx_importer.h"
#include "engine/core/editor/editor.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/mesh/mesh.h"

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
					MemoryReader memReader(m_fbxFile);
					if (memReader.getSize())
					{
						ofbx::IScene* fbxScene = ofbx::load(memReader.getData<ofbx::u8*>(), memReader.getSize(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
						if (fbxScene)
						{
							// meshes
							saveMeshs(fbxScene);

							fbxScene->destroy();
						}
					}
				}
			}
		}
	}

	void FbxImporter::saveMeshs(ofbx::IScene* fbxScene)
	{
		for (int i = 0; i < fbxScene->getMeshCount(); i++)
		{
			const ofbx::Mesh* fbxMesh = fbxScene->getMesh(i);
			if (fbxMesh)
			{
				const ofbx::Geometry* geometry = fbxMesh->getGeometry();
				if (geometry)
				{
					MeshVertexFormat vertFormat;
					vertFormat.m_isUseNormal = geometry->getNormals() ? true : false;

					MeshVertexData vertexData;
					vertexData.set(vertFormat, geometry->getVertexCount());

					// position
					for (int j = 0; j < geometry->getVertexCount(); j++)
					{
						const ofbx::Vec3 fbxPosition = geometry->getVertices()[j];
						vertexData.setPosition(j, Vector3(fbxPosition.x, fbxPosition.y, fbxPosition.z));
					}

					// normals
					for (int j = 0; j < geometry->getVertexCount(); j++)
					{
						const ofbx::Vec3 fbxNormal = geometry->getNormals()[j];
						vertexData.setNormal(j, Vector3(fbxNormal.x, fbxNormal.y, fbxNormal.z));
					}

					MeshPtr mesh = Mesh::create(true, true);
					mesh->updateIndices(static_cast<ui32>(geometry->getIndexCount()), sizeof(i32), geometry->getFaceIndices());
					mesh->updateVertexs(vertexData);

					if (mesh)
					{
						String meshName = PathUtil::GetPureFilename(m_fbxFile, false);
						mesh->setPath(m_targetFoler + "/" + meshName + ".mesh");
						mesh->save();
					}
				}
			}
		}
	}
}
#endif
