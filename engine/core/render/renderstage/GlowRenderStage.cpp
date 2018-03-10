#include "GlowRenderStage.h"
#include "RenderTargetManager.h"
#include "Engine/modules/Effect/EffectSystemManager.h"
#include "RenderStageManager.h"
#include "engine/core/Scene/scene_manager.h"

namespace Echo
{

	GlowRenderStage::GlowRenderStage()
		: RenderStage(RSI_Glow)
	{
	}

	GlowRenderStage::~GlowRenderStage()
	{
		destroy();
	}

	bool GlowRenderStage::initialize()
	{
		if (!EngineSettingsMgr::instance()->isEnableGlow() || m_bInitialized)
		{
			return false;
		}

		float targetScale = 1.0f;
		RenderTarget* depthTarget = nullptr;
		if (Renderer::instance()->getDeviceFeatures().supportGLES30() && EngineSettingsMgr::instance()->isEnableDownsampleDepth())
		{
			depthTarget = RenderTargetManager::instance()->getRenderTargetByID(RTI_HalfResDepth);
			targetScale = 0.5f;
		}
		else
		{
			depthTarget = RenderTargetManager::instance()->getRenderTargetByID(RTI_LDRSceneColorMap);
		}
		
		ui32 screenWidth = ui32(Renderer::instance()->getScreenWidth() * targetScale);
		ui32 screenHeight = ui32(Renderer::instance()->getScreenHeight() * targetScale);
		RenderTarget::Options option;
		option.depthTarget = depthTarget;
		RenderTarget* target = RenderTargetManager::instance()->createRenderTarget(RTI_Glow, screenWidth, screenHeight, PF_RGBA8_UNORM, option);
		if (target)
		{
			target->setScaleFactor(targetScale);
			target->setResolutionRelative(true);
		}
		m_bInitialized = true;
		return true;
	}

	void GlowRenderStage::render()
	{
		if (!m_bEnable)
		{
			PostProcessRenderStage* postprocessStage = static_cast<PostProcessRenderStage*>(RenderStageManager::instance()->getRenderStageByID(RSI_PostProcess));
			if (postprocessStage && postprocessStage->getFinalOutput() != -1)
			{
				RenderTargetManager::instance()->updateRenderTarget(postprocessStage->getFinalOutput(), RTI_DefaultBackBuffer, false, true);
			}

			return;
		}
			

		RenderTargetManager::instance()->beginRenderTarget(RTI_Glow, true, Color(0, 0, 0, 0.5), false);
		//SceneManager::instance()->renderGlow();
		EffectSystemManager::instance()->renderGLowEffectLayer();
		RenderTargetManager::instance()->endRenderTarget(RTI_Glow);
		RenderTargetManager::instance()->invalidateFrameBuffer(RTI_Glow, true, true, false);

		PostProcessRenderStage* postprocessStage = static_cast<PostProcessRenderStage*>(RenderStageManager::instance()->getRenderStageByID(RSI_PostProcess));
		if (postprocessStage && postprocessStage->getFinalOutput() != -1)
		{
			
			bool blurEnabled = postprocessStage->getImageEffectEnable("GuassBlur");
			bool dofEnabled = postprocessStage->getImageEffectEnable("Dof");
			bool bloomEnabled = postprocessStage->getImageEffectEnable("Bloom");
			postprocessStage->setImageEffectSourceRT(postprocessStage->getFinalOutput());

			postprocessStage->setImageEffectEnable("Bloom", false);
			postprocessStage->setImageEffectEnable("GuassBlur", false);
			postprocessStage->setImageEffectEnable("Dof", false);
			postprocessStage->setImageEffectEnable("Glow", true);
			postprocessStage->setImageEffectEnable("PBR", false);
			postprocessStage->render();

			postprocessStage->setImageEffectSourceRT(RTI_LDRSceneColorMap);

			postprocessStage->setImageEffectEnable("Bloom", bloomEnabled);
			postprocessStage->setImageEffectEnable("GuassBlur", blurEnabled);
			postprocessStage->setImageEffectEnable("Dof", dofEnabled);
			postprocessStage->setImageEffectEnable("Glow", false);
			postprocessStage->setImageEffectEnable("PBR", true);
		}
	}

	void GlowRenderStage::destroy()
	{
		RenderTargetManager::instance()->destroyRenderTargetByID(RTI_Glow);
	}

	void GlowRenderStage::setEnable(bool setting)
	{
		m_bEnable = setting;
		if (m_bEnable && !m_bInitialized)
			initialize();
	}

}
