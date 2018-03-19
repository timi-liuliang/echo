#pragma once

#include "engine/core/render/render/RenderQueue.h"
#include "engine/core/render/render/Color.h"
#include "Engine/core/Camera/Camera.h"
#include "Engine/core/Camera/CameraShadow.h"
#include "Node.h"
#include "Engine/core/Render/TextureRes.h"
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	/**
	 * 场景管理器
	 */
	class SceneManager
	{	
		__DeclareSingleton(SceneManager);

	public:
		SceneManager();
		virtual ~SceneManager();

		// 初始化
		bool initialize();

		// 销毁
		void destroy();

	public:
		// 获取主摄像机
		Camera* getMainCamera() const { return m_pMainCamera; }

		// 设置主摄像机
		void setMainCamera(Camera* camera) { m_pMainCamera = camera; }

		// 获取2D摄像机
		Camera* get2DCamera() const { return m_p2DCamera; }

		// 获取界面摄像机
		Camera* getGUICamera() const { return m_pGUICamera; }

		// 设置界面摄像机
		void setGUICamera(Camera* pCamera) { m_pGUICamera = pCamera; }

	public:
		void update( float elapsedTime);

		// 获取阴影摄像机
		CameraShadow& getShadowCamera() { EchoAssert( m_pShadowCamera);  return *m_pShadowCamera; }

		// 阴影有效距离
		float getShadowDistance(){ return m_shadowDistance; }
		void setShadowDistance(float dis){ m_shadowDistance = dis; }

	protected:
		Camera*			    m_pMainCamera;			// 主摄像机
		Camera*				m_p2DCamera;			// 2D摄像机
		Camera*				m_pGUICamera;			// 界面摄像机
		CameraShadow*		m_pShadowCamera;		// 阴影图摄像机		
		bool				m_bNeedUpate;
		float				m_shadowDistance;		// 阴影有效距离

		Node*				m_invisibleRoot;		// invisible root node
	};
}
