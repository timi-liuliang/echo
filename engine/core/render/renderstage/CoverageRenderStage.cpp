#include "CoverageRenderStage.h"
#include "engine/core/Scene/scene_manager.h"
#include "engine/core/main/EngineSettings.h"
#include "engine/core/render/render/Material.h"
#include "Engine/core/Render/RenderTargetManager.h"
#include "Engine/core/Render/MaterialInstance.h"

namespace Echo
{
	// 构造函数
	CoverageRenderStage::CoverageRenderStage()
		: RenderStage(RSI_Coverage)
		, m_coverageSamplerState(NULL)
		, m_pMtlCoverage(NULL)
	{
	}

	// 析构函数
	CoverageRenderStage::~CoverageRenderStage()
	{
		destroy();
	}

	bool CoverageRenderStage::initialize()
	{
		if (!EngineSettingsMgr::instance()->isEnableCoverage() || m_bInitialized)
		{
			return false;
		}
		// 初始化遮盖图
		ui32 screenWidth = Renderer::instance()->getScreenWidth();
		ui32 screenHeight = Renderer::instance()->getScreenHeight();

		// 创建一个float值得纹理
		RenderTarget::Options option;
		RenderTarget* target = RenderTargetManager::instance()->createRenderTarget(RTI_CoverageMap, 
			screenWidth, screenHeight, PF_RGBA8_UNORM, option);
		if (target)
		{
			SamplerState::SamplerDesc desc;
			desc.addrUMode = SamplerState::AM_CLAMP;
			desc.addrVMode = SamplerState::AM_CLAMP;
			desc.addrWMode = SamplerState::AM_CLAMP;
			m_coverageSamplerState = Renderer::instance()->getSamplerState(desc);
		}
		else
		{
			EchoLogError("RenderTargetManager::createRenderTarget( m_coverageSamplerState ) ... Failed");
			return false;
		}

		// 加载生成遮罩图的材质
		m_pMtlCoverage = MaterialManager::instance()->createMaterial();
		EchoAssert(m_pMtlCoverage);
		m_pMtlCoverage->loadFromFile("pp_CoverageMap.xml","");

		return true;
	}

	void CoverageRenderStage::destroy()
	{
		MaterialManager::instance()->destroyMaterial(m_pMtlCoverage);
	}

	void CoverageRenderStage::addQueryObject(class QueryObject* pQo)
	{
		m_vecQueryObjects.push_back(pQo);
	}

	// 渲染
	void CoverageRenderStage::render()
	{
		if (m_bEnable)
		{ 
			EchoAssert(SceneManager::instance());
			EchoAssert(RenderTargetManager::instance());
			ui32 coverageRendertarget;

			coverageRendertarget = RTI_CoverageMap;
			// 绑定渲染目标
			RenderTargetManager::instance()->beginRenderTarget(coverageRendertarget,true,Color(0,0,0,0.5)); 

	//		m_pMtlCoverage->active();
			//for (QueryObjectVector::iterator bit = m_vecQueryObjects.begin(); bit != m_vecQueryObjects.end(); ++bit)
			//{
				// 使用自定义材质渲染QueryObject
//				(*bit)->setCustomProgram(m_pMtlCoverage->getShaderProgram());
				//(*bit)->render();
//				(*bit)->setCustomProgram(NULL);
			//}
			m_vecQueryObjects.clear();
		}
	}
}