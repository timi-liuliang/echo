#include "MaterialInst.h"
#include "Engine/modules/Light/Light.h"
#include "Engine/core/Scene/NodeTree.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/MaterialDesc.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/render/TextureResManager.h"
#include "render/RenderQueueGroup.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_helper.hpp"

using namespace rapidxml;

namespace
{
	const char* globalUnfiorms[] = {
		"matW", "matV", "matWV", "matWVP", "matWVPWater", "matWVPSky", "camPos",
		"CameraPosition", "CameraDirection", "LightArrayInfo", "LightArrayData",
		"SceneObjectLightParams", "matBoneRows", "fogParam", "heightFogParam",
		"LMScale1", "LMScale2", "LMUV", "LMSampler1", "LMSampler2", "matWVPSM",
		"ShadowShade", "SMSampler", "CurrentTime", "ActorLitParam", "RefectSampler",
		"RefractionSampler", "SceneEnvSampler", "uLightPositions", "uLightDirections",
		"uLightColors", "uLightParams", "uLightSpot", "offset", "tShadowAlbedo"
	};
}

namespace Echo
{
	// 克隆
	MaterialInst::uniform* MaterialInst::uniform::clone()
	{
		uniform* result = EchoNew(uniform);
		result->name = name;
		result->type = type;
		result->count = count;
		if (value)
			result->value = MaterialInst::cloneVoid2Value(type, value, count);
		else
			result->value = NULL;

		return result;
	}

	// 构造函数
	MaterialInst::MaterialInst()
		: m_TextureCount(0)
		, m_material(NULL)
		, m_isHaveCustomBlend(false)
		, m_blendState(NULL)
		, m_isHaveCustomRasterizer(false)
		, m_rasterizerState(NULL)
		, m_isHaveCustomDepthStencil(false)
		, m_depthStencil(NULL)
		, m_isTemplate(false)
		, m_isSubmitToStageRenderQueue(false)
		, m_officialMaterialContent(nullptr)
	{

	}

	// 析构函数
	MaterialInst::~MaterialInst()
	{
		for (auto& element : m_unifromParamSet)
		{
			if (!element.second)
				continue;

			delVoid2Value(element.second->type, element.second->value, element.second->count);
			EchoSafeDelete(element.second, uniform);
		}
		m_unifromParamSet.clear();

		for (auto& element : m_unifromParamSetFromFile)
		{
			if (!element.second)
				continue;

			delVoid2Value(element.second->type, element.second->value, element.second->count);
			EchoSafeDelete(element.second, uniform);
		}

		m_unifromParamSetFromFile.clear();

		unloadTexture();
		m_TexturesName.clear();

		EchoSafeDelete(m_blendState, BlendState);
		EchoSafeDelete(m_rasterizerState, RasterizerState);
		EchoSafeDelete(m_depthStencil, DepthStencilState);
	}

	// create a material instance
	MaterialInst* MaterialInst::create()
	{
		return EchoNew(MaterialInst);
	}

	// 加载(内存)
	bool MaterialInst::loadByFile(const String& name, const String& macros)
	{
		try
		{
			m_name = name;
			m_macrosEx = StringUtil::Split(macros, ";");
			std::sort(m_macrosEx.begin(), m_macrosEx.end());
			m_isTemplate = true;

			MemoryReader memReader(name.c_str());
			if (!memReader.getData<char*>())
			{
				EchoLogError("Parse model file [%s] failed.", name.c_str());
				return false;
			}

			xml_document<> doc;
			doc.parse<0>(memReader.getData<char*>());

			xml_node<> *pRootNode = doc.first_node();
			if (!pRootNode)
			{
				EchoLogError("Material file content is valid. there is no root node");
				return false;
			}

			// 材质模板
			xml_node<>* matTemNode = pRootNode->first_node("MaterialTemplate");
			if (matTemNode)
			{
				m_materialTemplate = rapidxml_helper::get_string(matTemNode->first_attribute("name"), "");
				m_stage = rapidxml_helper::get_string(matTemNode->first_attribute("stage"), "");

				if (m_materialTemplate == "water.mt" || m_materialTemplate == "water2.mt" || m_materialTemplate == "water1.mt")
				{
					m_stage = "SampleWater";
				}

				m_isSubmitToStageRenderQueue = rapidxml_helper::parsebool(matTemNode->first_attribute("submit_to_stage"), false);
				m_macros = StringUtil::Split(rapidxml_helper::get_string(matTemNode->first_attribute("macros"), ""), ";");
				std::sort(m_macros.begin(), m_macros.end());
			}
			else
			{
				EchoLogError("Material file content is valid. there is no MaterialTemplate node");
				return false;
			}

			{
				xml_node<>* pSubNode = pRootNode->first_node("RasterizerState");
				if (pSubNode)
				{
					LoadRasterizerState(pSubNode);
				}
			}

			{
				xml_node<>* pSubNode = pRootNode->first_node("BlendState");
				if (pSubNode)
				{
					LoadBlendState(pSubNode);
				}
			}

			{
				xml_node<>* pSubNode = pRootNode->first_node("DepthStencilState");
				if (pSubNode)
				{
					LoadDepthStencilState(pSubNode);
				}
			}

			// 参数模板
			xml_node<>* uniformNode = pRootNode->first_node("Uniform");
			if (uniformNode)
			{
				xml_node<>* propertyNode = uniformNode->first_node("Property");
				while (propertyNode)
				{
					Echo::String uniformName = rapidxml_helper::get_string(propertyNode->first_attribute("name"), "");
					{
						uniform* pProperty = EchoNew(uniform);

						pProperty->name = uniformName;
						xml_attribute<> *pPropertyValueType = propertyNode->first_attribute("type");
						pProperty->type = S2ShaderParamType(pPropertyValueType->value());
						xml_attribute<> *pPropertyValueCount = propertyNode->first_attribute("count");
						if (pPropertyValueCount)
						{
							pProperty->count = StringUtil::ParseI32(pPropertyValueCount->value());
						}
						else
						{
							pProperty->count = 1;
						}
						xml_attribute<> *pPropertyValue = propertyNode->first_attribute("value");

						pProperty->value = createValue2Void(pProperty->type, pProperty->count);
						S2Void(pProperty->type, pPropertyValue->value(), pProperty->value, pProperty->count);

						m_unifromParamSetFromFile[pProperty->name] = pProperty;
					}

					propertyNode = propertyNode->next_sibling("Property");
				}
			}

			return true;
		}
		catch (...)
		{
			EchoLogError("Parse model file [%s] failed.", name.c_str());
		}

		return false;
	}

