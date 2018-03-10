#include "Engine/modules/Model/Mesh.h"
#include "engine/core/Util/StringUtil.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/LogManager.h"
#include "Engine/core/Render/TextureResManager.h"
#include "Render/Renderer.h"
#include "engine/core/resource/ResourceGroupManager.h"
#include "Render/PixelFormat.h"
#include "Render/GPUBuffer.h"
#include "engine/core/Util/StringUtil.h"
#include <engine/core/resource/DataStream.h>
#include <SubMesh.h>

namespace Echo
{
	// 构造函数
	Mesh::Mesh(const String& name)
		: Resource(name, RT_Mesh, false)
		, m_useSubMeshBoneMatrices(false)
		, m_isRetainPreparedData(false)
		, m_isSkinned(false)
		, m_isValid(false)
		, m_clone_name("")
		, m_isDisableDefaultTex(false)
	{
	}

	Mesh::~Mesh()
	{
		unloadImpl();
	}

	const Box& Mesh::getBox() const
	{
		return m_box;
	}

	// 重计算包围盒
	void Mesh::recalcBox()
	{
		m_box.reset();

		for (size_t i = 0; i < m_subMeshs.size(); i++)
			m_box.unionBox(m_subMeshs[i]->getLocalBox());
	}

	size_t Mesh::calculateSize() const
	{
		size_t result = 0;
		for ( size_t i=0; i<m_subMeshs.size(); i++)
			result += m_subMeshs[i]->calculateSize();

		return result;
	}

	bool Mesh::parseFromFileVersion_old(DataStream* stream, bool useHalfFloat)
	{
		m_box.reset();

		// 子模型数量
		i32 subMeshNum=0;
		stream->read(&subMeshNum, sizeof(i32));

		// 预留空间
		stream->skip(72);

		for( int i=0; i<subMeshNum; i++)
		{
			SubMesh* pSubMesh = EchoNew(SubMesh);
			EchoAssert(pSubMesh);

			// name
			ui32 smNameLen;
			char smName[260];
			stream->read(&smNameLen, sizeof(ui32));
			stream->read(&smName, smNameLen);
			smName[smNameLen] = 0;
			pSubMesh->m_name = smName;

			// material
			ui32 mtrlLen;
			char mtrlPath[260];
			stream->read(&mtrlLen, sizeof(ui32));
			stream->read(&mtrlPath, mtrlLen);
			mtrlPath[mtrlLen] = 0;
			String diffTexFile = mtrlPath;

			//to lower case.
			StringUtil::LowerCase(diffTexFile);

			if (!m_isDisableDefaultTex)
			{
				pSubMesh->m_pDiffTex = TextureResManager::instance()->createTexture(diffTexFile, Texture::TU_STATIC);
				pSubMesh->m_pDiffTex->prepare();
			}

			// skip 80 bytes
			i32	 lightmapResolution;
			stream->read(&pSubMesh->m_vertInfo.m_isUseNormal, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseVertexColor, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseLightmapUV, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseTangentBinormal, sizeof(bool));
			stream->read(&lightmapResolution, sizeof(i32));
			stream->skip(72);
			
			pSubMesh->m_vertInfo.m_isUseDiffuseUV = true;
			pSubMesh->m_vertInfo.m_isUseBoneData = m_isSkinned;
			pSubMesh->m_lightmapResolution = Echo::Math::Clamp( lightmapResolution, 8, 512);

			std::vector<int> floatPos;
			bool supportHalfFloat = false;
			pSubMesh->setVertexStructInfo(m_isSkinned, useHalfFloat && supportHalfFloat, floatPos);

			// load 骨骼索引数据
			if( m_isSkinned && m_useSubMeshBoneMatrices)
			{
				ui32 numBoneInfluenced=0;
				stream->read(&numBoneInfluenced, sizeof(ui32));
				pSubMesh->m_boneIdxs.resize( numBoneInfluenced);

				stream->read(pSubMesh->m_boneIdxs.data(), sizeof(ui32)*numBoneInfluenced);
				for (ui32 idx : pSubMesh->m_boneIdxs)
				{
					m_effectiveBones[idx] = true; 
				}
			}

			// load vertices
			ui32 vertCount;
			stream->read(&vertCount, sizeof(ui32));
			pSubMesh->m_vertInfo.m_count = vertCount;

			// 分配顶点数据存储空间
			pSubMesh->m_vertInfo.build();
			ui32 vertBuffSize = pSubMesh->m_vertInfo.m_count * pSubMesh->m_vertInfo.m_stride;
			pSubMesh->m_vertInfo.m_vertices = EchoAlloc(Byte, vertBuffSize);

			if(useHalfFloat && !supportHalfFloat)
			{
				int fileVertStride = 0;
				for(size_t k = 0; k < floatPos.size(); ++k)
				{
					if(floatPos[k] >= 0)
					{
						fileVertStride += sizeof(ui16) * floatPos[k];
					}
					else
					{
						fileVertStride -= floatPos[k];
					}
				}

				std::vector<unsigned char> FileVertData;
				FileVertData.resize(fileVertStride);

				for(ui32 l = 0; l < vertCount; ++l)
				{
					stream->read(&FileVertData[0], fileVertStride);
					unsigned char* pCurFileVertData = &FileVertData[0];
					unsigned char* pCurVertData = pSubMesh->m_vertInfo.m_vertices + pSubMesh->m_vertInfo.m_stride * l;

					for(size_t k = 0; k < floatPos.size(); ++k)
					{
						if(floatPos[k] >= 0)
						{
							for(int u = 0; u < floatPos[k]; ++u)
							{
								*(float*)pCurVertData = Math::HalfToFloat(*(ui16*)pCurFileVertData);
								pCurFileVertData += sizeof(ui16);
								pCurVertData += sizeof(float);
							}
						}
						else
						{
							memcpy(pCurVertData, pCurFileVertData, size_t(-floatPos[k]));
							pCurFileVertData -= floatPos[k];
							pCurVertData -= floatPos[k];
						}
					}
				}
			}
			else
			{
				stream->read(pSubMesh->m_vertInfo.m_vertices, vertBuffSize);
			}

			if (stream->eof())
			{
				EchoSafeDelete(pSubMesh, SubMesh);
				return false;
			}

			// load indices
			ui32 faceCount;
			stream->read(&faceCount, sizeof(ui32));
			pSubMesh->m_idxCount = faceCount * 3;
			pSubMesh->m_idxStride = sizeof(Word);

			ui32 idxBuffSize = faceCount * 3 * sizeof(Word);
			pSubMesh->m_indices = EchoAlloc(Byte,idxBuffSize);
			stream->read(pSubMesh->m_indices, idxBuffSize);

			// load aabb
			if(useHalfFloat)
			{
				ui16 iPosRot[6];
				stream->read(&iPosRot, sizeof(iPosRot));

				pSubMesh->m_box.vMin.x = Math::HalfToFloat(iPosRot[0]);
				pSubMesh->m_box.vMin.y = Math::HalfToFloat(iPosRot[1]);
				pSubMesh->m_box.vMin.z = Math::HalfToFloat(iPosRot[2]);
				pSubMesh->m_box.vMax.x = Math::HalfToFloat(iPosRot[3]);
				pSubMesh->m_box.vMax.y = Math::HalfToFloat(iPosRot[4]);
				pSubMesh->m_box.vMax.z = Math::HalfToFloat(iPosRot[5]);
			}
			else
			{
				stream->read(&pSubMesh->m_box.vMin, sizeof(Vector3));
				stream->read(&pSubMesh->m_box.vMax, sizeof(Vector3));
			}

			m_box.unionBox( pSubMesh->m_box);

			m_subMeshs.push_back( pSubMesh);
		}

		return true;
	}

