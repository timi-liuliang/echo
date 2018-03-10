#include "AfterPostProcessStage.h"
#include "Engine/core/main/Root.h"
#include "RenderStageManager.h"
#include "Engine/modules/Effect/EffectSystemManager.h"
#include "RenderStageManager.h"
#include "Engine/core/Render/RenderTargetManager.h"

namespace Echo
{
	AfterPostProcessStage::AfterPostProcessStage()
		: RenderStage(RSI_AfterPostProcess)
	{

	}

	AfterPostProcessStage::~AfterPostProcessStage()
	{
		destroy();
	}

	bool AfterPostProcessStage::initialize()
	{
		return true;
	}

	void AfterPostProcessStage::render()
	{
		if (m_bEnable)
		{
			Echo::RenderStageManager* manager = Echo::RenderStageManager::instance();
			Echo::PostProcessRenderStage* renderStage = (Echo::PostProcessRenderStage*)manager->getRenderStageByID(Echo::RSI_PostProcess);
			if (renderStage)
			{
				i32 _finalPostID =  renderStage->getFinalOutput();
				if (_finalPostID != -1 && Root::instance()->isRenderScene())
				{
					//if (!Renderer::instance()->getDeviceFeatures().supportHFColorBf())
						//SceneManager::instance()->renderTransparent(true);

					RenderTargetManager::instance()->beginRenderTarget(_finalPostID,false,Renderer::BGCOLOR,false);
					EffectSystemManager::instance()->endRender();
					RenderTargetManager::instance()->endRenderTarget(_finalPostID);
				}
				else
				{
					//场景不渲染时清理一下特效队列 半透明队列
					EffectSystemManager::instance()->beginRender();
					//if (!Renderer::instance()->getDeviceFeatures().supportHFColorBf())
						//SceneManager::instance()->renderTransparent(false);
				}
			}

			if (Root::instance()->getSettingsMgr().isEnableManualRenderActor())
			{
				RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer, false, Renderer::BGCOLOR, false, 1.0);
				//if (Root::instance()->getActorManagerUnsafe())
				//{
				//	EchoEngineActorManager->updateManualActors(Root::instance()->getFrameTime());
				//	EchoEngineActorManager->renderManualActors();
				//}

				RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
			}
		}
		
	}

	void AfterPostProcessStage::destroy()
	{

	}
}
