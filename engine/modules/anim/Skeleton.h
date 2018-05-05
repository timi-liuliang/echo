#pragma once

#include "engine/core/Resource/Resource.h"
#include "engine/core/math/MathLib.h"

namespace Echo
{
	class Bone;
	class Skeleton : public Resource
	{
	public:
		friend class SkeletonManager;
		typedef vector<Bone*>::type			BoneList;
		typedef map<String, Bone*>::type	BoneListByName;
	public:
		Skeleton(const String& name, bool isManual);
		virtual ~Skeleton();

		// 新建骨骼
		virtual Bone* createBone(int parentId, const String& name, const Quaternion& rot, const Vector4& pos, bool bHelper = false);

		// 根据骨骼ID获取骨骼
		virtual Bone* getBoneById(int boneId) const;

		// 根据骨骼名称获取骨骼
		virtual Bone* getBone(const String& boneName);

		// 获取骨骼数量
		virtual ui32 getBoneCount() const;

		// 获取挂点数量
		virtual ui32 getDummyCount() const;

		// 获取骨骼总量(真实骨骼结点+挂点)
		virtual ui32 getTotalCount() const;

		// 获取骨骼列表
		virtual const BoneList&	getBones() const;

		// 获取主骨骼
		Bone* getRootBone();

		// 获取所有子节点(包含自身)
		bool getAllChildren(BoneList& oBoneList, Bone* parentBone);

		bool supportScale() const;

	protected:
		virtual size_t calculateSize() const;
		virtual bool prepareImpl(DataStream* stream);
		virtual void unprepareImpl();
		virtual bool loadImpl();
		virtual void unloadImpl();

	protected:
		BoneList		m_bones;			// 所包含骨骼
		BoneListByName	m_bonesByName;		// 骨骼名称
		ui32			m_boneCount;		// 骨骼数量 without dummy
		ui32			m_dummyCount;		// 挂点数量
		Bone*			m_pRootBone;		// 主骨骼
		bool			m_supportScale;
	};

	/**
	* SkeletonPose
	*/
	struct SkeletonPose
	{
		vector<Quaternion>::type		m_rotations;		// 旋转
		vector<Vector4>::type			m_translations;		// 位移a
		map<int, Quaternion>::type		m_lookAtRotations;	// 朝向固定点旋转

		// 构造函数
		SkeletonPose(Skeleton* skeleton);

		// 重置
		void reset(Skeleton* skeleton);
	};
}