	void Mesh::disableDefaultTexLoad()
	{
		m_isDisableDefaultTex = true;
	}

	// 加载Mesh
	bool Mesh::prepareImplInteral( DataStream* stream, bool isSkinned)
	{
		if(! stream)
			return false;

		m_isSkinned = isSkinned;

		i32 tVersion;
		stream->read(&tVersion, sizeof(i32));

		if(VERSION_EXTAND_MATERIAL == tVersion)
		{
			// 新版本模型格式（暂无）
			m_useSubMeshBoneMatrices = true;
//			parseFromFileVersion_materialext();
			m_isValid = true;
			return true;
		}
		else if(tVersion < VERSION_EXTAND_MATERIAL)
		{
			// 老版本，靠版本号区分HalfFloat
			bool useHalfFloat = false;
			if(tVersion >= VERSION_HALF_FLOAT_BASE)
			{
				tVersion = tVersion - VERSION_HALF_FLOAT_BASE + VERSION_ORIGIN_BASE;
				useHalfFloat = true;
			}

			if(tVersion >= 3)
			{
				m_useSubMeshBoneMatrices = true;
			}

			if(tVersion >= 2)
			{
				if (parseFromFileVersion_old(stream, useHalfFloat))
				{
					m_isValid = true;
					return true;
				}
			}
		}

		EchoLogError( "Mesh [%s] prepareImplInteral failed", mName.c_str());
		return false;
	}

	bool Mesh::prepareImpl( DataStream* stream)
	{
		return prepareImplInteral( stream, false);
	}

	void Mesh::unprepareImpl()
	{
		if(!m_bNeedBackUp)
		{
			for ( size_t i=0; i<m_subMeshs.size(); i++)
				m_subMeshs[i]->unprepareImpl();
		}
	}

	bool Mesh::loadImpl()
	{
		bool result = true;
		for ( size_t i=0; i<m_subMeshs.size(); i++)
			result &= m_subMeshs[i]->load();

		return result;
	}

	void Mesh::generateTangentData( bool useNormalMap )
	{
		size_t size = m_subMeshs.size( );
		for ( size_t i = 0; i < size; ++i )
		{
			m_subMeshs[i]->generateTangentData( useNormalMap );
		}
	}

	bool Mesh::isValid()
	{
		return m_isValid;
	}

	const array<bool,256>& Mesh::getEffectiveBones()
	{
		return m_effectiveBones;
	}

	void Mesh::rebuildVertexFomat_Remove(RenderInput::VertexSemantic semantic)
	{
		if(!m_bNeedBackUp)
			return;

		for ( size_t i=0; i<m_subMeshs.size(); i++)
			m_subMeshs[i]->dataRemove( semantic);
	}

	void Mesh::rebuildVertexFomat_Insert(const RenderInput::VertexElement& element, void* templateData)
	{
		if(!m_bNeedBackUp)
			return;

		for ( size_t i=0; i<m_subMeshs.size(); i++)
			m_subMeshs[i]->dataInsert( element, templateData);
	}

	void Mesh::unloadImpl()
	{	
		for ( size_t i=0; i<m_subMeshs.size(); i++)
		{
			m_subMeshs[i]->unloadImpl();
			EchoSafeDelete(m_subMeshs[i], SubMesh);
		}
		m_subMeshs.clear();
	}

	// 获取子模型
	SubMesh* Mesh::getSubMesh( int idx)
	{
		EchoAssert(idx>=0 && idx<static_cast<int>(m_subMeshs.size()));

		return m_subMeshs[idx];
	}

	// 获取顶点数量
	int Mesh::getVertexCount()
	{
		int result = 0;
		for (SubMesh* subMesh : m_subMeshs)
		{
			result += subMesh->getVertexCount();
		}

		return result;
	}

	void Mesh::redundancySubMeshs()
	{
		for (unsigned int i = 0; i < m_subMeshs.size(); i++)
		{
			m_subMeshs[i]->removeRedundancyVertex();
		}
	}

	void writeString(DataStream* fp, const char* str)
	{
		ui32 len = strlen(str);
		fp->write(&len, sizeof(ui32));
		fp->write(str, sizeof(char)* len);
	}

