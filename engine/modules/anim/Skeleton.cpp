#include "Animation.h"
#include "Engine/core/Scene/Scene_Manager.h"
#include "Bone.h"
#include "Skeleton.h"

namespace Echo
{
	#pragma pack(push, 1)
	// 骨骼数量
	struct BoneData
	{
		i32		id;
		i32		parentId;
		bool	bHelper;
		char	name[32];
		float	matLocal[16];
	};
	#pragma pack(pop)

	// 构造函数
	Skeleton::Skeleton(const String& name, bool isManual)
		:Resource(name, RT_Skeleton, isManual)
		,m_boneCount(0)
		,m_dummyCount(0)
		,m_pRootBone(NULL)
		, m_supportScale(false)
	{
	}

	// 析构函数
	Skeleton::~Skeleton()
	{
		unloadImpl();
	}

	// 计算内存占用
	size_t Skeleton::calculateSize() const
	{
		// not implement
		return 0;
	}

	bool Skeleton::prepareImpl(DataStream* stream)
	{
		if (stream == NULL)
			return false;

		i32 version;
		stream->read(&version, sizeof(i32));

		bool useHalfFloat = false;
		if (version >= VERSION_HALF_FLOAT_BASE)
		{
			useHalfFloat = true;
		}

		if (!useHalfFloat && version >= VERSION_SKELETON_SUPPORT_SCALE)
		{
			m_supportScale = true;
		}

		// load each bone
		String boneName;

		ui32 boneNameLen;
		char szBoneName[260] = {};
		i32 parentId;
		float position[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float rotation[4];
		Vector4 vPos;
		Quaternion qRot;

		// bone count
		i32 boneCount;
		stream->read(&boneCount, sizeof(i32));

#ifdef ECHO_PLATFORM_ANDROID
		if (boneCount < 0 || boneCount > 1024)
		{
			EchoLogError("[Skeleton::prepareImpl:%d]::Load Skeleton[%s], bone count == %d", __LINE__, stream->getName().c_str(), boneCount); 
			return false; 
		}
#endif

		for (int i = 0; i < boneCount; ++i)
		{
			stream->read(&boneNameLen, sizeof(ui32));
			stream->read(&szBoneName, std::min<ui32>(boneNameLen, 260));
			szBoneName[boneNameLen] = 0;
			boneName = szBoneName;

			stream->read(&parentId, sizeof(i32));
			if (useHalfFloat)
			{
				ui16 iPosRot[7];
				stream->read(&iPosRot, sizeof(iPosRot));
				for (int k = 0; k < 3; ++k)
				{
					position[k] = Math::HalfToFloat(iPosRot[k]);
				}
				vPos.set(position[0], position[1], position[2], 1.0f);

				for (int k = 0; k < 4; ++k)
				{
					rotation[k] = Math::HalfToFloat(iPosRot[k + 3]);
				}
				qRot.set(rotation[0], rotation[1], rotation[2], rotation[3]);
			}
			else
			{
				if (m_supportScale)
				{
					stream->read(&position, sizeof(float)* 4);
				}
				else
				{
					stream->read(&position, sizeof(float)* 3);
				}
				vPos.set(position[0], position[1], position[2], position[3]);
				stream->read(&rotation, sizeof(rotation));
				qRot.set(rotation[0], rotation[1], rotation[2], rotation[3]);
			}
			createBone(parentId, boneName, qRot, vPos, false);
		}

		// dummy count
		i32 dummyCount;
		stream->read(&dummyCount, sizeof(i32));
		for (int i = 0; i < dummyCount; ++i)
		{
			stream->read(&boneNameLen, sizeof(ui32));
			stream->read(&szBoneName, boneNameLen);
			szBoneName[boneNameLen] = 0;
			boneName = szBoneName;
			stream->read(&parentId, sizeof(i32));

			if (useHalfFloat)
			{
				ui16 iPosRot[7];
				stream->read(&iPosRot, sizeof(iPosRot));
				for (int k = 0; k < 3; ++k)
				{
					position[k] = Math::HalfToFloat(iPosRot[k]);
				}
				vPos.set(position[0], position[1], position[2], 1.0f);

				for (int k = 0; k < 4; ++k)
				{
					rotation[k] = Math::HalfToFloat(iPosRot[k + 3]);
				}
				qRot.set(rotation[0], rotation[1], rotation[2], rotation[3]);
			}
			else
			{
				if (m_supportScale)
				{
					stream->read(&position, sizeof(float)* 4);
				}
				else
				{
					stream->read(&position, sizeof(float)* 3);
				}
				vPos.set(position[0], position[1], position[2], position[3]);
				stream->read(&rotation, sizeof(rotation));
				qRot.set(rotation[0], rotation[1], rotation[2], rotation[3]);
			}
			createBone(parentId, boneName, qRot, vPos, true);
		}

		// orgnize bones hierarchy
		for (int i = 0; i < (boneCount + dummyCount); ++i)
		{
			Bone* pBone = m_bones[i];

			if (pBone->getParentId() != -1)
			{
				Bone* pBoneParent = m_bones[pBone->getParentId()];
				pBone->setParent(pBoneParent);
				pBoneParent->addChildren(pBone);
			}
		}

		return true;
	}

	void Skeleton::unprepareImpl()
	{
	}

	bool Skeleton::loadImpl()
	{
		return true;
	}

	void Skeleton::unloadImpl()
	{
		EchoSafeDeleteContainer(m_bones, Bone);

		m_pRootBone = NULL;
	}

	// 创建骨骼
	Bone* Skeleton::createBone(int parentId, const String& name, const Quaternion& rot, const Vector4& pos, bool bHelper)
	{
		Bone* pBone = NULL;
		try
		{
			int boneId = (int)m_bones.size();
			pBone = EchoNew(Bone(this, boneId, parentId, name, rot, pos, bHelper));
			m_bones.push_back(pBone);
			m_bonesByName.insert(std::make_pair(name, pBone));

			if(bHelper)
				++m_dummyCount;
			else
				++m_boneCount;

			if(!m_pRootBone)
				m_pRootBone = pBone;
		}
		catch (Exception e)
		{
			EchoLogError(e.getMessage().c_str());
		}

		return pBone;
	}

	// 根据ID获取骨骼
	Bone* Skeleton::getBoneById(int boneId) const
	{
		if(boneId < 0 || boneId >= (int)m_bones.size())
			return NULL;

		return m_bones[boneId];
	}

	Bone* Skeleton::getBone(const String& boneName)
	{
		BoneListByName::iterator it = m_bonesByName.find(boneName);
		return (it!=m_bonesByName.end()) ? it->second : NULL;
	}

	ui32 Skeleton::getBoneCount() const
	{
		return m_boneCount;
	}

	ui32 Skeleton::getDummyCount() const
	{
		return m_dummyCount;
	}

	ui32 Skeleton::getTotalCount() const
	{
		return (ui32)m_bones.size();
	}

	const Skeleton::BoneList& Skeleton::getBones() const
	{
		return m_bones;
	}

	// 获取主骨骼
	Bone* Skeleton::getRootBone() 
	{  
		if( m_pRootBone)
		{
			EchoAssert(m_pRootBone==m_bones[0]); 
			return m_pRootBone; 
		}
		
		return NULL;
	}

	// 获取所有子节点(包含自身)
	bool Skeleton::getAllChildren( BoneList& oBoneList, Bone* parentBone)
	{
		if ( parentBone->isDummy() )
		{
			return false;
		}
		oBoneList.push_back( parentBone);
		const BoneList&	children = parentBone->getChildren();
		for( size_t i=0; i<children.size(); i++)
		{
			getAllChildren( oBoneList, children[i]);
		}

		return true;
	}

	bool Skeleton::supportScale() const
	{
		return m_supportScale;
	}

	// 构造函数
	SkeletonPose::SkeletonPose( Skeleton* skeleton)
	{
		reset( skeleton);
	}

	// 重置
	void SkeletonPose::reset( Skeleton* skeleton)
	{
		m_rotations.resize( skeleton->getBoneCount(), Quaternion::IDENTITY);
		m_translations.resize( skeleton->getBoneCount());

		for( size_t i=0; i<skeleton->getBoneCount(); i++)
		{
			m_translations[i] = Vector4(skeleton->getBoneById(i)->getPosition(), 1.0f);
		}
	}
}
