#pragma once

#include <Foundation/Math/EchoMath.h>
#include <Engine/Scene/node.h>
#include "spine/spine.h"

namespace Spine
{
	/**
	 * SpineActor
	 */
	class Actor
	{
	public:
		Actor(const char* resName);
		~Actor();

		// 更新(second)
		void update(float delta, bool isForUI=false);

		// 获取位置
		ECHO::node& getSceneNode() { return *m_sceneNode; }

		// 播放动画
		void playAnimation( const char* animName);

		// 设置缩放
		void setScale(float scale) { m_sceneNode->setLocalScalingXYZ(scale, scale, scale); }

		// 设置位置
		void setPosition(const ECHO::Vector3& pos) { m_sceneNode->setLocalPosition(pos); }

		// 设置旋转
		void setOrientation(const ECHO::Quaternion& ort) { m_sceneNode->setLocalOrientation(ort); }

	public:
		// 提交到渲染队列
		void submitToRenderQueue();

	private:
		ECHO::node*		m_sceneNode;		// 结点
		spAtlas*				m_spAtlas;			// 图集
		spSkeleton*				m_spSkeleton;		// 骨架
		spAnimationState*		m_spAnimState;		// 动画状态
		spAttachmentLoader*		m_attachmendLoader;	// 加载器
		float*					m_worldVertices;	// 顶点数据
		ECHO::Matrix4			m_matWVP;			// 世界观察投影矩阵
	};
}