	// 保存
	bool Mesh::save( const char* filePath)
	{
		if( !m_subMeshs.size()) return false;

		FileHandleDataStream* fp = EchoNew(FileHandleDataStream(filePath, DataStream::WRITE));
		if (!fp)
			return false;

		try
		{
			// 版本号
			i32 ver = VERSION_ORIGIN_BASE + 2;//3;
// 			if(RESOURCE_USE_HALF_FLOAT)
// 			{
// 				ver = VERSION_HALF_FLOAT_BASE + 2;
// 			}

			fp->write(&ver, sizeof(i32));

			i32 subMeshNum = static_cast<i32>( m_subMeshs.size());
			fp->write(&subMeshNum, sizeof(i32));

			unsigned char reserve[72];
			memset(reserve, 0, 72);
			fp->write(reserve, 72);

			for( size_t i=0; i<m_subMeshs.size(); i++)
			{
				SubMesh* pSubMesh = m_subMeshs[i];

				// 贴图
				if( !pSubMesh->m_pDiffTex)
					throw;

				// 保存Mesh名称
				String meshName = pSubMesh->m_name;
				//GBKToUTF8( meshName, pSubMesh->m_name);
				writeString( fp, meshName.c_str());

				String mtrlName = pSubMesh->m_pDiffTex->getName();
				//GBKToUTF8(mtrlName, pSubMesh->m_pDiffTex->getName());
				writeString(fp, mtrlName.c_str());

				// 材质预留80字节
				bool bExpNormal = pSubMesh->m_vertInfo.m_isUseNormal;
				fp->write(&bExpNormal, sizeof(bool));

				bool bExpVertexColor = pSubMesh->m_vertInfo.m_isUseVertexColor;
				fp->write(&bExpVertexColor, sizeof(bool));

				bool bExpUniqueUV = pSubMesh->m_vertInfo.m_isUseLightmapUV;
				fp->write(&bExpUniqueUV, sizeof(bool));

				bool bExpTangentBinormal = pSubMesh->m_vertInfo.m_isUseTangentBinormal;
				fp->write(&bExpTangentBinormal, sizeof(bool));

				int	 lightMapResolution = pSubMesh->m_lightmapResolution;
				fp->write(&lightMapResolution, sizeof(int));

				unsigned char reserve[72];
				memset(reserve, 0, 72);
				fp->write(reserve, 72);

				// 引用骨骼数据
				if( m_isSkinned)
				{
					ui32 numBoneInfluenced=pSubMesh->m_boneIdxs.size();
					fp->write(&numBoneInfluenced, sizeof(ui32));

					fp->write(&pSubMesh->m_boneIdxs[0], sizeof(ui32)*pSubMesh->m_boneIdxs.size());
				}

				// write vertices
				ui32 vertCount = pSubMesh->m_vertInfo.m_count;
				fp->write(&vertCount, sizeof(ui32));

				ui32 vertBuffSize = vertCount * pSubMesh->m_vertInfo.m_stride;
				fp->write(pSubMesh->m_vertInfo.m_vertices, vertBuffSize);

				// write indices
				ui32 faceCount = pSubMesh->m_idxCount / 3;
				fp->write(&faceCount, sizeof(ui32));

				ui32 idxBuffSize = faceCount * 3 * sizeof(Word);
				fp->write(pSubMesh->m_indices, idxBuffSize);

				// write aabb
				if(RESOURCE_USE_HALF_FLOAT)
				{
					ui16 aabb[6];
					aabb[0] = Math::FloatToHalf(pSubMesh->m_box.vMin.x);
					aabb[1] = Math::FloatToHalf(pSubMesh->m_box.vMin.y);
					aabb[2] = Math::FloatToHalf(pSubMesh->m_box.vMin.z);
					aabb[3] = Math::FloatToHalf(pSubMesh->m_box.vMax.x);
					aabb[4] = Math::FloatToHalf(pSubMesh->m_box.vMax.y);
					aabb[5] = Math::FloatToHalf(pSubMesh->m_box.vMax.z);
					fp->write(aabb, sizeof(aabb));
				}
				else
				{
					fp->write(&pSubMesh->m_box.vMin, sizeof(Vector3));
					fp->write(&pSubMesh->m_box.vMax, sizeof(Vector3));
				}
			}
		}
		catch(...)
		{
			return false;
		}

		EchoSafeDelete(fp, FileHandleDataStream);

		return true;
	}

	bool Mesh::saveToFile( const char* filePath )
	{
		if ( m_subMeshs.empty() )
		{
			return false;
		}

		FileHandleDataStream* fp = EchoNew(FileHandleDataStream( filePath, DataStream::WRITE));
		if ( !fp )
			return false;

		try
		{
			// 版本号
			i32 ver = VERSION_ORIGIN_BASE + 2;//3;`

			fp->write( &ver, sizeof(i32) );

			i32 subMeshNum = static_cast<int>(m_subMeshs.size( ));
			fp->write( &subMeshNum, sizeof(i32));

			unsigned char reserve[72];
			memset( reserve, 0, 72 );
			fp->write( reserve, 72 );

			for ( size_t i = 0; i < m_subMeshs.size( ); i++ )
			{
				SubMesh* pSubMesh = m_subMeshs[i];

				// 保存Mesh名称
				String meshName = pSubMesh->m_name;
				writeString( fp, meshName.c_str( ) );

				String mtrlName;
				if (pSubMesh->m_pDiffTex)
				{
					mtrlName = pSubMesh->m_pDiffTex->getName();
				}
				writeString(fp, mtrlName.c_str());

				// 材质预留80字节
				bool bExpNormal = pSubMesh->m_vertInfo.m_isUseNormal;
				fp->write( &bExpNormal, sizeof(bool) );

				bool bExpVertexColor = pSubMesh->m_vertInfo.m_isUseVertexColor;
				fp->write( &bExpVertexColor, sizeof(bool) );

				bool bExpUniqueUV = pSubMesh->m_vertInfo.m_isUseLightmapUV;
				fp->write( &bExpUniqueUV, sizeof(bool) );

				bool bExpTangentBinormal = pSubMesh->m_vertInfo.m_isUseTangentBinormal;
				fp->write( &bExpTangentBinormal, sizeof(bool) );

				i32	 lightMapResolution = pSubMesh->m_lightmapResolution;
				fp->write( &lightMapResolution, sizeof(i32) );

				unsigned char reserve[72];
				memset( reserve, 0, 72 );
				fp->write( reserve, 72 );

				// 引用骨骼数据
				if ( m_isSkinned )
				{
					ui32 numBoneInfluenced = pSubMesh->m_boneIdxs.size( );
					fp->write( &numBoneInfluenced, sizeof(ui32) );

					fp->write( &pSubMesh->m_boneIdxs[0], sizeof(ui32)*pSubMesh->m_boneIdxs.size( ) );
				}

				// write vertices
				ui32 vertCount = pSubMesh->m_vertInfo.m_count;
				fp->write( &vertCount, sizeof(ui32) );

				ui32 vertBuffSize = vertCount * pSubMesh->m_vertInfo.m_stride;
				fp->write( pSubMesh->m_vertInfo.m_vertices, vertBuffSize );

				// write indices
				ui32 faceCount = pSubMesh->m_idxCount / 3;
				fp->write( &faceCount, sizeof(ui32) );

				ui32 idxBuffSize = faceCount * 3 * sizeof(Word);
				fp->write( pSubMesh->m_indices, idxBuffSize );

				// write aabb
				fp->write( &pSubMesh->m_box.vMin, sizeof(Vector3) );
				fp->write( &pSubMesh->m_box.vMax, sizeof(Vector3) );
			}
		}
		catch ( ... )
		{
			return false;
		}

		EchoSafeDelete( fp, FileHandleDataStream);

		return true;
	}

