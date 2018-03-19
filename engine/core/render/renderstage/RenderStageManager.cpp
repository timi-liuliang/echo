#include "RenderStageManager.h"
#include "PostProcessRenderStage.h"
#include "AfterPostProcessStage.h"
#include "LDRSceneColorRenderStage.h"
#include "UIRenderStage.h"
#include "CoverageRenderStage.h"
#include "WaterRefectionStage.h"
#include "GlowRenderStage.h"
#include "SpineRenderStage.h"
#include "Engine/core/main/EngineSettings.h"
#include "Engine/core/main/Root.h"

namespace Echo
{
	__ImplementSingleton(RenderStageManager);

	// 构造函数
	RenderStageManager::RenderStageManager()
	{
		__ConstructSingleton;

		// 阴影图渲染阶段
		//m_shadowMapRenderStage = EchoNew(ShadowMapRenderStage);
		//m_shadowMapRenderStage->setEnable(EngineSettingsMgr::instance()->isActorCastShadow());
		//m_renderStages.push_back(m_shadowMapRenderStage);

		// 渲染遮罩面片阶段
		RenderStage* pRs = NULL;
		//pRs = EchoNew(CoverageRenderStage);
		//pRs->setEnable(Root::instance()->getSettingsMgr().isEnableCoverage());
		//m_renderStages.push_back(pRs);

		//渲染水面
		//pRs = EchoNew(WaterRefectRenderStage);
		//pRs->setEnable(false);
		//m_renderStages.push_back(pRs);

		// 渲染场景阶段
		pRs = EchoNew(LDRSceneColorRenderStage);
		pRs->setEnable(true);
		m_renderStages.push_back(pRs);

		// 后处理阶段
		//pRs = EchoNew(PostProcessRenderStage);
		//pRs->setEnable(true);
		//m_renderStages.push_back(pRs);

		//pRs = EchoNew(AfterPostProcessStage);
		//pRs->setEnable(true);
		//m_renderStages.push_back(pRs);

		//pRs = EchoNew(GlowRenderStage);
		//pRs->setEnable(EngineSettingsMgr::instance()->isEnableGlow());
		//m_renderStages.push_back(pRs);

		// 2D骨骼动画阶段
		//pRs = EchoNew(SpineRenderStage);
		//pRs->setEnable(true);
		//m_renderStages.push_back(pRs);

		//pRs = EchoNew(UIRenderStage);
		//pRs->setEnable(true);
		//m_renderStages.push_back(pRs);
	}

	// 析构函数
	RenderStageManager::~RenderStageManager()
	{
		for (vector<RenderStage*>::type::iterator bit = m_renderStages.begin(); bit != m_renderStages.end();)
		{
			EchoSafeDelete(*bit, RenderStage);
			bit = m_renderStages.erase(bit);
		}

		__DestructSingleton;
	}

	bool RenderStageManager::initialize()
	{
		vector<RenderStage*>::type::iterator bit = m_renderStages.begin();
		for (; bit != m_renderStages.end(); ++bit)
		{
			(*bit)->initialize();
		}
		return true;
	}

	// 处理所有渲染阶段
	void RenderStageManager::processRenderStages()
	{
		// 处理其它渲染阶段
		for (size_t i = 0; i < m_renderStages.size(); i++)
		{
			m_renderStages[i]->render();
		}
	}

	// 通过渲染阶段ID获取渲染阶段
	RenderStage* RenderStageManager::getRenderStageByID(RenderStageID _rsid)
	{
		// 线性遍历
		for (vector<RenderStage*>::type::iterator bit = m_renderStages.begin(); bit != m_renderStages.end(); ++bit)
		{
			if ((*bit)->getID() == _rsid)
				return *bit;
		}
		return NULL;
	}
}