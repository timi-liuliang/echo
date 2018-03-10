#pragma once

#include "engine/core/Resource/Resource.h"

namespace Echo
{
	class AnimManager;
	class AnimKeyFrame;
	class Bone;
	class Animation: public Resource
	{
		typedef vector<AnimKeyFrame*>::type	AnimKeyFrameList;
	public:
		friend class AnimManager;

		// 动画类型
		enum AnimType
		{
			AT_UNKNOWN,
			AT_SKELETON,
			AT_KEYFRAME,
		};

	public:
		// 创建
		static Animation* create(const String& name, bool isManual=false);

		// 释放
		void release();

		virtual AnimKeyFrame* createAnimKeyFrame(ui32 time);
		virtual void removeAnimKeyFrameByIdx(ui32 idx);
		virtual void removeAnimKeyFrames();

		virtual void setAnimType(AnimType type);

		// 获取动画类型
		virtual AnimType getAnimType() const;
		virtual ui32 getFrameCount() const;
		virtual ui32 getLength() const;
		virtual ui32 getFPS() const;
		virtual bool hasScaling() const;
		virtual AnimKeyFrame* getAnimKeyFrame(ui32 time) const;
		virtual AnimKeyFrame* getAnimKeyFrameByIdx(ui32 idx) const;
		virtual const AnimKeyFrameList&	getAnimKeyFrameList() const;
		virtual ui32 getFrameIndex(ui32 time, i32 deltaTime) const;

		// 设置骨骼权重
		void setBoneWeight( const Bone* bone, float weight);

		// 获取对指定骨骼影响权重
		float getBoneWeight( const Bone* bone);

		// 获取骨骼权重
		vector<float>::type& getBoneWeights() { return m_boneWeights; }

		// 是否是局部动画
		bool isPartition() const { return m_isPartition; }

		// 若是部分动画，返回部分动画主骨骼
		int getPartionRootBone() const { return m_partitionRootBone; }

		// 获取骨骼数量
		ui32 getBoneCount() const { return m_boneCount;}

		// 保存
		void save( const char* filePath);

		void optAnimation();

	protected:
		Animation(const String& name, bool isManual);
		virtual ~Animation();

		virtual size_t calculateSize() const;
		virtual bool prepareImpl( DataStream* stream);
		virtual bool prepareImplmmap();
		AnimKeyFrame* createAnimKeyFramemmap(ui32 time, ui32 boneCount, const char*& dataPtr);
		virtual void unprepareImpl();
		virtual bool loadImpl();
		virtual void unloadImpl();

		// 分析weights
		void parseBoneWeights();

	protected:
		AnimType			m_type;			// 动画类型
		ui32				m_version;
		ui32				m_frameCount;	// 帧数量
		ui32				m_length;		// 动画长度
		ui32				m_fps;			// 帧率
		ui32				m_boneCount;	// 骨骼数量
		vector<float>::type m_boneWeights;	// 骨骼权重数组
		bool				m_bHasScaling;	// 是否包含缩放信息
		AnimKeyFrameList	m_keyFrames;	// 帧数据
		bool				m_isPartition;	// 是否为部分动画
		int					m_partitionRootBone;	// 部分动画主骨骼

#ifdef LIULIANG_ANIMATION_MMAP
		struct Header
		{
			ui32 version;	// 版本号
			ui32 fps;
			ui32 frameCount;
			ui32 boneCount;
		};

		DataStream*			m_stream;		// 文件流(唯一需要删除的指针)
		const char*			m_data;			// 文件数据
		ui32				m_size;			// 数据大小
#endif
	};
}