	void Mesh::reBuildVertexBuffer()
	{
		for (size_t i = 0; i < m_subMeshs.size(); i++)
			m_subMeshs[i]->reBuildVertexBuffer();
	}

	// 实例化索引。
	void InstanceMesh::InstanceIndices(SubMesh* subMesh,size_t instanceVertexCount,size_t instanceCount)
	{
		ui32 instanceIndexCount=subMesh->m_idxCount;
		ui32 instanceBytes=instanceIndexCount*subMesh->m_idxStride;
		subMesh->m_idxCount*=instanceCount;

		// 重新分配顶点数据。
		Echo::Byte* buff=subMesh->m_indices;
		subMesh->m_indices=EchoAlloc(Byte,subMesh->m_idxCount*subMesh->m_idxStride);
		memcpy(subMesh->m_indices,buff,instanceBytes);
		EchoSafeFree(buff);

		for(size_t instanceIndex=1;instanceIndex<instanceCount;++instanceIndex)
		{
			ui32 instanceBytePos=instanceIndex*instanceBytes;
			Echo::Byte* instanceDstBuff=&(subMesh->m_indices)[instanceBytePos];
			memcpy(instanceDstBuff,subMesh->m_indices,instanceBytes);

			Echo::ui16* idx=(Echo::ui16*)instanceDstBuff;
			ui32 instanceVertexPos=instanceIndex*instanceVertexCount;
			for(ui32 index=0;index<instanceIndexCount;++index)
			{
				idx[index]+=instanceVertexPos;
			}
		}
	}

	// 实例化顶点。
	void InstanceMesh::InstanceVertices( SubMesh* subMesh, const vector<Echo::Vector3>::type& instancePos, const vector<Echo::Quaternion>::type& instanceRot, const vector<Echo::Vector3>::type& instanceScale)
	{
		bool useHalfFloat = subMesh->getUseHalfFloat();

		//
		ui32 instanceVertCount = subMesh->m_vertInfo.m_count;
		ui32 instanceBytes = instanceVertCount*subMesh->m_vertInfo.m_stride;
		subMesh->m_vertInfo.m_count *= m_instanceCount;

		// 重新分配顶点数据。
		Echo::Byte* buff=subMesh->m_vertInfo.m_vertices;
		subMesh->m_vertInfo.m_vertices = EchoAlloc(Byte, instanceBytes*m_instanceCount);
		memcpy(subMesh->m_vertInfo.m_vertices, buff, instanceBytes);
		EchoSafeFree(buff);

		// 复制原始数据到实例网格。
		for( int instanceIndex=1;instanceIndex<m_instanceCount;++instanceIndex)
		{
			ui32 instanceBytePos=instanceIndex*instanceBytes;
			Echo::Byte* dstBuff = &subMesh->m_vertInfo.m_vertices[instanceBytePos];
			memcpy(dstBuff, subMesh->m_vertInfo.m_vertices, instanceBytes);
		}

		// 对实例网格的顶点进行变换。
		for( int instanceIndex=0;instanceIndex<m_instanceCount;++instanceIndex)
		{
			ui32 instanceBytePos=instanceIndex*instanceBytes;
			ui32 vertexBytePos=instanceBytePos;

			Echo::Matrix4 matrix;
			matrix.fromQuan(instanceRot[instanceIndex]);
			matrix.translate(instancePos[instanceIndex]);
			matrix.scale(instanceScale[instanceIndex]);
			for(ui32 index=0;index<instanceVertCount;++index)
			{
				// 对顶点进行变换。
				if(useHalfFloat)
				{
					ui16* pVert16 = (ui16*)&subMesh->m_vertInfo.m_vertices[vertexBytePos];
					// 坐标
					Echo::Vector3 Vert;
					Vert.x = Math::HalfToFloat(pVert16[0]);
					Vert.y = Math::HalfToFloat(pVert16[1]);
					Vert.z = Math::HalfToFloat(pVert16[2]);

					Matrix4::TransformVec3(Vert, Vert, matrix);

					pVert16[0] = Math::FloatToHalf(Vert.x);
					pVert16[1] = Math::FloatToHalf(Vert.y);
					pVert16[2] = Math::FloatToHalf(Vert.z);

					// 法线
					Echo::Vector3 normal;
					normal.x = Math::HalfToFloat(pVert16[3]);
					normal.y = Math::HalfToFloat(pVert16[4]);
					normal.z = Math::HalfToFloat(pVert16[5]);

					normal = matrix.rotateVec3(normal);

					pVert16[3] = Math::FloatToHalf(normal.x);
					pVert16[4] = Math::FloatToHalf(normal.y);
					pVert16[5] = Math::FloatToHalf(normal.z);
				}
				else
				{
					Echo::Vector3* pVert = (Echo::Vector3*)&subMesh->m_vertInfo.m_vertices[vertexBytePos];
					Matrix4::TransformVec3(*pVert,*pVert,matrix);
					*(pVert + 1) = matrix.rotateVec3(*(pVert + 1));
				}

				// 移动到下一个位置。
				vertexBytePos += subMesh->m_vertInfo.m_stride;
			}
		}

		// 重新计算灯光图的纹理坐标。
		if (subMesh->m_vertInfo.m_isUseLightmapUV)
		{
			ui32 lightmapPow=0;
			while(lightmapPow*lightmapPow< static_cast<ui32>(m_instanceCount))
				++lightmapPow;

			float lightmapRate=1/float(lightmapPow);
			ui32 stride = subMesh->getVertexInfo().getVertexStride();

			ui32 lightmapByteOffset;
			if(useHalfFloat)
			{
				lightmapByteOffset = stride - PixelUtil::GetPixelSize( PF_RG16_FLOAT);
			}
			else
			{
				lightmapByteOffset = stride - PixelUtil::GetPixelSize( PF_RG32_FLOAT);
			} 

			for( int instanceIndex=0;instanceIndex<m_instanceCount;++instanceIndex)
			{
				ui32 instanceLightmapPosX=instanceIndex%lightmapPow;
				ui32 instanceLightmapPosY=instanceIndex/lightmapPow;
				ui32 instanceBytePos=instanceIndex*instanceBytes;
				ui32 lightmapVertexBytePos=instanceBytePos+lightmapByteOffset;

				for(ui32 index=0;index<instanceVertCount;++index)
				{
					if(useHalfFloat)
					{
						ui16* lightmapVertex = (ui16*)&subMesh->m_vertInfo.m_vertices[lightmapVertexBytePos];

						float x = Math::HalfToFloat(lightmapVertex[0]);
						float y = Math::HalfToFloat(lightmapVertex[1]);
						x = (x + float(instanceLightmapPosX))*lightmapRate;
						y = (y + float(instanceLightmapPosY))*lightmapRate;

						lightmapVertex[0] = Math::FloatToHalf(x);
						lightmapVertex[1] = Math::FloatToHalf(y);
					}
					else
					{
						Echo::Vector2* lightmapVertex=(Echo::Vector2*)&subMesh->m_vertInfo.m_vertices[lightmapVertexBytePos];

						lightmapVertex->x=(lightmapVertex->x+float(instanceLightmapPosX))*lightmapRate;
						lightmapVertex->y=(lightmapVertex->y+float(instanceLightmapPosY))*lightmapRate;
					}

					// 移动到下一个位置。
					lightmapVertexBytePos += subMesh->m_vertInfo.m_stride;
				}
			}
		}
	}

