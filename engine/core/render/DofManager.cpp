#include "DofManager.h"
#include "RenderStage/RenderStageManager.h"
#include "RenderStage/PostProcessRenderStage.h"
#include "PostImageEffect.h"

namespace Echo
{
	DofManager::DofManager()
		: m_bloomEnabled(false)
		, m_gaussianBlur(false)
		, m_dofEnabled(false)
	{

	}
	DofManager::~DofManager()
	{

	}

	void DofManager::setEnableDof(bool _val)
	{
		if (_val == m_dofEnabled)
		{
			return;
		}
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			m_dofEnabled = _val;
			renderStage->setImageEffectEnable("Dof", _val);
			if (_val)
			{
				m_bloomEnabled = renderStage->getImageEffectEnable("Bloom");
				renderStage->setImageEffectEnable("Bloom", _val);
			}
			else
			{
				renderStage->setImageEffectEnable("Bloom", m_bloomEnabled);
			}
		}
	}
	bool DofManager::getEnableDof() const
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			return renderStage->getImageEffectEnable("Dof");
		}
		return false;
	}
	void DofManager::setDofParam(float focus, float attenuation)
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue[0] = focus;
			renderStage->setImageEffectParameter("Dof", "dof_h", uniformValue);
			renderStage->setImageEffectParameter("Dof", "dof_c", uniformValue);
			uniformValue[0] = attenuation;
			renderStage->setImageEffectParameter("Dof", "attenuation", uniformValue);
		}
	}
	void DofManager::setBright(float bright)
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue[0] = bright;
			renderStage->setImageEffectParameter("Dof", "bright", uniformValue);
		}
	}
	float DofManager::getParam(DofManager::DOF_PARAM param_type)
	{
		Echo::String sParamName = "";
		switch (param_type)
		{
		case Echo::DofManager::DOF_Param_Fource:
			sParamName = "dof_h";
			break;
		case Echo::DofManager::DOF_Param_Attenuation:
			sParamName = "attenuation";
			break;
		case Echo::DofManager::DOF_Param_Bright:
			sParamName = "bright";
			break;
		default:
			EchoLogInfo("dof getParam param_type error");
			break;
		}

		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue = renderStage->getImageEffectParameter("Dof", sParamName);

			return uniformValue.x;
		}

		return 0.0f;
	}

	void DofManager::setEnableGuassBlur(bool _val)
	{
		if (_val == m_gaussianBlur)
		{
			return;
		}
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			m_gaussianBlur = _val;
			renderStage->setImageEffectEnable("GuassBlur", _val);
			if (_val)
			{
				m_bloomEnabled = renderStage->getImageEffectEnable("Bloom");
				renderStage->setImageEffectEnable("Bloom", _val);
			}
			else
			{
				renderStage->setImageEffectEnable("Bloom", m_bloomEnabled);
			}
		}
	}
	bool DofManager::getEnableGuassBlur()const
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			return renderStage->getImageEffectEnable("GuassBlur");
		}
		return false;
	}
	void DofManager::setGuassBright(float bright)
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue[0] = bright;
			renderStage->setImageEffectParameter("GuassBlur", "bright", uniformValue);
		}
	}

	void DofManager::cacheGuassBlurOneFrame()
	{
	}
	void DofManager::uncacheGuassBlurOneFrame()
	{
	}

	void DofManager::setMotionBlurEnable(bool enable)
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			renderStage->setImageEffectEnable("MotionBlur", enable);
		}
	}

	bool DofManager::getMotionBlurEnable() const
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			return renderStage->getImageEffectEnable("MotionBlur");
		}
		return false;
	}

	void DofManager::setMotionBlurParam(float param_value)
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue[0] = Math::Clamp(param_value, 0.f, 1.f);
			renderStage->setImageEffectParameter("MotionBlur", "blur_param", uniformValue);
		}
	}

	float DofManager::getMotionBlurParam()
	{
		auto renderStage = (Echo::PostProcessRenderStage*)RenderStageManager::instance()->getRenderStageByID(Echo::RSI_PostProcess);
		if (renderStage)
		{
			Vector4 uniformValue;
			uniformValue = renderStage->getImageEffectParameter("MotionBlur", "blur_param");

			return uniformValue.x;
		}

		return 0.0f;
	}

}