	bool MaterialInst::applyLoadedData()
	{
		// 加载材质模板
		buildRenderQueue();

		if (m_isHaveCustomBlend && NULL == m_blendState)
		{
			m_blendState = Renderer::instance()->createBlendState(m_blendDesc);
		}

		if (m_isHaveCustomRasterizer && NULL == m_rasterizerState)
		{
			m_rasterizerState = Renderer::instance()->createRasterizerState(m_rasterizerStateDesc);
		}

		if (m_isHaveCustomDepthStencil && NULL == m_depthStencil)
		{
			m_depthStencil = Renderer::instance()->createDepthStencilState(m_depthStencilDesc);
		}

		// 获取着色器
		ShaderProgram* shaderProgram = m_material->getShaderProgram();
		if (!shaderProgram)
		{
			return false;
		}

		auto iter = m_unifromParamSetFromFile.begin();
		for (; iter != m_unifromParamSetFromFile.end(); ++iter)
		{
			if (shaderProgram->getUniform(iter->first) && !isGlobalUniform(iter->first))
			{
				m_unifromParamSet[iter->first] = iter->second;
				iter->second = NULL;
			}
		}

		// 随手清理Load文件剩余的UniformProperty
		for (auto& element : m_unifromParamSetFromFile)
		{
			if (!element.second)
				continue;

			delVoid2Value(element.second->type, element.second->value, element.second->count);
			EchoSafeDelete(element.second, uniform);
		}

		m_unifromParamSetFromFile.clear();

#ifdef ECHO_EDITOR_MODE
		// 参数匹配
		matchUniforms();
#endif
		return true;
	}

	// 保存到文件
	void MaterialInst::saveToFile(const String& name)
	{
		using namespace rapidxml;

		rapidxml::xml_document<> doc;
		Echo::String str;
		char* strValue;
		str = "xml version='1.0' encoding='utf-8'";
		strValue = doc.allocate_string(str.c_str());
		rapidxml::xml_node<>* rot = doc.allocate_node(rapidxml::node_pi, strValue);
		doc.append_node(rot);

		// root node
		rapidxml::xml_node<>* MaterialRoot = doc.allocate_node(rapidxml::node_element, "MaterialRoot", NULL);
		doc.append_node(MaterialRoot);

		// 保存材质模板
		{
			xml_node<>* matTemplate = doc.allocate_node(node_element, "MaterialTemplate");
			xml_attribute<>* matTemplateName = doc.allocate_attribute("name", m_materialTemplate.c_str());
			xml_attribute<>* matTemplateStage = doc.allocate_attribute("stage", m_stage.c_str());
			xml_attribute<>* matTemplateSubmitToStage = doc.allocate_attribute("submit_to_stage", m_isSubmitToStageRenderQueue ? "true" : "false");
			xml_attribute<>* matTemplateMacros = doc.allocate_attribute("macros", doc.allocate_string(StringUtil::ToString(m_macros, ";").c_str()));
			matTemplate->append_attribute(matTemplateName);
			matTemplate->append_attribute(matTemplateStage);
			matTemplate->append_attribute(matTemplateSubmitToStage);
			matTemplate->append_attribute(matTemplateMacros);
			MaterialRoot->append_node(matTemplate);
		}

		{
			if (m_blendState)
			{
				BlendState::BlendDesc defaultDesc;
				const BlendState::BlendDesc& desc = m_blendState->getDesc();
				rapidxml::xml_node<>* renderState = doc.allocate_node(rapidxml::node_element, "BlendState", NULL);

				if (desc.bBlendEnable != defaultDesc.bBlendEnable)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BlendEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", (desc.bBlendEnable ? "true" : "false")));
					renderState->append_node(property);
				}

				if (desc.blendOP != defaultDesc.blendOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BlendOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendOperation[desc.blendOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.srcBlend != defaultDesc.srcBlend)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "SrcBlend", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendFactor[desc.srcBlend].c_str()));
					renderState->append_node(property);
				}