	// 实例化不同submesh的顶点数据
	void InstanceMesh::InstanceSubMeshVertices(const vector<Echo::Vector3>::type& instancePos,
		const vector<Echo::Quaternion>::type& instanceRot, const vector<Echo::Vector3>::type& instanceScale)
	{
		bool useHalfFloat = false;

		int buffSize = 0;
		int count = 0;
		int first_submesh_count = m_subMeshs[0]->m_vertInfo.m_count;
		int first_submesh_stride = m_subMeshs[0]->m_vertInfo.m_stride;
		for (int i = 0; i < m_subMeshCount; i++)
		{
			SubMesh* subMesh = m_subMeshs[i];
			ui32 instanceVertCount = subMesh->m_vertInfo.m_count;
			if (first_submesh_stride != subMesh->m_vertInfo.m_stride)
			{
				EchoAssertX(false, "stride not same!");
			}

			ui32 instanceBytes = instanceVertCount*subMesh->m_vertInfo.m_stride;
			buffSize += instanceBytes;
			count += instanceVertCount;
		}
		
		m_subMeshs[0]->m_vertInfo.m_count = count;

		// 重新分配顶点数据。
		Echo::Byte* buff = m_subMeshs[0]->m_vertInfo.m_vertices;
		m_subMeshs[0]->m_vertInfo.m_vertices = EchoAlloc(Byte, buffSize);
		memcpy(m_subMeshs[0]->m_vertInfo.m_vertices, buff, first_submesh_count*first_submesh_stride);
		EchoSafeFree(buff);

		int pos = first_submesh_count*m_subMeshs[0]->m_vertInfo.m_stride;
		// 复制原始数据到实例网格。
		for (int instanceIndex = 1; instanceIndex < m_instanceCount; ++instanceIndex)
		{
			ui32 instanceBytePos = pos;
			int instanceBytes = m_subMeshs[instanceIndex]->m_vertInfo.m_count*m_subMeshs[instanceIndex]->m_vertInfo.m_stride;
			Echo::Byte* dstBuff = &m_subMeshs[0]->m_vertInfo.m_vertices[instanceBytePos];
			memcpy(dstBuff, m_subMeshs[instanceIndex]->m_vertInfo.m_vertices, instanceBytes);
			pos += instanceBytes;
		}

		pos = 0;
		// 对实例网格的顶点进行变换。
		for (int instanceIndex = 0; instanceIndex < m_instanceCount; ++instanceIndex)
		{
			ui32 instanceBytePos = pos;
			if (instanceIndex == 0)
				pos = first_submesh_count*m_subMeshs[0]->m_vertInfo.m_stride;
			else
				pos += m_subMeshs[instanceIndex]->m_vertInfo.m_count*m_subMeshs[instanceIndex]->m_vertInfo.m_stride;
			ui32 vertexBytePos = instanceBytePos;

			Echo::Matrix4 matrix;
			matrix.fromQuan(instanceRot[instanceIndex]);
			matrix.translate(instancePos[instanceIndex]);
			matrix.scale(instanceScale[instanceIndex]);

			ui32 instanceVertCount; 
			if (instanceIndex == 0)
				instanceVertCount = first_submesh_count;
			else
				instanceVertCount = m_subMeshs[instanceIndex]->m_vertInfo.m_count;
			for (ui32 index = 0; index < instanceVertCount; ++index)
			{
				// 对顶点进行变换。
				
				if (useHalfFloat)
				{
					ui16* pVert16 = (ui16*)&m_subMeshs[0]->m_vertInfo.m_vertices[vertexBytePos];
					// 坐标
					Echo::Vector3 Vert;
					Vert.x = Math::HalfToFloat(pVert16[0]);
					Vert.y = Math::HalfToFloat(pVert16[1]);
					Vert.z = Math::HalfToFloat(pVert16[2]);

					Matrix4::TransformVec3(Vert, Vert, matrix);

					pVert16[0] = Math::FloatToHalf(Vert.x);
					pVert16[1] = Math::FloatToHalf(Vert.y);
					pVert16[2] = Math::FloatToHalf(Vert.z);

					// 法线
					Echo::Vector3 normal;
					normal.x = Math::HalfToFloat(pVert16[3]);
					normal.y = Math::HalfToFloat(pVert16[4]);
					normal.z = Math::HalfToFloat(pVert16[5]);

					normal = matrix.rotateVec3(normal);

					pVert16[3] = Math::FloatToHalf(normal.x);
					pVert16[4] = Math::FloatToHalf(normal.y);
					pVert16[5] = Math::FloatToHalf(normal.z);
				}
				else
				{
					Echo::Vector3* pVert = (Echo::Vector3*)&m_subMeshs[0]->m_vertInfo.m_vertices[vertexBytePos];
					Matrix4::TransformVec3(*pVert, *pVert, matrix);
					*(pVert + 1) = matrix.rotateVec3(*(pVert + 1));
				}

				// 移动到下一个位置。
				vertexBytePos += m_subMeshs[0]->m_vertInfo.m_stride;
			}
		}

		// 重新计算灯光图的纹理坐标。
		if (m_subMeshs[0]->m_vertInfo.m_isUseLightmapUV)
		{
			ui32 lightmapPow = 0;
			while (lightmapPow*lightmapPow < static_cast<ui32>(m_instanceCount))
				++lightmapPow;

			float lightmapRate = 1 / float(lightmapPow);
			ui32 stride = m_subMeshs[0]->getVertexInfo().getVertexStride();

			ui32 lightmapByteOffset;
			if (useHalfFloat)
			{
				lightmapByteOffset = stride - PixelUtil::GetPixelSize(PF_RG16_FLOAT);
			}
			else
			{
				lightmapByteOffset = stride - PixelUtil::GetPixelSize(PF_RG32_FLOAT);
			}

			pos = 0;
			for (int instanceIndex = 0; instanceIndex < m_instanceCount; ++instanceIndex)
			{
				ui32 instanceLightmapPosX = instanceIndex%lightmapPow;
				ui32 instanceLightmapPosY = instanceIndex / lightmapPow;
				ui32 instanceBytePos = pos;
				if (instanceIndex == 0)
					pos = first_submesh_count*m_subMeshs[0]->m_vertInfo.m_stride;
				else
					pos += m_subMeshs[instanceIndex]->m_vertInfo.m_count*m_subMeshs[instanceIndex]->m_vertInfo.m_stride;
				
				ui32 lightmapVertexBytePos = instanceBytePos + lightmapByteOffset;

				ui32 instanceVertCount;
				if (instanceIndex == 0)
					instanceVertCount = first_submesh_count;
				else
					instanceVertCount = m_subMeshs[instanceIndex]->m_vertInfo.m_count;

				for (ui32 index = 0; index < instanceVertCount; ++index)
				{
					if (useHalfFloat)
					{
						ui16* lightmapVertex = (ui16*)&m_subMeshs[0]->m_vertInfo.m_vertices[lightmapVertexBytePos];

						float x = Math::HalfToFloat(lightmapVertex[0]);
						float y = Math::HalfToFloat(lightmapVertex[1]);
						x = (x + float(instanceLightmapPosX))*lightmapRate;
						y = (y + float(instanceLightmapPosY))*lightmapRate;

						lightmapVertex[0] = Math::FloatToHalf(x);
						lightmapVertex[1] = Math::FloatToHalf(y);
					}
					else
					{
						Echo::Vector2* lightmapVertex = (Echo::Vector2*)&m_subMeshs[0]->m_vertInfo.m_vertices[lightmapVertexBytePos];

						lightmapVertex->x = (lightmapVertex->x + float(instanceLightmapPosX))*lightmapRate;
						lightmapVertex->y = (lightmapVertex->y + float(instanceLightmapPosY))*lightmapRate;
					}

					// 移动到下一个位置。
					lightmapVertexBytePos += m_subMeshs[0]->m_vertInfo.m_stride;
				}
			}
		}
	}

