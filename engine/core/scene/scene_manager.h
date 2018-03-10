#pragma once

#include "engine/core/render/render/RenderQueue.h"
#include "engine/core/render/render/Color.h"
#include "Engine/core/Camera/Camera.h"
#include "Engine/core/Camera/CameraShadow.h"
#include "Node.h"
#include "scene.h"
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
		// 根据材质模板添加渲染队列
		RenderQueue* addRenderQueue(const String& materialTemplate, const char* stage, const String& macros, bool isAlphaTest);

		// 根据渲染队列名称获取渲染队列
		RenderQueue* getRenderQueue(const String& strQueueName) const;

		// 根据索引获取渲染队列
		RenderQueue* getRenderQueueByIndex(ui32 nIndex) const;

		// 根据名称获取队列索引
		ui8 getRenderQueueIndex(const String& strQueueName) const;

		// 获取队列数量
		size_t getRenderQueueCount() { return m_RenderQueueGroup.size(); }

		// 渲染队列执行(包含endQueue)
		void execRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue=false);

		// 清空渲染队列(包含endQueue)
		void clearRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue = false);

	public:
		void update( float elapsedTime);

		void render();

		// 获取阴影摄像机
		CameraShadow& getShadowCamera() { EchoAssert( m_pShadowCamera);  return *m_pShadowCamera; }

		// 阴影有效距离
		float getShadowDistance(){ return m_shadowDistance; }
		void setShadowDistance(float dis){ m_shadowDistance = dis; }

	protected:
		void initRenderQueueGroupManual();
		void initRenderQueueGroup();
		void destroyRenderQueueGroup();

	protected:
		bool				m_isSceneVisible; 
		Camera*			    m_pMainCamera;			// 主摄像机
		Camera*				m_p2DCamera;			// 2D摄像机
		Camera*				m_pGUICamera;			// 界面摄像机
		CameraShadow*		m_pShadowCamera;		// 阴影图摄像机		
		Vector4				m_LightParam[3];		// 主光源朝向,主光源颜色，环境光颜色
		Vector4				m_LightParamForActor[3];
		Vector4				m_LightParamForSceneObject[3];
		Vector4				m_FogParam[3][3];			// param, color, mainpos
		Vector4				m_UIFogParam[3];		// UI Fog Param
		Vector4				m_heightFogParam;		// x:start height, y:deep
		Vector4				m_UIheightFogParam;		// x:start height, y:deep
		bool				m_bNeedUpate;
		RenderQueueGroup	m_RenderQueueGroup;
		bool				m_bIsRenderActorHigh;
		bool				m_isUseXRay;
		bool				m_bisComputeActorFog;
		std::map<ui32, Node*>	m_sceneNodes;
		Vector3				m_mainActorPos;
		Vector3				m_lookatPos;
		float				m_shadowDistance;		// 阴影有效距离
	};
}
