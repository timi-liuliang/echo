#include "EngineSettings.h"
#include <engine/core/render/render/RenderThread.h>
#include <Engine/modules/Effect/EffectSystemManager.h>
#include "Root.h"
#include "Engine/core/Render/RenderStage/RenderStageManager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "CoverageRenderStage.h"
#include "GlowRenderStage.h"
#include "EngineTimeController.h"

using namespace rapidxml;

namespace Echo
{ 
	// 获取实例
	static EngineSettingsMgr* g_engineSettings = nullptr;

	// 构造函数
	EngineSettingsMgr::EngineSettingsMgr()
		: m_isActorCastShadow(true)
		, m_isActorUseEasyShadow(false)
		, m_isShowOcclusionOccluder(true)
		, m_bEnableHighEffectActor(false)
		, m_bEnableBloom(false)
		, m_bEnableToneMapping(true)
		, m_bEnableFXAA(false)
		, m_bEnableFilterAdditional(false)
		, m_bEnableDistortion(false)
		, m_bEnableGlow(false)
		, m_bEnableLensFlare(false)
		, m_bPhysicsRender(false)
		, m_bEnableCalcThread(true)
		, m_bEnableStreamThread(true)
		, m_bEnableCoverage(false)
		, m_bEnableManualRenderActor(false)
		, m_maxEffectBatchNum(-1)
		, m_defaultEffectParticleSizePerBatch(384)
		, m_maxEffectParticlesSizePerBatch(384)
		, m_effectQuality(EQ_High)
		, m_bInitVrMode(false)
		, m_bUseVRMode(false)
		, m_bSlowDownExclusiveUI(true)
		, m_guassScaleSetting(0)
		, m_waterQuality(WQ_High)
		, m_bAutoOptMatInt(true)
		, m_waterRefectRadius(40.0f)
		, m_bEnableSmallObjectCull(false)
		, m_shadowMapSize(1024)
		, m_bEnableRenderLayer(true)
		, m_bEnableFocusBlur(false)
		, m_bEnableAnimOptimize(false)
		, m_bEnableMainActorShadow(false)
		, m_bEnableBatchMerge(true)
	{
		EchoAssert(!g_engineSettings);
		g_engineSettings = this;
	}

	// 析构函数
	EngineSettingsMgr::~EngineSettingsMgr()
	{

	}

	// 获取实例
	EngineSettingsMgr* EngineSettingsMgr::instance()
	{
		EchoAssert(g_engineSettings);
		return g_engineSettings;
	}