	// 实例化不同submesh的索引数据
	void InstanceMesh::InstanceSubMeshIndices(size_t firstSubMeshVertexCount, size_t instanceCount)
	{
		int count = 0;
		int buffSize = 0;
		int first_submesh_idxcount = m_subMeshs[0]->m_idxCount;
		for (ui32 i = 0; i < instanceCount; i++)
		{
			ui32 instanceIndexCount = m_subMeshs[i]->m_idxCount;
			ui32 instanceBytes = instanceIndexCount*m_subMeshs[i]->m_idxStride;
			count += instanceIndexCount;
			buffSize += instanceBytes;
		}
		
		m_subMeshs[0]->m_idxCount = count;

		// 重新分配顶点数据。
		Echo::Byte* buff = m_subMeshs[0]->m_indices;
		m_subMeshs[0]->m_indices = EchoAlloc(Byte, count*m_subMeshs[0]->m_idxStride);
		memcpy(m_subMeshs[0]->m_indices, buff, first_submesh_idxcount*m_subMeshs[0]->m_idxStride);
		EchoSafeFree(buff);

		int pos = first_submesh_idxcount*m_subMeshs[0]->m_idxStride;
		int last_count = firstSubMeshVertexCount; 
		ui32 idx_count = first_submesh_idxcount;
		for (size_t instanceIndex = 1; instanceIndex < instanceCount; ++instanceIndex)
		{
			ui32 instanceBytePos = pos;
			int instanceBytes = m_subMeshs[instanceIndex]->m_idxCount*m_subMeshs[instanceIndex]->m_idxStride;
			pos += instanceBytes;
			Echo::Byte* instanceDstBuff = &(m_subMeshs[0]->m_indices)[instanceBytePos];
			memcpy(instanceDstBuff, m_subMeshs[instanceIndex]->m_indices, instanceBytes);

			Echo::ui16* idx = (Echo::ui16*)instanceDstBuff;
			ui32 instanceVertexPos = last_count;

			for (ui32 index = 0; index < m_subMeshs[instanceIndex]->m_idxCount; ++index)
			{
				idx[index] += instanceVertexPos;
			}
			idx_count += m_subMeshs[instanceIndex]->m_idxCount;
			last_count += m_subMeshs[instanceIndex]->getVertexInfo().m_count;
		}
	}

