#include "engine/core/Base/EchoDef.h"
#include "LDRSceneColorRenderStage.h"
#include "Engine/modules/Effect/EffectSystemManager.h"
#include "Engine/core/Scene/NodeTree.h"
#include "Engine/core/Camera/Camera.h"
#include "Engine/core/main/Root.h"
#include "engine/core/render/RenderTargetManager.h"
#include "render/RenderQueueGroup.h"

namespace Echo
{
	// 构造函数
	LDRSceneColorRenderStage::LDRSceneColorRenderStage()
		: RenderStage(RSI_LDRSceneColor)
	{
	}

	// 析构函数
	LDRSceneColorRenderStage::~LDRSceneColorRenderStage()
	{
		destroy();
	}

	bool LDRSceneColorRenderStage::initialize()
	{
		return true;
	}

	void LDRSceneColorRenderStage::destroy()
	{
	}

	// 渲染
	void LDRSceneColorRenderStage::render()
	{
        if (!m_bEnable)
            return;
            
		bool isRenderScene = Root::instance()->isRenderScene();

		EchoAssert(NodeTree::instance());
		EchoAssert(RenderTargetManager::instance());
		RenderTargetID ldrRendertarget = RTI_LDRSceneColorMap;

		if (EngineSettingsMgr::instance()->isInitVRMode() && EngineSettingsMgr::instance()->isUseVRMode())
		{
			// 绑定渲染目标
			RenderTargetManager::instance()->beginRenderTarget(RTI_LDRVRSceneColorMap);

			// 1
			RenderTargetManager::instance()->getRenderTargetByID(RTI_LDRVRSceneColorMap)->bindTarget_left();
// 			EchoSceneManager->getMainCamera()->setVRModeForEye(true);
// 			EchoRoot->frameMove(0);
			//SceneManager::instance()->renderScene(Root::instance()->isRenderScene());
			// 2
			RenderTargetManager::instance()->getRenderTargetByID(RTI_LDRVRSceneColorMap)->bindTarget_right();
			NodeTree::instance()->get3dCamera()->setVRModeForEye(false);
			Root::instance()->tick(0);
			//SceneManager::instance()->renderScene(Root::instance()->isRenderScene());

			// 换切渲染目标
			RenderTargetManager::instance()->endRenderTarget(RTI_LDRVRSceneColorMap);
		}
		else
		{
			// 绑定渲染目标
			RenderTargetManager::instance()->beginRenderTarget(ldrRendertarget);

#ifdef ECHO_EDITOR_MODE
			// 编辑器模式下QueryObject渲两遍(解决半透无法看到网格)
			//QueryObjectManager::instance()->render(QueryObjectManager::QOT_Default | QueryObjectManager::QOT_NavMesh | QueryObjectManager::QOT_Physics);
#endif
			// 渲染场景
			//SceneManager::instance()->renderScene(isRenderScene);

			RenderQueueGroup::instance()->execRenderQueue("Opaque", "Transparent+3", true);
		}

		/*if (EchoRoot->getEnableDistortion() && RenderTargetManager::Instance()->getRenderTargetByID(RTI_SceneColorMap))
		{
			RenderTargetManager::Instance()->endRenderTarget(RTI_LDRSceneColorMap);
			if (EchoRoot->getEnableFilterAdditional())
			{
				RenderTargetManager::Instance()->updateRenderTargetWithFilter(RTI_LDRSceneColorMap, RTI_SceneColorMap);
			}
			else
			{
				RenderTargetManager::Instance()->updateRenderTarget(RTI_LDRSceneColorMap, RTI_SceneColorMap);
			}

			RenderTargetManager::Instance()->beginRenderTarget(RTI_LDRSceneColorMap, false, Renderer::BGCOLOR, false, 1.0);
			EchoEffectSystemManager->renderDistortionLayer();
		}*/
		//QueryObjectManager::instance()->render(QueryObjectManager::QOT_Default | QueryObjectManager::QOT_NavMesh | QueryObjectManager::QOT_Physics);

		// 环境探针处理
		//Scene* pCurScene = SceneManager::instance()->getCurrentScene();
		//if (pCurScene)
		//{
			//LightProbeManager* lpm = pCurScene->getLightProbeManager();
			//if (lpm)
			{
				//lpm->displayLightProbes();
			}
		//}

		// 换切渲染目标
		RenderTargetManager::instance()->endRenderTarget(ldrRendertarget);
        RenderTargetManager::instance()->invalidateFrameBuffer(ldrRendertarget, true, false, false);

		if (Renderer::instance()->getDeviceFeatures().supportGLES30() && EngineSettingsMgr::instance()->isEnableDownsampleDepth())
		{
			RenderTargetManager::instance()->downsampleDepthTarget(ldrRendertarget, RTI_HalfResDepth);
		}

		if (Root::instance()->getEnableFilterAdditional())
		{
			RenderTargetManager::instance()->updateRenderTargetWithFilter(ldrRendertarget, ldrRendertarget);
		}
	}
}
