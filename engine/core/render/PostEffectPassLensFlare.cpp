#include "PostEffectPassLensFlare.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/ShaderProgram.h"
#include "engine/core/render/RenderTargetManager.h"
#include "rapidxml/rapidxml.hpp"
#include "Engine/core/Render/MaterialInstance.h"

namespace Echo
{
	StarDef::StarDef()
	{
		arrChromaticAberrationColor[0] = Color(0.5f, 0.5f, 0.5f, 0.0f);
		arrChromaticAberrationColor[1] = Color(0.8f, 0.3f, 0.3f, 0.0f);
		arrChromaticAberrationColor[2] = Color(1.0f, 0.2f, 0.2f, 0.0f);
		arrChromaticAberrationColor[3] = Color(0.5f, 0.2f, 0.6f, 0.0f);
		arrChromaticAberrationColor[4] = Color(0.2f, 0.2f, 1.0f, 0.0f);
		arrChromaticAberrationColor[5] = Color(0.2f, 0.3f, 0.7f, 0.0f);
		arrChromaticAberrationColor[6] = Color(0.2f, 0.6f, 0.2f, 0.0f);
		arrChromaticAberrationColor[7] = Color(0.3f, 0.5f, 0.3f, 0.0f);
	}

	PostEffectPassLensFlare::PostEffectPassLensFlare(PostImageEffect* parent)
		: PostImageEffectPass(parent)
		, m_materialMerge(nullptr)
		, m_renderableMerge(nullptr)
	{
		m_starDef.nStarLines = 2;
		m_starDef.nLinePasses = 3;
		m_starDef.attenuation = 0.96f;
		m_starDef.chromaticAberration = 0.5f;
		m_starDef.sampleLength = 0.5f;
		m_starDef.starLines[0].sinAngle = Math::Sin(Math::Rad(90.0f));
		m_starDef.starLines[0].cosAngle = Math::Cos(Math::Rad(90.0f));
		m_starDef.starLines[1].sinAngle = Math::Sin(Math::Rad(-90.0f));
		m_starDef.starLines[1].cosAngle = Math::Cos(Math::Rad(-90.0f));

		memset(m_rtStarTemp, 0, sizeof(m_rtStarTemp));
		memset(m_rtStarLine, 0, sizeof(m_rtStarLine));

		m_sampleOffsets.assign(Vector2::ZERO);
		m_sampleWeights.assign(Vector4::ZERO);

		const Color colorWhite(0.60f, 0.60f, 0.70f, 0.0f);
		for (int p = 0; p < MAX_PASSES; p++)
		{
			float ratio;
			ratio = (float)(p + 1) / (float)MAX_PASSES;

			for (int s = 0; s < 8; s++)
			{
				Color chromaticAberrColor = m_starDef.arrChromaticAberrationColor[s] + (colorWhite - m_starDef.arrChromaticAberrationColor[s]) * ratio;
				m_aaColor[p][s] = colorWhite + (chromaticAberrColor - colorWhite) * m_starDef.chromaticAberration;
			}
		}

	}

	PostEffectPassLensFlare::~PostEffectPassLensFlare()
	{
		MaterialManager::instance()->destroyMaterial(m_materialMerge);
		RenderInput* renderinput = m_renderableMerge->getRenderInput();
		EchoSafeDelete(renderinput, RenderInput);
		Renderer::instance()->destroyRenderables(&m_renderableMerge, 1);

		RenderTargetManager::instance()->destroyRenderTargetByID(m_rtStarTemp[0]);
		RenderTargetManager::instance()->destroyRenderTargetByID(m_rtStarTemp[1]);
		RenderTargetManager::instance()->destroyRenderTargetByID(m_rtStarLine[0]);
		RenderTargetManager::instance()->destroyRenderTargetByID(m_rtStarLine[1]);
	}

	void PostEffectPassLensFlare::process()
	{
		if (!EngineSettingsMgr::instance()->isEnableLensFlare())
			return;

		const auto& sampler = m_samplers[0];
		auto rendtargetMgr = RenderTargetManager::instance();
		RenderTarget* rtFinal = rendtargetMgr->getRenderTargetByID(m_renderTargetID);
		float targetWidth = static_cast<Real>(rtFinal->width());
		float targetHeight = static_cast<Real>(rtFinal->height());

		for (int d = 0; d < m_starDef.nStarLines; ++d)
		{
			ui32 sourceRT = rendtargetMgr->getRenderTargetID(sampler.sourceRT);
			
			Vector2 stepUV;
			stepUV.x = m_starDef.starLines[d].sinAngle / targetWidth;
			stepUV.y = m_starDef.starLines[d].cosAngle / targetHeight;

			float attnPowScale = 1.6f;

			int iWorkTexture = 0;
			for (int p = 0; p < m_starDef.nLinePasses; ++p)
			{
				ui32 target = 0;
				if (p == m_starDef.nLinePasses - 1)
				{
					target = m_rtStarLine[d];
				}
				else
				{
					target = m_rtStarTemp[iWorkTexture];
				}

				const int nSamples = 8;
				for (int i = 0; i < nSamples; ++i)
				{
					float lum;
					lum = Math::Pow(m_starDef.attenuation, attnPowScale * i);

					Color aaColor = m_aaColor[m_starDef.nLinePasses - 1 - p][i] * lum * (p + 1.0f) * 0.5f;
					m_sampleWeights[i] = Vector4(aaColor.r, aaColor.g, aaColor.b, aaColor.a);

					m_sampleOffsets[i].x = stepUV.x * i;
					m_sampleOffsets[i].y = stepUV.y * i;
					if (Math::Abs(m_sampleOffsets[i].x) >= 0.9f ||
						Math::Abs(m_sampleOffsets[i].y) >= 0.9f)
					{
						m_sampleOffsets[i].x = 0.0f;
						m_sampleOffsets[i].y = 0.0f;
						m_sampleWeights[i] *= 0.0f;
					}
				}

				RenderTarget* rt = rendtargetMgr->getRenderTargetByID(sourceRT);
				m_renderable->setTexture(sampler.slot, rt->getBindTexture(), m_material->getSamplerState(sampler.slot));

				rendtargetMgr->beginRenderTarget(target, false, Renderer::BGCOLOR, false);
				m_renderable->render();
				rendtargetMgr->endRenderTarget(target);

				stepUV *= nSamples * m_starDef.sampleLength;
				attnPowScale *= nSamples * m_starDef.sampleLength;

				sourceRT = m_rtStarTemp[iWorkTexture];
				iWorkTexture = (++iWorkTexture) % 2;
			}
		}

		RenderTarget* rt = rendtargetMgr->getRenderTargetByID(m_rtStarLine[0]);
		m_renderableMerge->setTexture(0, rt->getBindTexture(), m_materialMerge->getSamplerState(0));
		rt = rendtargetMgr->getRenderTargetByID(m_rtStarLine[1]);
		m_renderableMerge->setTexture(1, rt->getBindTexture(), m_materialMerge->getSamplerState(1));

		rendtargetMgr->beginRenderTarget(m_renderTargetID, false, Renderer::BGCOLOR, false);
		m_renderableMerge->render();
		rendtargetMgr->endRenderTarget(m_renderTargetID);
	}