	bool InstanceMesh::parseFromFileOneSumesh(DataStream* stream, bool useHalfFloat)
	{
		if (!stream)
			return false;

		m_isSkinned = false;

		i32 tVersion;
		stream->read(&tVersion, sizeof(i32));
		useHalfFloat = false;
		// 老版本，靠版本号区分HalfFloat
		if (tVersion >= VERSION_HALF_FLOAT_BASE)
		{
			tVersion = tVersion - VERSION_HALF_FLOAT_BASE + VERSION_ORIGIN_BASE;
			useHalfFloat = true;
		}

		if (tVersion >= 3)
		{
			m_useSubMeshBoneMatrices = true;
		}

		if (tVersion >= 2)
		{
			//m_box.reset();

			// 子模型数量
			i32 subMeshNum = 0;
			stream->read(&subMeshNum, sizeof(i32));

			// 预留空间
			stream->skip(72);


			SubMesh* pSubMesh = EchoNew(SubMesh);
			EchoAssert(pSubMesh);

			// name
			ui32 smNameLen;
			char smName[260];
			stream->read(&smNameLen, sizeof(ui32));
			stream->read(&smName, smNameLen);
			smName[smNameLen] = 0;
			pSubMesh->m_name = smName;

			// material
			ui32 mtrlLen;
			char mtrlPath[260];
			stream->read(&mtrlLen, sizeof(ui32));
			stream->read(&mtrlPath, mtrlLen);
			mtrlPath[mtrlLen] = 0;
			String diffTexFile = mtrlPath;

			//to lower case.
			StringUtil::LowerCase(diffTexFile);

			pSubMesh->m_pDiffTex = TextureResManager::instance()->createTexture(diffTexFile, Texture::TU_STATIC);
			pSubMesh->m_pDiffTex->prepare();

			// skip 80 bytes
			i32	 lightmapResolution;
			stream->read(&pSubMesh->m_vertInfo.m_isUseNormal, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseVertexColor, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseLightmapUV, sizeof(bool));
			stream->read(&pSubMesh->m_vertInfo.m_isUseTangentBinormal, sizeof(bool));
			stream->read(&lightmapResolution, sizeof(i32));
			stream->skip(72);

			pSubMesh->m_vertInfo.m_isUseBoneData = m_isSkinned;
			pSubMesh->m_lightmapResolution = Echo::Math::Clamp(lightmapResolution, 8, 512);

			std::vector<int> floatPos;
			bool supportHalfFloat = false;
			pSubMesh->setVertexStructInfo(m_isSkinned, useHalfFloat && supportHalfFloat, floatPos);

			// load 骨骼索引数据
			if (m_isSkinned && m_useSubMeshBoneMatrices)
			{
				ui32 numBoneInfluenced = 0;
				stream->read(&numBoneInfluenced, sizeof(ui32));
				pSubMesh->m_boneIdxs.resize(numBoneInfluenced);

				stream->read(pSubMesh->m_boneIdxs.data(), sizeof(ui32)*numBoneInfluenced);
			}

			// load vertices
			ui32 vertCount;
			stream->read(&vertCount, sizeof(ui32));
			pSubMesh->m_vertInfo.m_count = vertCount;

			// 分配顶点数据存储空间
			pSubMesh->m_vertInfo.build();
			ui32 vertBuffSize = pSubMesh->m_vertInfo.m_count * pSubMesh->m_vertInfo.m_stride;
			pSubMesh->m_vertInfo.m_vertices = EchoAlloc(Byte, vertBuffSize);

			if (useHalfFloat && !supportHalfFloat)
			{
				int fileVertStride = 0;
				for (size_t k = 0; k < floatPos.size(); ++k)
				{
					if (floatPos[k] >= 0)
					{
						fileVertStride += sizeof(ui16) * floatPos[k];
					}
					else
					{
						fileVertStride -= floatPos[k];
					}
				}

				std::vector<unsigned char> FileVertData;
				FileVertData.resize(fileVertStride);

				for (ui32 l = 0; l < vertCount; ++l)
				{
					stream->read(&FileVertData[0], fileVertStride);
					unsigned char* pCurFileVertData = &FileVertData[0];
					unsigned char* pCurVertData = pSubMesh->m_vertInfo.m_vertices + pSubMesh->m_vertInfo.m_stride * l;

					for (size_t k = 0; k < floatPos.size(); ++k)
					{
						if (floatPos[k] >= 0)
						{
							for (int u = 0; u < floatPos[k]; ++u)
							{
								*(float*)pCurVertData = Math::HalfToFloat(*(ui16*)pCurFileVertData);
								pCurFileVertData += sizeof(ui16);
								pCurVertData += sizeof(float);
							}
						}
						else
						{
							memcpy(pCurVertData, pCurFileVertData, size_t(-floatPos[k]));
							pCurFileVertData -= floatPos[k];
							pCurVertData -= floatPos[k];
						}
					}
				}
			}
			else
			{
				stream->read(pSubMesh->m_vertInfo.m_vertices, vertBuffSize);
			}

			if (stream->eof())
			{
				EchoSafeDelete(pSubMesh, SubMesh);
				return false;
			}

			// load indices
			ui32 faceCount;
			stream->read(&faceCount, sizeof(ui32));
			pSubMesh->m_idxCount = faceCount * 3;
			pSubMesh->m_idxStride = sizeof(Word);

			ui32 idxBuffSize = faceCount * 3 * sizeof(Word);
			pSubMesh->m_indices = EchoAlloc(Byte, idxBuffSize);
			stream->read(pSubMesh->m_indices, idxBuffSize);

			// load aabb
			if (useHalfFloat)
			{
				ui16 iPosRot[6];
				stream->read(&iPosRot, sizeof(iPosRot));

				pSubMesh->m_box.vMin.x = Math::HalfToFloat(iPosRot[0]);
				pSubMesh->m_box.vMin.y = Math::HalfToFloat(iPosRot[1]);
				pSubMesh->m_box.vMin.z = Math::HalfToFloat(iPosRot[2]);
				pSubMesh->m_box.vMax.x = Math::HalfToFloat(iPosRot[3]);
				pSubMesh->m_box.vMax.y = Math::HalfToFloat(iPosRot[4]);
				pSubMesh->m_box.vMax.z = Math::HalfToFloat(iPosRot[5]);
			}
			else
			{
				stream->read(&pSubMesh->m_box.vMin, sizeof(Vector3));
				stream->read(&pSubMesh->m_box.vMax, sizeof(Vector3));
			}

			m_box.unionBox(pSubMesh->m_box);

			m_subMeshs.push_back(pSubMesh);
			
		}

		m_isValid = true;
		return true;
	}

