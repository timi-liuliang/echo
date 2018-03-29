#pragma once

#include "Node.h"
#include "engine/core/Util/Singleton.h"
#include "Engine/core/Camera/Camera.h"
#include "Engine/core/Camera/CameraShadow.h"

namespace Echo
{
	class NodeTree
	{	
		__DeclareSingleton(NodeTree);

	public:
		NodeTree();
		virtual ~NodeTree();

		// 初始化
		bool init();

		// 销毁
		void destroy();

	public:
		// 获取主摄像机
		Camera* get3dCamera() const { return m_3dCamera; }

		// 设置主摄像机
		void setMainCamera(Camera* camera) { m_3dCamera = camera; }

		// 获取2D摄像机
		Camera* get2DCamera() const { return m_2dCamera; }

		// 获取界面摄像机
		Camera* getGUICamera() const { return m_uiCamera; }

		// 设置界面摄像机
		void setGUICamera(Camera* pCamera) { m_uiCamera = pCamera; }

	public:
		void update( float elapsedTime);

		// 获取阴影摄像机
		CameraShadow& getShadowCamera() { EchoAssert( m_shadowCamera);  return *m_shadowCamera; }

	protected:
		Camera*			    m_3dCamera;			// 主摄像机
		Camera*				m_2dCamera;			// 2D摄像机
		Camera*				m_uiCamera;			// 界面摄像机
		CameraShadow*		m_shadowCamera;		// 阴影图摄像机		
		Node*				m_invisibleRoot;	// invisible root node
	};
}
