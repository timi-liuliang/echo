#include "fbx_importer.h"
#include "engine/core/editor/editor.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/io.h"
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
		if (IO::instance()->convertFullPathToResPath(targetFolder, m_targetFoler))
		{
			QStringList resFiles = QFileDialog::getOpenFileNames(nullptr, "Import Fbx", "", "*.fbx");
			for (const QString& qfbxFile : resFiles)
			{
				String fbxFile = qfbxFile.toStdString().c_str();
				if (!fbxFile.empty())
				{
					MemoryReader memReader(fbxFile);
					if (memReader.getSize())
					{
						ofbx::IScene* fbxScene = ofbx::load(memReader.getData<ofbx::u8*>(), memReader.getSize(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
						if (fbxScene)
						{
							// meshes
							saveMeshs(fbxScene, fbxFile);

							fbxScene->destroy();
						}
					}
				}
			}
		}
	}

	#define CM_TO_M 0.01

	void FbxImporter::saveMeshs(ofbx::IScene* fbxScene, const String& fbxFile)
	{
		for (int i = 0; i < fbxScene->getMeshCount(); i++)
		{
			const ofbx::Mesh* fbxMesh = fbxScene->getMesh(i);
			if (fbxMesh)
			{
				const ofbx::Geometry* geometry = fbxMesh->getGeometry();
				if (geometry)
				{
					ofbx::Matrix meshMatrix = fbxMesh->getGeometricMatrix();

					MeshVertexFormat vertFormat;
					vertFormat.m_isUseNormal = geometry->getNormals() ? true : false;
					vertFormat.m_isUseUV = geometry->getUVs(0) ? true : false;

					MeshVertexData vertexData;
					vertexData.set(vertFormat, geometry->getVertexCount());

					// position cm->m
					for (int j = 0; j < geometry->getVertexCount(); j++)
					{
						const ofbx::Vec3 fbxPosition = geometry->getVertices()[j];
						vertexData.setPosition(j, Vector3(fbxPosition.x, fbxPosition.y, fbxPosition.z) * fbxScene->getGlobalSettings()->UnitScaleFactor * CM_TO_M);
					}

					// normals
					if (geometry->getNormals())
					{
						for (int j = 0; j < geometry->getVertexCount(); j++)
						{
							const ofbx::Vec3 fbxNormal = geometry->getNormals()[j];
							vertexData.setNormal(j, Vector3(fbxNormal.x, fbxNormal.y, fbxNormal.z));
						}
					}

					// uvs
					if (geometry->getUVs(0))
					{
						for (int j = 0; j < geometry->getVertexCount(); j++)
						{
							const ofbx::Vec2 fbxUv = geometry->getUVs(0)[j];
							vertexData.setUV0(j, Vector2(fbxUv.x, fbxUv.y));
						}
					}

					// update vertices data
					MeshPtr mesh = Mesh::create(true, true);
					mesh->updateVertexs(vertexData);

					// indices
					const i32* fbxFaceIndices = geometry->getFaceIndices();
					if (geometry->getVertexCount() < 65535)
					{
						vector<i16>::type shortIndices;
						for (int j = 0; j < geometry->getIndexCount(); ++j)
						{
							int idx = (fbxFaceIndices[j] < 0) ? (-fbxFaceIndices[j] - 1)  : (fbxFaceIndices[j]);
							shortIndices.push_back(idx);
						}

						mesh->updateIndices(static_cast<ui32>(geometry->getIndexCount()), sizeof(i16), shortIndices.data());
					}
					else
					{
						vector<i32>::type intIndices;
						for (int j = 0; j < geometry->getIndexCount(); ++j)
						{
							int idx = (fbxFaceIndices[j] < 0) ? (-fbxFaceIndices[j] - 1) : (fbxFaceIndices[j]);
							intIndices.push_back(idx);
						}

						mesh->updateIndices(static_cast<ui32>(geometry->getIndexCount()), sizeof(i32), intIndices.data());
					}

					if (mesh)
					{
						String meshName = PathUtil::GetPureFilename(fbxFile, false);
						mesh->setPath(m_targetFoler + "/" + meshName + ".mesh");
						mesh->save();
					}
				}
			}
		}
	}
}
#endif