				if (desc.dstBlend != defaultDesc.dstBlend)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DstBlend", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendFactor[desc.dstBlend].c_str()));
					renderState->append_node(property);
				}

				if (desc.alphaBlendOP != defaultDesc.alphaBlendOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "AlphaBlendOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendOperation[desc.alphaBlendOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.srcAlphaBlend != defaultDesc.srcAlphaBlend)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "SrcAlphaBlend", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendFactor[desc.srcAlphaBlend].c_str()));
					renderState->append_node(property);
				}

				if (desc.dstAlphaBlend != defaultDesc.dstAlphaBlend)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DstAlphaBlend", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_BlendFactor[desc.dstAlphaBlend].c_str()));
					renderState->append_node(property);
				}

				if (desc.colorWriteMask != defaultDesc.colorWriteMask)
				{
					// TODO: modify
					i32 colorMask[6] = { 1, 2, 4, 8, 7, 15 };
					i32 colorIndex = 0;
					for (i32 i = 0; i < 6; ++i)
					{
						if (desc.colorWriteMask == colorMask[i])
						{
							colorIndex = i;
							break;
						}
					}
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "ColorWriteMask", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_ColorMask[colorIndex].c_str()));
					renderState->append_node(property);
				}

				if (desc.bA2CEnable != defaultDesc.bA2CEnable)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "A2CEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", (desc.bA2CEnable ? "true" : "false")));
					renderState->append_node(property);
				}

				if (desc.bIndependBlendEnable != defaultDesc.bIndependBlendEnable)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "IndependBlendEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", (desc.bIndependBlendEnable ? "true" : "false")));
					renderState->append_node(property);
				}

				if (desc.blendFactor != defaultDesc.blendFactor)
				{
					String color = StringUtil::ToString(desc.blendFactor);
					strValue = doc.allocate_string(color.c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BlendFactor", NULL);
					property->append_attribute(doc.allocate_attribute("value", strValue));
					renderState->append_node(property);
				}

				MaterialRoot->append_node(renderState);
			}

			if (m_rasterizerState)
			{
				RasterizerState::RasterizerDesc defaultDesc;
				const RasterizerState::RasterizerDesc& desc = m_rasterizerState->getDesc();
				rapidxml::xml_node<>* renderState = doc.allocate_node(rapidxml::node_element, "RasterizerState", NULL);

				if (desc.polygonMode != defaultDesc.polygonMode)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "PolygonMode", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_PolygonMode[desc.polygonMode].c_str()));
					renderState->append_node(property);
				}

				if (desc.shadeModel != defaultDesc.shadeModel)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "ShadeModel", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_ShadeModel[desc.shadeModel].c_str()));
					renderState->append_node(property);
				}

				if (desc.cullMode != defaultDesc.cullMode)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "CullMode", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_CullMode[desc.cullMode].c_str()));
					renderState->append_node(property);
				}

				if (desc.bFrontFaceCCW != defaultDesc.bFrontFaceCCW)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontFaceCCW", NULL);
					property->append_attribute(doc.allocate_attribute("value", "true"));
					renderState->append_node(property);
				}

				if (desc.depthBias != defaultDesc.depthBias)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.depthBias).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DepthBias", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.depthBiasFactor != defaultDesc.depthBiasFactor)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.depthBiasFactor).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DepthBiasFactor", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.bDepthClip != defaultDesc.bDepthClip)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DepthClip", NULL);
					property->append_attribute(doc.allocate_attribute("value", "false"));
					renderState->append_node(property);
				}

				if (desc.bScissor != defaultDesc.bScissor)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "Scissor", NULL);
					property->append_attribute(doc.allocate_attribute("value", "true"));
					renderState->append_node(property);
				}

				if (desc.bMultisample != defaultDesc.bMultisample)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "Multisample", NULL);
					property->append_attribute(doc.allocate_attribute("value", "true"));
					renderState->append_node(property);
				}

				MaterialRoot->append_node(renderState);
			}

			if (m_depthStencil)
			{
				DepthStencilState::DepthStencilDesc defaultDesc;
				const DepthStencilState::DepthStencilDesc& desc = m_depthStencil->getDesc();
				rapidxml::xml_node<>* renderState = doc.allocate_node(rapidxml::node_element, "DepthStencilState", NULL);

				if (desc.bDepthEnable != defaultDesc.bDepthEnable)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DepthEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", desc.bDepthEnable ? "true" : "false"));
					renderState->append_node(property);
				}

				if (desc.bWriteDepth != defaultDesc.bWriteDepth)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "WriteDepth", NULL);
					property->append_attribute(doc.allocate_attribute("value", desc.bWriteDepth ? "true" : "false"));
					renderState->append_node(property);
				}

				if (desc.depthFunc != defaultDesc.depthFunc)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "DepthFunc", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_ComparisonFunc[desc.depthFunc].c_str()));
					renderState->append_node(property);
				}

				if (desc.bFrontStencilEnable != defaultDesc.bFrontStencilEnable)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", desc.bFrontStencilEnable ? "true" : "false"));
					renderState->append_node(property);
				}

				if (desc.frontStencilFunc != defaultDesc.frontStencilFunc)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilFunc", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_ComparisonFunc[desc.frontStencilFunc].c_str()));
					renderState->append_node(property);
				}

				if (desc.frontStencilReadMask != defaultDesc.frontStencilReadMask)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.frontStencilReadMask).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilReadMask", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.frontStencilWriteMask != defaultDesc.frontStencilWriteMask)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.frontStencilWriteMask).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilWriteMask", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.frontStencilFailOP != defaultDesc.frontStencilFailOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilFailOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.frontStencilFailOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.frontStencilDepthFailOP != defaultDesc.frontStencilDepthFailOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilDepthFailOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.frontStencilDepthFailOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.frontStencilPassOP != defaultDesc.frontStencilPassOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilPassOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.frontStencilPassOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.frontStencilRef != defaultDesc.frontStencilRef)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.frontStencilRef).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "FrontStencilRef", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.bBackStencilEnable != defaultDesc.bBackStencilEnable)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.bBackStencilEnable).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilEnable", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.backStencilFunc != defaultDesc.backStencilFunc)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilFunc", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_ComparisonFunc[desc.backStencilFunc].c_str()));
					renderState->append_node(property);
				}

				if (desc.backStencilReadMask != defaultDesc.backStencilReadMask)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.backStencilReadMask).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilReadMask", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.backStencilWriteMask != defaultDesc.backStencilWriteMask)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.backStencilWriteMask).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilWriteMask", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				if (desc.backStencilFailOP != defaultDesc.backStencilFailOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilFailOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.backStencilFailOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.backStencilDepthFailOP != defaultDesc.backStencilDepthFailOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilDepthFailOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.backStencilDepthFailOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.backStencilPassOP != defaultDesc.backStencilPassOP)
				{
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilPassOP", NULL);
					property->append_attribute(doc.allocate_attribute("value", s_StencilOperation[desc.backStencilPassOP].c_str()));
					renderState->append_node(property);
				}

				if (desc.backStencilRef != defaultDesc.backStencilRef)
				{
					char* str = doc.allocate_string(StringUtil::ToString(desc.backStencilRef).c_str());
					xml_node<>* property = doc.allocate_node(rapidxml::node_element, "BackStencilRef", NULL);
					property->append_attribute(doc.allocate_attribute("value", str));
					renderState->append_node(property);
				}

				MaterialRoot->append_node(renderState);
			}
		}

		rapidxml::xml_node<>* Uniform = doc.allocate_node(rapidxml::node_element, "Uniform", NULL);
		for (const auto& item : m_unifromParamSet)
		{
			xml_node<>* Property = doc.allocate_node(rapidxml::node_element, "Property", NULL);
			str = item.second->name;
			strValue = doc.allocate_string(str.c_str());
			Property->append_attribute(doc.allocate_attribute("name", strValue));
			str = (ShaderParamType2S(item.second->type));
			strValue = doc.allocate_string(str.c_str());
			Property->append_attribute(doc.allocate_attribute("type", strValue));
			str = StringUtil::ToString(item.second->count);
			strValue = doc.allocate_string(str.c_str());
			Property->append_attribute(doc.allocate_attribute("count", strValue));
			void2s(item.second, str);
			strValue = doc.allocate_string(str.c_str());
			Property->append_attribute(doc.allocate_attribute("value", strValue));
			Uniform->append_node(Property);
		}
		MaterialRoot->append_node(Uniform);

		std::ofstream out(name.c_str());
		out << doc;
	}

	// 复制材质实例
	void MaterialInst::cloneFromTemplate(MaterialInst* _template)
	{
		// 拷贝名称，材质
		m_name = _template->m_name;
		m_stage = _template->m_stage;
		m_macros = _template->m_macros;
		m_macrosEx = _template->m_macrosEx;
		m_materialTemplate = _template->m_materialTemplate;
		m_material = _template->m_material;
		m_isHaveCustomRasterizer = _template->m_isHaveCustomRasterizer;
		m_isHaveCustomBlend = _template->m_isHaveCustomBlend;
		m_isHaveCustomDepthStencil = _template->m_isHaveCustomDepthStencil;
		m_blendDesc = _template->m_blendDesc;
		m_rasterizerStateDesc = _template->m_rasterizerStateDesc;
		m_depthStencilDesc = _template->m_depthStencilDesc;
		m_isSubmitToStageRenderQueue = _template->m_isSubmitToStageRenderQueue;

#ifdef ECHO_EDITOR_MODE
		if (_template->m_blendState)
		{
			m_isHaveCustomBlend = true;
			m_blendDesc = _template->m_blendState->getDesc();
		}
		if (_template->m_rasterizerState)
		{
			m_isHaveCustomRasterizer = true;
			m_rasterizerStateDesc = _template->m_rasterizerState->getDesc();
		}
		if (_template->m_depthStencil)
		{
			m_isHaveCustomDepthStencil = true;
			m_depthStencilDesc = _template->m_depthStencil->getDesc();
		}
#endif // ECHO_EDITOR_MODE

		for (const auto& item : _template->m_unifromParamSetFromFile)
		{
			uniform* pUniform = EchoNew(uniform);
			pUniform->name = item.second->name;
			pUniform->type = item.second->type;
			pUniform->count = item.second->count;
			if (item.second->value)
				pUniform->value = cloneVoid2Value(pUniform->type, item.second->value, pUniform->count);
			else
				pUniform->value = NULL;
			m_unifromParamSetFromFile[pUniform->name] = pUniform;
		}

		for (const auto& item : _template->m_unifromParamSet)
		{
			uniform* pUniform = EchoNew(uniform);
			pUniform->name = item.second->name;
			pUniform->type = item.second->type;
			pUniform->count = item.second->count;
			if (item.second->value)
				pUniform->value = cloneVoid2Value(pUniform->type, item.second->value, pUniform->count);
			else
				pUniform->value = NULL;
			m_unifromParamSet[pUniform->name] = pUniform;
		}

		m_TexturesName = _template->m_TexturesName;
	}

	// 参数继承
	void MaterialInst::deriveUniforms(MaterialInst* from)
	{
		// 继承参数
		for (auto& item : m_unifromParamSetFromFile)
		{
			ParamMap::iterator fromItem = from->m_unifromParamSet.find(item.first);
			if (fromItem != from->m_unifromParamSet.end() && item.second->type == fromItem->second->type)
			{
				delVoid2Value((item.second)->type, (item.second)->value, (item.second)->count);
				EchoSafeDelete(item.second, uniform);
				item.second = fromItem->second->clone();

				// 拷贝纹理名
				if (item.second->type == SPT_TEXTURE)
				{
					int idx = *((int*)item.second->value);
					m_TexturesName[idx] = from->m_TexturesName[idx];
				}
			}
		}
	}

	Echo::ShaderParamType MaterialInst::S2ShaderParamType(const String& value)
	{
		if (value == "SPT_INT")	return SPT_INT;
		else if (value == "SPT_FLOAT")	return SPT_FLOAT;
		else if (value == "SPT_VEC2")	return SPT_VEC2;
		else if (value == "SPT_VEC3")	return SPT_VEC3;
		else if (value == "SPT_VEC4")	return SPT_VEC4;
		else if (value == "SPT_MAT4")	return SPT_MAT4;
		else if (value == "SPT_TEXTURE")return SPT_TEXTURE;
		else							return SPT_UNKNOWN;
	}

	String MaterialInst::ShaderParamType2S(const ShaderParamType& type) const
	{
		switch (type)
		{
			case SPT_INT:		return "SPT_INT";
			case SPT_FLOAT:		return "SPT_FLOAT";
			case SPT_VEC2:		return "SPT_VEC2";
			case SPT_VEC3:		return "SPT_VEC3";
			case SPT_VEC4:		return "SPT_VEC4";
			case SPT_MAT4:		return "SPT_MAT4";;
			case SPT_TEXTURE:	return "SPT_TEXTURE";
			default:			return "SPT_UNKNOWN";
		}
	}


	//是否是全局变量，全局变量在model里面赋值
	bool MaterialInst::isGlobalUniform(const String& name)
	{
		auto begin = std::begin(globalUnfiorms);
		auto end = std::end(globalUnfiorms);
		auto it = std::find_if(begin, end, [&name](const char* s) { return !name.compare(s); });
		return it != end;
	}

	void MaterialInst::S2Void(const ShaderParamType& type, const String& value, void* dstValue, const int count /* = 1 */)
	{
		switch (type)
		{
			case SPT_INT:
			{
				if (count > 1)
				{
					StringArray valueStr = StringUtil::Split(value, ";");
					ui32 size = valueStr.size();
					for (ui32 i = 0; i < size; ++i)
					{
						int& curValue = *(int*)((Byte*)dstValue + i * sizeof(int));
						curValue = StringUtil::ParseInt(valueStr[i]);
					}
				}
				else
				{
					*(int*)dstValue = StringUtil::ParseInt(value);
				}
			}break;
			case SPT_FLOAT:
			{
				if (count > 1)
				{
					StringArray valueStr = StringUtil::Split(value, ";");
					ui32 size = valueStr.size();
					for (ui32 i = 0; i < size; ++i)
					{
						float& curValue = *(float*)((Byte*)dstValue + i * sizeof(float));
						curValue = StringUtil::ParseFloat(valueStr[i]);
					}
				}
				else
				{
					*(float*)dstValue = StringUtil::ParseFloat(value);
				}
			}break;
			case SPT_VEC2:
			{
				if (count > 1)
				{
					StringArray valueStr = StringUtil::Split(value, ";");
					ui32 size = valueStr.size();
					for (ui32 i = 0; i < size; ++i)
					{
						Vector2& curValue = *(Vector2*)((Byte*)dstValue + i * 2 * sizeof(Real));
						curValue = StringUtil::ParseVec2(valueStr[i]);
					}
				}
				else
				{
					*(Vector2*)dstValue = StringUtil::ParseVec2(value);
				}
			}break;
			case SPT_VEC3:
			{
				if (count > 1)
				{
					StringArray valueStr = StringUtil::Split(value, ";");
					ui32 size = valueStr.size();
					for (ui32 i = 0; i < size; ++i)
					{
						Vector3& curValue = *(Vector3*)((Byte*)dstValue + i * 3 * sizeof(Real));
						curValue = StringUtil::ParseVec3(valueStr[i]);
					}
				}
				else
				{
					*(Vector3*)dstValue = StringUtil::ParseVec3(value);
				}

			}; break;
			case SPT_VEC4:
			{
				if (count > 1)
				{
					StringArray valueStr = StringUtil::Split(value, ";");
					ui32 size = valueStr.size();
					for (ui32 i = 0; i < size; ++i)
					{
						Vector4& curValue = *(Vector4*)((Byte*)dstValue + i * 4 * sizeof(Real));
						curValue = StringUtil::ParseVec4(valueStr[i]);
					}
				}
				else
				{
					*(Vector4*)dstValue = StringUtil::ParseVec4(value);
				}

			}; break;
			case SPT_MAT4:
			{
				//目前矩阵应该都是全局变量
			}; break;
			case SPT_TEXTURE:
			{
				*(int*)dstValue = m_TextureCount++;
				m_TexturesName[*(int*)dstValue] = value;
			}; break;
			case SPT_UNKNOWN:
			default:
			break;
		}
	}

	void MaterialInst::delVoid2Value(const ShaderParamType& type, void* value, const int count /* = 1 */)
	{
		if (value)
		{
			EchoSafeFree(value);
		}
	}

	void* MaterialInst::cloneVoid2Value(const ShaderParamType& type, void* value, const int count /* = 1 */)
	{
		// 计算所需内存大小
		int bytes = 0;
		switch (type)
		{
		case SPT_INT:   bytes = sizeof(int)*count;		break;
		case SPT_FLOAT: bytes = sizeof(real32)*count;	break;
		case SPT_VEC2:	bytes = sizeof(Vector2)*count;	break;
		case SPT_VEC3:	bytes = sizeof(Vector3)*count;	break;
		case SPT_VEC4:	bytes = sizeof(Vector4)*count;	break;
		case SPT_MAT4:	bytes = 0;						break;
		case SPT_TEXTURE: bytes = sizeof(int)*count;	break;
		default:		bytes = 0;						break;
		}

		// 分配内存
		if (bytes)
		{
			void* result = EchoMalloc(bytes);
			memcpy(result, value, bytes);

			return result;
		}

		return nullptr;
	}

	// 获取变量值
	void* MaterialInst::getUniformValue(const String& name)
	{
		const auto& it = m_unifromParamSet.find(name);
		if (it != m_unifromParamSet.end())
		{
			return  it->second->value;
		}

		const Material::DefaultUniform* dUniform = m_material->getDefaultUniformValue(name);
		return dUniform ? dUniform->value : NULL;
	}

	// 修改变量的值
	void MaterialInst::modifyUniformValue(const String& name, void* value)
	{
		const ParamMap::iterator iter = m_unifromParamSet.find(name);
		if (iter != m_unifromParamSet.end())
		{
			CopyUniformValue(iter->second->value, iter->second->type, value);
		}
		else
		{
			EchoLogError("MaterialInstance::modifyUniformValue ERROR, Can't find uniform [%s] in [%s]", name.c_str(), m_name.c_str());
		}
	}

	// 准备资源IO
	TextureRes* MaterialInst::prepareTextureImp(const String& texName)
	{
		TextureRes* pTexture;
		size_t cubePos = texName.find("_cube_");
		if (texName.find("_cube_") == String::npos)
		{
			pTexture = TextureResManager::instance()->createTexture(texName.empty() ? "OFFICAL_MATERTAL_TEMPLATE.tga" : texName, Texture::TU_STATIC);
		}
		else
		{
			String cubeTexNamePre = texName.substr(0, cubePos + 6);
			String cubeTexNamePost = texName.substr(cubePos + 7);
			array<String, 6> texNames;
			for (size_t i = 0; i < 6; ++i)
			{
				texNames[i] = StringUtil::Format("%s%d%s", cubeTexNamePre.c_str(), i, cubeTexNamePost.c_str());
			}
			pTexture = TextureResManager::instance()->createTextureCubeFromFiles(texNames[0], texNames[1], texNames[2], texNames[3], texNames[4], texNames[5]);
		}

		pTexture->prepare();

		return pTexture;
	}

	// 资源加载线程准备纹理
	void MaterialInst::prepareTexture()
	{
		TextureRes* pTexture;
		for (const auto& element : m_TexturesName)
		{
			pTexture = prepareTextureImp(element.second);
			m_textures[element.first] = pTexture;
		}
	}

	void MaterialInst::loadTexture()
	{
		for (auto& it : m_textures)
		{
			TextureRes* texRes = it.second;

			// 若纹理状态尚未Prepare, 准备之
			if (texRes->getLoadingState() == Resource::LOADSTATE_CREATED)
			{
				EchoLogError("void MaterialInstance::loadTexture()");
				texRes->prepare();
			}

			// 加载
			texRes->load();
		}
	}

	void MaterialInst::unloadTexture()
	{
		TextureMapItor it = m_textures.begin();
		for (; it != m_textures.end(); ++it)
		{
			if (it->second)
				TextureResManager::instance()->releaseResource(it->second);
		}
		m_textures.clear();
	}

	// 根据索引获取纹理
	TextureRes* MaterialInst::getTexture(const int& index)
	{
		TextureMapItor it = m_textures.find(index);
		if (it != m_textures.end())
		{
			return it->second;
		}

		return nullptr;
	}

	// 设置宏定义
	void MaterialInst::setMacros(const String& macros) 
	{ 
		m_macros = StringUtil::Split(macros, ";");
		std::sort(m_macros.begin(), m_macros.end());
	}

	void MaterialInst::AddUniformParam(uniform* param)
	{
		if (param)
			m_unifromParamSet[param->name] = param;
	}

	// 判断变量是否存在
	bool MaterialInst::isUniformExist(const String& name)
	{
		return m_unifromParamSet.find(name)!=m_unifromParamSet.end();
	}

	// 修改变量
	void MaterialInst::ModifyUniformParam(const String& name, const ShaderParamType& type, void* value)
	{
		const auto& it = m_unifromParamSet.find(name);
		if (it != m_unifromParamSet.end())
		{
			if (it->second->type == type)
				CopyUniformValue(it->second->value, type, value);
			else
				EchoLogError("MaterialInstance::ModifyUnifromParam Type Error!");
		}
	}

	void* MaterialInst::GetuniformValue(const String& name, ShaderParamType type)
	{
		if (type == SPT_TEXTURE)
		{
			void* index = getUniformValue(name);
			if (index)
			{
				return &m_TexturesName[*(int*)index];
			}
			return NULL;
		}

		return getUniformValue(name);
	}

	void MaterialInst::AddTextureName(int idex, const String& name)
	{
		m_TexturesName[idex] = name;
	}

	TextureRes* MaterialInst::SetTexture(int idex, const String& name)
	{
		if (name.empty())
		{
			return NULL;
		}

		TextureMapItor it = m_textures.find(idex);
		if (it != m_textures.end())
		{
			TextureResManager::instance()->releaseResource(it->second);
			m_textures.erase(it);
		}

		TextureRes* pTexture = prepareTextureImp(name);
		pTexture->load();
		m_textures[idex] = pTexture;
		m_TexturesName[idex] = name;
		return pTexture;
	}

	// 获取变量
	MaterialInst::uniform* MaterialInst::GetUniform(const String& name)
	{
		const auto& it = m_unifromParamSet.find(name);
		if (it != m_unifromParamSet.end())
			return it->second;

		return NULL;
	}

	void MaterialInst::void2s(uniform* param, String& value)
	{
		ShaderParamType type = param->type;
		switch (type)
		{
			case SPT_INT:
			{
				if (param->count > 1)
				{
					value = "";
					for (int i = 0; i < param->count; ++i)
					{
						value += StringUtil::ToString(*(int*)(((Byte*)param->value + i * sizeof(int))));
						value += ";";
					}
				}
				{
					value = StringUtil::ToString(*(int*)param->value);
				}

			}break;
			case SPT_FLOAT:
			{
				if (param->count > 1)
				{
					value = "";
					for (int i = 0; i < param->count; ++i)
					{
						value += StringUtil::ToString(*(float*)(((Byte*)param->value + i * sizeof(float))));
						value += ";";
					}
				}
				else
				{
					value = StringUtil::ToString(*(float*)param->value);
				}
			}break;
			case SPT_VEC2:
			{
				if (param->count > 1)
				{
					value = "";
					for (int i = 0; i < param->count; ++i)
					{
						value += StringUtil::ToString(*(Vector2*)(((Byte*)param->value + i * 2 * sizeof(Real))));
						value += ";";
					}
				}
				else
				{
					value = StringUtil::ToString(*(Vector2*)param->value);
				}

			}break;
			case SPT_VEC3:
			{
				if (param->count > 1)
				{
					value = "";
					for (int i = 0; i < param->count; ++i)
					{
						value += StringUtil::ToString(*(Vector3*)(((Byte*)param->value + i * 3 * sizeof(Real))));
						value += ";";
					}
				}
				else
				{
					value = StringUtil::ToString(*(Vector3*)param->value);
				}

			}; break;
			case SPT_VEC4:
			{
				if (param->count > 1)
				{
					value = "";
					for (int i = 0; i < param->count; ++i)
					{
						value += StringUtil::ToString(*(Vector4*)(((Byte*)param->value + i * 4 * sizeof(Real))));
						value += ";";
					}
				}
				else
				{
					value = StringUtil::ToString(*(Vector4*)param->value);
				}
			}; break;
			case SPT_MAT4:
			{
				//目前矩阵应该都是全局变量
			}; break;
			case SPT_TEXTURE:
			{
				value = m_TexturesName[*(int*)param->value];
			}; break;
			case SPT_UNKNOWN:
			default:
			break;
		}
	}

	// 每帧更新
	void MaterialInst::update(ui32 delta)
	{
	}

	void* MaterialInst::createValue2Void(const ShaderParamType& type, const int count /* = 1 */)
	{
		void* value = NULL;
		switch (type)
		{
			case Echo::SPT_INT:
			{
				int* _value = count > 1 ? EchoAlloc(int, count) : EchoNew(int);
				*_value = 1;
				value = _value;
			}break;
			case Echo::SPT_FLOAT:
			{
				float* _value = count > 1 ? EchoAlloc(float, count) : EchoNew(float);
				*_value = 1.0f;
				value = _value;
			}break;
			case Echo::SPT_VEC2:
			{
				Echo::Vector2* _value = count > 1 ? EchoNewArray(Echo::Vector2, count) : EchoNew(Echo::Vector2);
				*_value = Echo::Vector2::ONE;
				value = _value;
			}break;
			case Echo::SPT_VEC3:
			{
				Echo::Vector3* _value = count > 1 ? EchoNewArray(Echo::Vector3, count) : EchoNew(Echo::Vector3);
				*_value = Echo::Vector3::ONE;
				value = _value;
			}; break;
			case Echo::SPT_VEC4:
			{
				Echo::Vector4* _value = count > 1 ? EchoNewArray(Echo::Vector4, count) : EchoNew(Echo::Vector4);
				*_value = Echo::Vector4::ONE;
				value = _value;
			}; break;
			case Echo::SPT_MAT4:
			{
				//目前矩阵应该都是全局变量
			}; break;
			case Echo::SPT_TEXTURE:
			{
				int* _value = EchoNew(int);
				*_value = 1;
				value = _value;
			}; break;
			case Echo::SPT_UNKNOWN:
			default:
			break;
		}

		return value;
	}

	void MaterialInst::CopyUniformValue(void* dstValue, const ShaderParamType& type, void* srcValue)
	{
		switch (type)
		{
			case SPT_INT:
			{
				*(int*)(dstValue) = *(int*)(srcValue);
			}break;
			case SPT_FLOAT:
			{
				*(float*)(dstValue) = *(float*)(srcValue);
			}break;
			case SPT_VEC2:
			{
				*(Vector2*)(dstValue) = *(Vector2*)(srcValue);
			}break;
			case SPT_VEC3:
			{
				*(Vector3*)(dstValue) = *(Vector3*)(srcValue);
			}; break;
			case SPT_VEC4:
			{
				*(Vector4*)(dstValue) = *(Vector4*)(srcValue);
			}; break;
			case SPT_MAT4:
			{
				//目前矩阵应该都是全局变量
			}; break;
			case SPT_TEXTURE:
			{
				*(int*)(dstValue) = *(int*)(srcValue);
			}; break;
			case SPT_UNKNOWN:
			default:
			break;
		}
	}

	// 是否使用了宏定义
	bool MaterialInst::isMacroUsed(const String& macro)
	{
		for (const String& _macro : m_macros)
		{
			if (macro == _macro)
				return true;
		}

		for (const String& macroEx : m_macrosEx)
		{
			if (macro == macroEx)
				return true;
		}

		return false;
	}