	void PostEffectPassLensFlare::initializeShaderParams()
	{
		if (m_samplers.size() == 0)
		{
			EchoLogError("[PostEffectPassLensFlare:%d]::Samplers is empty.", __LINE__);
			return;
		}
		
		ShaderProgram* shader = m_material->getShaderProgram();
		ShaderProgram::UniformArray* uniformArray = shader->getUniforms();
		ShaderProgram::UniformArray::iterator itUnifrom = uniformArray->begin();
		m_renderable->beginShaderParams(uniformArray->size());
		for (; itUnifrom != uniformArray->end(); ++itUnifrom)
		{
			const ShaderProgram::Uniform& uniform = itUnifrom->second;
			void* value = getUniformValue(uniform.m_name);
			m_renderable->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name), uniform.m_type, value, uniform.m_count);
		}
		m_renderable->endShaderParams();

		shader = m_materialMerge->getShaderProgram();
		uniformArray = shader->getUniforms();
		itUnifrom = uniformArray->begin();
		m_renderableMerge->beginShaderParams(uniformArray->size());
		for (; itUnifrom != uniformArray->end(); ++itUnifrom)
		{
			const ShaderProgram::Uniform& uniform = itUnifrom->second;
			void* value = getUniformValue(uniform.m_name);
			m_renderableMerge->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name), uniform.m_type, value, uniform.m_count);
		}
		m_renderableMerge->endShaderParams();
	}

	void PostEffectPassLensFlare::loadCustom(rapidxml::xml_node<char>* node)
	{
		rapidxml::xml_attribute<>* attrCustomMaterial = node->first_attribute("customMaterial");
		m_materialMerge = createMaterial(attrCustomMaterial->value());
		m_renderableMerge = createRenderable(m_materialMerge);

		rapidxml::xml_attribute<>* scaleAttr = node->first_attribute("scale");
		float scale = StringUtil::ParseFloat(scaleAttr->value());
		m_rtStarTemp[0] = createCustomTarget("LensFlareTemp0", scale);
		m_rtStarTemp[1] = createCustomTarget("LensFlareTemp1", scale);
		m_rtStarLine[0] = createCustomTarget("LensFlareStarLine0", scale);
		m_rtStarLine[1] = createCustomTarget("LensFlareStarLine1", scale);
	}

	void* PostEffectPassLensFlare::getUniformValue(const String& name)
	{
		static ui32 mergeSamplerSlot0 = 0;
		static ui32 mergeSamplerSlot1 = 1;

		if (name == "texSampler")
		{
			return (void*)&m_samplers[0].slot;
		}
		else if (name == "g_sampleOffsets")
		{
			return m_sampleOffsets.data();
		}
		else if (name == "g_sampleWeights")
		{
			return m_sampleWeights.data();
		}
		else if (name == "texSamplerSrc0")
		{
			return &mergeSamplerSlot0;
		}
		else if (name == "texSamplerSrc1")
		{
			return &mergeSamplerSlot1;
		}
		return nullptr;
	}

	ui32 PostEffectPassLensFlare::createCustomTarget(const String& name, float scale)
	{
		ui32 targetID = RenderTargetManager::instance()->allocateRenderTargetID(name);
		ui32 screenWidth = Renderer::instance()->getScreenWidth();
		ui32 screenHeight = Renderer::instance()->getScreenHeight();
		ui32 w = static_cast<ui32>(screenWidth * scale);
		ui32 h = static_cast<ui32>(screenHeight * scale);

		RenderTarget::Options option;
		option.depth = true;
		option.multiResolution = RenderTargetManager::instance()->getEnableLogicResolution();
		auto rt = RenderTargetManager::instance()->createRenderTarget(targetID, w, h, PF_RGBA8_UNORM, option);
		if (rt)
		{
			rt->setScaleFactor(scale);
			rt->setResolutionRelative(true);
			return targetID;
		}
		else
		{
			EchoLogError("RenderTargetManager::createRenderTarget( %s ) ... Failed", name.c_str());
			return RTI_End;
		}
	}

}