	bool InstanceMesh::prepareImplInteral(DataStream* stream, bool isSkinned)
	{
		if (!stream)
			return false;

		char fileType[5];
		stream->read(fileType, 4);
		fileType[4]=0;

		if(strcmp(fileType,"IMSH"))
			return false;
		ui32 version;
		stream->read(&version, 4);

		if( version==101)
		{

			// 读入subMesh数量
			stream->read(&m_subMeshCount, sizeof(i32));
			// 读入所有实例的矩阵。
			stream->read(&m_instanceCount, sizeof(i32));
			// 读入为同模型还是同材质
			int isSameModel;
			stream->read(&isSameModel, sizeof(i32));
			m_isSameModel = !!isSameModel;

			if (m_isSameModel)
			{
				// 读网格名字。
				ui32 meshNameLength;
				stream->read(&meshNameLength, sizeof(meshNameLength));
				char* meshFileName = ECHO_ALLOC_T(char, meshNameLength + 1);
				stream->read(meshFileName, meshNameLength);
				meshFileName[meshNameLength] = 0;
				m_templateName = meshFileName;

				//读入网格数据。
				Echo::String instanceMeshName = mName;
				mName = meshFileName;
				ECHO_FREE(meshFileName);
				meshFileName = NULL;
				DataStream* meshStream = ResourceGroupManager::instance()->openResource(mName);
				if (!Mesh::prepareImplInteral(meshStream, isSkinned))
				{
					return false;
				}
				EchoSafeDelete(meshStream, DataStream);
				mName = instanceMeshName;

				m_box.reset();

				//实例化。
				for (int i = 0; i < this->getSubMeshNum(); i++)
				{
					SubMesh* subMesh = getSubMesh(i);

					// 读包围盒。
					Box box;
					stream->read(&box.vMax, sizeof(box.vMax));
					stream->read(&box.vMin, sizeof(box.vMin));
					stream->read(&box.mExtent, sizeof(box.mExtent));

					// 读取实例转换(平移,旋转,缩放)
					for (int i = 0; i < m_instanceCount; i++)
					{
						Echo::Vector3 pos;
						stream->read(&pos.m, sizeof(Real) * 3);
						m_instancePos.push_back(pos);
						Echo::Quaternion rot;
						stream->read(&rot.m, sizeof(Real) * 4);
						m_instanceRot.push_back(rot);
						Echo::Vector3 scl;
						stream->read(&scl.m, sizeof(Real) * 3);
						m_instanceScale.push_back(scl);
					}

					ui32 instanceVertexCount = subMesh->m_vertInfo.m_count;
					InstanceVertices(subMesh, m_instancePos, m_instanceRot, m_instanceScale);
					InstanceIndices(subMesh, instanceVertexCount, m_instanceCount);

					subMesh->m_box = box;
					m_box.unionBox(subMesh->m_box);
				}

			}
			else
			{
				//读入网格数据。
				for (int i = 0; i < m_instanceCount; i++)
				{
					ui32 meshNameLength;
					stream->read(&meshNameLength, sizeof(meshNameLength));
					char* meshFileName = ECHO_ALLOC_T(char, meshNameLength + 1);
					stream->read(meshFileName, meshNameLength);
					meshFileName[meshNameLength] = 0;
					//m_templateName = meshFileName;

					DataStream* meshStream = ResourceGroupManager::instance()->openResource(meshFileName);
					if (!parseFromFileOneSumesh(meshStream, isSkinned))
					{
						return false;
					}
					ECHO_FREE(meshFileName);
					meshFileName = NULL;
					EchoSafeDelete(meshStream, DataStream);
				}
				m_box.reset();

				//SubMesh* subMesh = getSubMesh(0);

				// 读包围盒。
				Box box;
				stream->read(&box.vMax, sizeof(box.vMax));
				stream->read(&box.vMin, sizeof(box.vMin));
				stream->read(&box.mExtent, sizeof(box.mExtent));

				// 读取实例转换(平移,旋转,缩放)
				for (int i = 0; i < m_instanceCount; i++)
				{
					Echo::Vector3 pos;
					stream->read(&pos.m, sizeof(Real) * 3);
					m_instancePos.push_back(pos);
					Echo::Quaternion rot;
					stream->read(&rot.m, sizeof(Real) * 4);
					m_instanceRot.push_back(rot);
					Echo::Vector3 scl;
					stream->read(&scl.m, sizeof(Real) * 3);
					m_instanceScale.push_back(scl);
				}

				m_subMeshCount = m_instanceCount;
				ui32 firstSubMeshVertexCount = m_subMeshs[0]->m_vertInfo.m_count;
				InstanceSubMeshVertices(m_instancePos, m_instanceRot, m_instanceScale);
				InstanceSubMeshIndices(firstSubMeshVertexCount, m_instanceCount);

				m_subMeshs[0]->m_box = box;
				m_box.unionBox(box);

				for (size_t i = 1; i < m_subMeshs.size(); i++)
					m_subMeshs[i]->unprepareImpl();
				size_t size = m_subMeshs.size();
				for (size_t i = 1; i < size; i++)
					m_subMeshs.pop_back();
				
				int i = 3;
			}
			
			
		}
		else
			return false;

		return true;
	}

	void InstanceMesh::unloadImpl()
	{
		Mesh::unloadImpl();
		m_instancePos.clear();
		m_instanceRot.clear();
		m_instanceScale.clear();
	}
}