#ifdef ECHO_EDITOR_MODE
	// 设置宏定义
	void MaterialInst::setMacro(const String& macro, bool enabled)
	{
		if (enabled)
		{
			if (!isMacroUsed(macro))
			{
				m_macros.push_back(macro);
			}
		}
		else
		{
			if (isMacroUsed(macro))
			{
				m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), macro), m_macros.end());
			}
			else
			{
				ui32 size = m_macros.size();
				for (ui32 i = 0; i < size; ++i)
				{
					if (StringUtil::StartWith(m_macros[i], macro))
					{
						m_macros.erase(std::remove(m_macros.begin(), m_macros.end(), m_macros[i]), m_macros.end());
						break;
					}
				}
			}
		}
		std::sort(m_macros.begin(), m_macros.end());
		refresh();
	}

	void MaterialInst::setPBRLight(const vector<PBRLight*>::type& lights)
	{
		String macros = StringUtil::Format("LIGHT_COUNT %d", lights.size());
		if (lights.size() > 0)
		{
			setMacro("LIGHT_COUNT", false);
			setMacro(macros, true);
		}
		else
		{
			setMacro("LIGHT_COUNT", false);
		}
	}

	bool MaterialInst::isUsingSceneEnvMap() const
	{
		return std::find(m_macros.begin(), m_macros.end(), "USE_SCENE_ENV_MAP") != m_macros.end();
	}

	void MaterialInst::refresh()
	{
		buildRenderQueue();
		matchUniforms();
	}