	// 应用设置
	void EngineSettingsMgr::Apply(const String& fileName)
	{
		Echo::String lstrFile = Root::instance()->getRootPath() + fileName;
		if (!PathUtil::IsFileExist(lstrFile))
		{
			EchoLogInfo("Config file Engine.xml not exits");
			return;
		}

		Echo::FileHandleDataStream memory(lstrFile.c_str());
		xml_document<> doc;        // character type defaults to cha
		vector<char>::type buffer;
		buffer.resize(memory.size() + 1);
		memory.readAll(&buffer[0]);
		doc.parse<0>(&buffer[0]);

		xml_node<>* pEngineConfigRoot = doc.first_node();
		if (!pEngineConfigRoot)
		{
			EchoLogInfo("invalid engine config file [%s].", fileName.c_str());
			return;
		}
		xml_node<>* pValueNode = pEngineConfigRoot->first_node();
		while (pValueNode)
		{
			String strNodeName = pValueNode->name();
			xml_attribute<>* pValue = pValueNode->first_attribute();
			if (pValue)
			{
				String strValue = pValue->value();
				if (strNodeName == "EnableHighEffectActor")
				{
					m_bEnableHighEffectActor = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableActorCastShadow")
				{
					m_isActorCastShadow = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableBloom")
				{
					m_bEnableBloom = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableToneMapping")
				{
					m_bEnableToneMapping = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableFXAA")
				{
					m_bEnableFXAA = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableCoverage")
				{
					m_bEnableCoverage = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableManualRenderActor")
				{
					m_bEnableManualRenderActor = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableFilterAdditional")
				{
					m_bEnableFilterAdditional = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableDistortion")
				{
					m_bEnableDistortion = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableGlow")
				{
					setEnableGlow(StringUtil::ParseBool(strValue));
				}
				else if (strNodeName == "EnableLensFlare")
				{
					m_bEnableLensFlare = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableCalcThread")
				{
					m_bEnableCalcThread = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "EnableStreamThread")
				{
					m_bEnableStreamThread = StringUtil::ParseBool(strValue);
				}
				else if (strNodeName == "GuassScaleSetting")
				{
					m_guassScaleSetting = StringUtil::ParseFloat(strValue);
				}
				else if (strNodeName == "EnableSmallObjectCull")
				{
					setEnableSmallObjectCull(StringUtil::ParseBool(strValue));
				}
				else if (strNodeName == "ShadowMapSize")
				{
					m_shadowMapSize = StringUtil::ParseUI32(strValue);
				}
			}
			pValueNode = pValueNode->next_sibling();
		}
		//config all
		Root::instance()->setEnableFilterAdditional(m_bEnableFilterAdditional);
		setEnableDistortion(m_bEnableDistortion);
		EffectSystemManager::instance()->setEnableDistortionRender(m_bEnableDistortion);
		Root::instance()->enableStreamThread(m_bEnableStreamThread);
	}

	// 设置是否使用流加载
	void EngineSettingsMgr::setEnableStreamThread(bool isUseStreamThread)
	{
		m_bEnableStreamThread = isUseStreamThread;
		Root::instance()->enableStreamThread(m_bEnableStreamThread);
	}

	void EngineSettingsMgr::setEnableSmallObjectCull(bool enable)
	{
#ifndef ECHO_EDITOR_MODE
		m_bEnableSmallObjectCull = enable;
#endif
	}

	// 设置是否启用阴影
	void EngineSettingsMgr::setActorCastShadow(bool isEnable)
	{
		m_isActorCastShadow = isEnable;
		ECHO_DOWN_CAST<Echo::ShadowMapRenderStage*>(Echo::RenderStageManager::instance()->
			getRenderStageByID(Echo::RSI_ShadowMap))->setEnable(isEnable);
	}

	void EngineSettingsMgr::setShadowMapSize(ui32 size)
	{
		if (isActorCastShadow() && m_shadowMapSize != size)
		{
			m_shadowMapSize = size;
			ShadowMapRenderStage* pStage = ECHO_DOWN_CAST<Echo::ShadowMapRenderStage*>(Echo::RenderStageManager::instance()->getRenderStageByID(Echo::RSI_ShadowMap));
			pStage->setShadowMapSize(m_shadowMapSize);
		}
	}

	bool EngineSettingsMgr::isEnableBlurShadow() const
	{
		return ECHO_DOWN_CAST<Echo::ShadowMapRenderStage*>(Echo::RenderStageManager::instance()->getRenderStageByID(Echo::RSI_ShadowMap))->isEnableBlurShadow();
	}

	void EngineSettingsMgr::setEnableBlurShadow(bool isEnable)
	{
		ECHO_DOWN_CAST<Echo::ShadowMapRenderStage*>(Echo::RenderStageManager::instance()->getRenderStageByID(Echo::RSI_ShadowMap))->enableBlurShadow(isEnable);
	}

	void EngineSettingsMgr::setEnableCoverage(bool isEnable)
	{
		m_bEnableCoverage = isEnable;

		CoverageRenderStage* pConverageRenderStage = ECHO_DOWN_CAST<CoverageRenderStage*>(RenderStageManager::instance()->getRenderStageByID(RSI_Coverage));
		if (pConverageRenderStage)
		{
			pConverageRenderStage->setEnable(m_bEnableCoverage);
		}
		auto renterTarget = dynamic_cast<PostProcessRenderStage*>(RenderStageManager::instance()->getRenderStageByID(RSI_PostProcess));
		if (renterTarget)
		{
			renterTarget->setImageEffectEnable("Coverage", m_bEnableCoverage);
		}
	}

	void EngineSettingsMgr::setEnableManualRenderActor(bool isEnable)
	{
		m_bEnableManualRenderActor = isEnable;
	}

	void EngineSettingsMgr::setEnableGlow(bool isEnable)
	{
		 m_bEnableGlow = isEnable; 
		 if (RenderStageManager::hasInstance())
		 {
			 auto renterTarget = dynamic_cast<GlowRenderStage*>(RenderStageManager::instance()->getRenderStageByID(RSI_Glow));
			 if (renterTarget)
			 {
				 renterTarget->setEnable(isEnable);
			 }
		 }
	}

	// 设置是否渲染物理
	void EngineSettingsMgr::setEnablePhysicsRender(bool physicsRender)
	{
		m_bPhysicsRender = physicsRender;
	}

	// 开启关闭多线程渲染
	void EngineSettingsMgr::setEnableMultiThreadRendering(bool isEnable)
	{
		Echo::g_render_thread->enableThreadedRendering(false);
	}

	//设置高斯模糊品质
	void EngineSettingsMgr::setGuassScaleSetting(float guass_setting_scale)
	{
		m_guassScaleSetting = guass_setting_scale;
		if (m_guassScaleSetting > 1.0f)
		{
			m_guassScaleSetting = 1.0f;
		}
		if (m_guassScaleSetting < 0.125f)
		{
			m_guassScaleSetting = 0.125f;
		}
	}

	// 获得高斯模糊品质
	float EngineSettingsMgr::getGuassScaleSetting()
	{
		return m_guassScaleSetting;
	}

	// 设置水面品质
	void EngineSettingsMgr::setWaterQuality(WaterQuality water_qulity)
	{
		m_waterQuality = water_qulity;
	}

	// 获得水面品质
	WaterQuality EngineSettingsMgr::getWaterQuality()
	{
		return m_waterQuality;
	}

	void EngineSettingsMgr::setSlowDownExclusiveUI(bool value)
	{
		m_bSlowDownExclusiveUI = value;
	}

	bool EngineSettingsMgr::isSlowDownExclusiveUI() const
	{
		return m_bSlowDownExclusiveUI;
	}

	void EngineSettingsMgr::setEnableAnimOptimize(bool enable)
	{
		m_bEnableAnimOptimize = enable;
	}

	bool EngineSettingsMgr::isEnableAnimOptimize() const
	{
		return m_bEnableAnimOptimize;
	}

	void EngineSettingsMgr::setEnableBatchMerge(bool enable)
	{
		m_bEnableBatchMerge = enable;
	}

	bool EngineSettingsMgr::isEnableBatchMerge() const
	{
		return m_bEnableBatchMerge;
	}

}