#endif // ECHO_EDITOR_MODE

	// 构建渲染队列
	void MaterialInst::buildRenderQueue()
	{
		// confirm render queue
		m_renderQueue = RenderQueueGroup::instance()->getRenderQueue(m_stage);

		// make sure macros
		String finalMacros; finalMacros.reserve(512);
		for (const String& macro : m_macros)
		{
			finalMacros += "#define " + macro + "\n";
		}

		for (const String& macro : m_macrosEx)
		{
			finalMacros += "#define " + macro + "\n";
		}

		// create material
		m_material = EchoNew(Material);
		if (m_officialMaterialContent)
			m_material->loadFromContent(m_officialMaterialContent, finalMacros);
		else if (!m_materialTemplate.empty())
			m_material->loadFromFile( m_materialTemplate, finalMacros);
	}

	void MaterialInst::LoadBlendState(void* pNode)
	{
		m_blendDesc.reset();

		rapidxml::xml_node<>* pSubNode = static_cast<xml_node<>*>(pNode);
		rapidxml::xml_node<>* pElementNode = pSubNode->first_node();
		while (pElementNode)
		{
			m_isHaveCustomBlend = true;
			String strName(pElementNode->name());
			if (strName == "BlendEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_blendDesc.bBlendEnable = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "SrcBlend")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < BlendState::BF_MAX; ++i)
				{
					if (val == s_BlendFactor[i])
					{
						m_blendDesc.srcBlend = (BlendState::BlendFactor)i;
						break;
					}
				}
			}
			else if (strName == "DstBlend")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < BlendState::BF_MAX; ++i)
				{
					if (val == s_BlendFactor[i])
					{
						m_blendDesc.dstBlend = (BlendState::BlendFactor)i;
						break;
					}
				}
			}
			else if (strName == "BlendOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 6; ++i)
				{
					if (val == s_BlendOperation[i])
					{
						m_blendDesc.blendOP = (BlendState::BlendOperation)i;
						break;
					}
				}
			}
			else if (strName == "SrcAlphaBlend")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 6; ++i)
				{
					if (val == s_BlendFactor[i])
					{
						m_blendDesc.srcAlphaBlend = (BlendState::BlendFactor)i;
						break;
					}
				}
			}
			else if (strName == "DstAlphaBlend")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 6; ++i)
				{
					if (val == s_BlendFactor[i])
					{
						m_blendDesc.dstAlphaBlend = (BlendState::BlendFactor)i;
						break;
					}
				}
			}
			else if (strName == "AlphaBlendOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 6; ++i)
				{
					if (val == s_BlendOperation[i])
					{
						m_blendDesc.alphaBlendOP = (BlendState::BlendOperation)i;
						break;
					}
				}
			}
			else if (strName == "ColorWriteMask")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				if (val == s_ColorMask[0])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_RED;
				}
				else if (val == s_ColorMask[1])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_GREEN;
				}
				else if (val == s_ColorMask[2])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_BLUE;
				}
				else if (val == s_ColorMask[3])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_ALPHA;
				}
				else if (val == s_ColorMask[4])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_COLOR;
				}
				else if (val == s_ColorMask[5])
				{
					m_blendDesc.colorWriteMask = RenderState::CMASK_ALL;
				}
			}
			else if (strName == "A2CEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_blendDesc.bA2CEnable = StringUtil::ParseBool(pVarNode->value());
			}
			else if (strName == "IndependBlendEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_blendDesc.bIndependBlendEnable = StringUtil::ParseBool(pVarNode->value());
			}
			else if (strName == "BlendFactor")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_blendDesc.blendFactor = StringUtil::ParseColor(pVarNode->value());
			}
			pElementNode = pElementNode->next_sibling();
		} // while
	}

	void MaterialInst::LoadRasterizerState(void* pNode)
	{
		m_rasterizerStateDesc.reset();

		rapidxml::xml_node<>* pSubNode = static_cast<rapidxml::xml_node<>*>(pNode);
		rapidxml::xml_node<>* pElementNode = pSubNode->first_node();
		while (pElementNode)
		{
			m_isHaveCustomRasterizer = true;
			String strName = pElementNode->name();
			if (strName == "PolygonMode")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 3; ++i)
				{
					if (val == s_PolygonMode[i])
					{
						m_rasterizerStateDesc.polygonMode = (RasterizerState::PolygonMode)i;
						break;
					}
				}
			}
			else if (strName == "ShadeModel")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 2; ++i)
				{
					if (val == s_ShadeModel[i])
					{
						m_rasterizerStateDesc.shadeModel = (RasterizerState::ShadeModel)i;
						break;
					}
				}
			}
			else if (strName == "CullMode")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < 2; ++i)
				{
					if (val == s_CullMode[i])
					{
						m_rasterizerStateDesc.cullMode = (RasterizerState::CullMode)i;
						break;
					}
				}
			}
			else if (strName == "FrontFaceCCW")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.bFrontFaceCCW = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "DepthBias")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.depthBias = StringUtil::ParseFloat(String(pVarNode->value()));
			}
			else if (strName == "DepthBiasFactor")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.depthBiasFactor = StringUtil::ParseFloat(String(pVarNode->value()));
			}
			else if (strName == "DepthClip")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.bDepthClip = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "Scissor")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.bScissor = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "Multisample")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_rasterizerStateDesc.bMultisample = StringUtil::ParseBool(String(pVarNode->value()));
			}
			pElementNode = pElementNode->next_sibling();
		} // end while
	}

	void MaterialInst::LoadDepthStencilState(void* pNode)
	{
		m_depthStencilDesc.reset();

		rapidxml::xml_node<>* pSubNode = static_cast<rapidxml::xml_node<>*>(pNode);	
		rapidxml::xml_node<>* pElementNode = pSubNode->first_node();
		while (pElementNode)
		{
			m_isHaveCustomDepthStencil = true;
			String strName = pElementNode->name();
			if (strName == "DepthEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.bDepthEnable = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "WriteDepth")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.bWriteDepth = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "DepthFunc")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < RenderState::CF_MAXNUM; ++i)
				{
					if (val == s_ComparisonFunc[i])
					{
						m_depthStencilDesc.depthFunc = (RenderState::ComparisonFunc)i;
						break;
					}
				}
			}
			else if (strName == "FrontStencilEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.bFrontStencilEnable = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "FrontStencilFunc")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < RenderState::CF_MAXNUM; ++i)
				{
					if (val == s_ComparisonFunc[i])
					{
						m_depthStencilDesc.frontStencilFunc = (RenderState::ComparisonFunc)i;
						break;
					}
				}
			}
			else if (strName == "FrontStencilReadMask")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.frontStencilReadMask = StringUtil::ParseUI16(String(pVarNode->value()));
			}
			else if (strName == "FrontStencilWriteMask")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.frontStencilWriteMask = StringUtil::ParseUI16(String(pVarNode->value()));
			}
			else if (strName == "FrontStencilFailOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.frontStencilFailOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "FrontStencilDepthFailOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.frontStencilDepthFailOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "FrontStencilPassOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.frontStencilPassOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "FrontStencilRef")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.frontStencilRef = StringUtil::ParseUI32(String(pVarNode->value()));
			}
			else if (strName == "BackStencilEnable")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.bBackStencilEnable = StringUtil::ParseBool(String(pVarNode->value()));
			}
			else if (strName == "BackStencilFunc")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < RenderState::CF_MAXNUM; ++i)
				{
					if (val == s_ComparisonFunc[i])
					{
						m_depthStencilDesc.backStencilFunc = (RenderState::ComparisonFunc)i;
						break;
					}
				}
			}
			else if (strName == "BackStencilReadMask")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.backStencilReadMask = StringUtil::ParseUI16(String(pVarNode->value()));
			}
			else if (strName == "BackStencilWriteMask")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.backStencilWriteMask = StringUtil::ParseI16(String(pVarNode->value()));
			}
			else if (strName == "BackStencilFailOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.backStencilFailOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "BackStencilDepthFailOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.backStencilDepthFailOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "BackStencilPassOP")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				String val(pVarNode->value());
				for (size_t i = 0; i < DepthStencilState::SOP_MAX; ++i)
				{
					if (val == s_StencilOperation[i])
					{
						m_depthStencilDesc.backStencilPassOP = (DepthStencilState::StencilOperation)i;
						break;
					}
				}
			}
			else if (strName == "BackStencilRef")
			{
				rapidxml::xml_attribute<>* pVarNode = pElementNode->first_attribute();
				m_depthStencilDesc.backStencilRef = StringUtil::ParseI32(String(pVarNode->value()));
			}
			pElementNode = pElementNode->next_sibling();
		}
	}

	// 参数匹配
	void MaterialInst::matchUniforms()
	{
		ShaderProgram* shaderProgram = m_material->getShaderProgram();
		if (shaderProgram)
		{
			// 添加未设置参数
			ShaderProgram::UniformArray* uniforms = shaderProgram->getUniforms();
			for (ShaderProgram::UniformArray::iterator it = uniforms->begin(); it != uniforms->end(); it++)
			{
				const ShaderProgram::Uniform& suniform = it->second;
				if (!isGlobalUniform(suniform.m_name) && !GetUniform(suniform.m_name))
				{
					uniform* pUnifrom = EchoNew(uniform);
					pUnifrom->name = suniform.m_name;
					pUnifrom->type = suniform.m_type;
					pUnifrom->count = suniform.m_count;

					switch (pUnifrom->type)
					{
						case Echo::SPT_INT:		pUnifrom->value = pUnifrom->count > 1 ? EchoNewArray(int, pUnifrom->count) : EchoNew(int(1));					break;
						case Echo::SPT_FLOAT:	pUnifrom->value = pUnifrom->count > 1 ? EchoNewArray(float, pUnifrom->count) : EchoNew(float(1.f));				break;
						case Echo::SPT_VEC2:	pUnifrom->value = pUnifrom->count > 1 ? EchoNewArray(Vector2, pUnifrom->count) : EchoNew(Vector2(Vector2::ONE));	break;
						case Echo::SPT_VEC3:	pUnifrom->value = pUnifrom->count > 1 ? EchoNewArray(Vector3, pUnifrom->count) : EchoNew(Vector3(Vector3::ONE));	break;
						case Echo::SPT_VEC4:	pUnifrom->value = pUnifrom->count > 1 ? EchoNewArray(Vector4, pUnifrom->count) : EchoNew(Vector4(Vector4::ONE));	break;
						case Echo::SPT_MAT4:	EchoAssert(false);	break;
						case Echo::SPT_TEXTURE:
						{
							pUnifrom->value = EchoNew(int(getTextureNum()));
							AddTextureName(getTextureNum(), "");
						};
						break;

						default:
						break;
					}

					Material* pMaterial = m_material;
					const Material::DefaultUniform* defaultUniform = pMaterial->getDefaultUniformValue(pUnifrom->name);
					if (defaultUniform && pUnifrom->count == defaultUniform->count && pUnifrom->type == defaultUniform->type)
					{
						memcpy(pUnifrom->value, defaultUniform->value, defaultUniform->sizeInByte);
					}

					this->AddUniformParam(pUnifrom);
				}
			}
		}
	}
}