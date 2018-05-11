#include "MaterialInst.h"
#include "Engine/modules/Light/Light.h"
#include "Engine/core/Scene/NodeTree.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/MaterialDesc.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/render/TextureResManager.h"
#include <thirdparty/pugixml/pugixml.hpp>

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

	// release
	void MaterialInst::release()
	{
		ECHO_DELETE_T(this, MaterialInst);
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

			pugi::xml_document doc;
			doc.load(memReader.getData<char*>());

			pugi::xml_node pRootNode = doc.first_child();
			if (!pRootNode)
			{
				EchoLogError("Material file content is valid. there is no root node");
				return false;
			}

			// 材质模板
			pugi::xml_node matTemNode = pRootNode.child("MaterialTemplate");
			if (matTemNode)
			{
				m_materialTemplate = matTemNode.attribute("name").as_string("");
				m_renderStage = matTemNode.attribute("stage").as_string("");

				if (m_materialTemplate == "water.mt" || m_materialTemplate == "water2.mt" || m_materialTemplate == "water1.mt")
				{
					m_renderStage = "SampleWater";
				}

				m_isSubmitToStageRenderQueue = matTemNode.attribute("submit_to_stage").as_bool(false);
				m_macros = StringUtil::Split(matTemNode.attribute("macros").as_string(""), ";");
				std::sort(m_macros.begin(), m_macros.end());
			}
			else
			{
				EchoLogError("Material file content is valid. there is no MaterialTemplate node");
				return false;
			}

			{
				pugi::xml_node pSubNode = pRootNode.child("RasterizerState");
				if (pSubNode)
				{
					LoadRasterizerState(pSubNode);
				}
			}

			{
				pugi::xml_node pSubNode = pRootNode.child("BlendState");
				if (pSubNode)
				{
					LoadBlendState(pSubNode);
				}
			}

			{
				pugi::xml_node pSubNode = pRootNode.child("DepthStencilState");
				if (pSubNode)
				{
					LoadDepthStencilState(pSubNode);
				}
			}

			// 参数模板
			pugi::xml_node uniformNode = pRootNode.child("Uniform");
			if (uniformNode)
			{
				for(pugi::xml_node propertyNode = uniformNode.child("Property"); propertyNode; propertyNode=pRootNode.next_sibling("Property"))
				{
					Echo::String uniformName = propertyNode.attribute("name").as_string("");
					{
						uniform* pProperty = EchoNew(uniform);

						pProperty->name = uniformName;
						pProperty->type = S2ShaderParamType(propertyNode.attribute("type").as_string(""));
						pProperty->count = propertyNode.attribute("count").as_int(1);
						pProperty->value = createValue2Void(pProperty->type, pProperty->count);
						S2Void(pProperty->type, propertyNode.attribute("value").as_string(""), pProperty->value, pProperty->count);

						m_unifromParamSetFromFile[pProperty->name] = pProperty;
					}
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
		pugi::xml_document doc;

		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		// root node
		pugi::xml_node MaterialRoot = doc.append_child( "MaterialRoot");

		// 保存材质模板
		{
			pugi::xml_node matTemplate = doc.append_child("MaterialTemplate");
			matTemplate.append_attribute("name").set_value(m_materialTemplate.c_str());
			matTemplate.append_attribute("stage").set_value(m_renderStage.c_str());
			matTemplate.append_attribute("submit_to_stage").set_value(m_isSubmitToStageRenderQueue ? "true" : "false");
			matTemplate.append_attribute("macros").set_value(StringUtil::ToString(m_macros, ";").c_str());
		}

		{
			if (m_blendState)
			{
				BlendState::BlendDesc defaultDesc;
				const BlendState::BlendDesc& desc = m_blendState->getDesc();
				pugi::xml_node renderState = MaterialRoot.append_child("BlendState");

				if (desc.bBlendEnable != defaultDesc.bBlendEnable)
					renderState.append_attribute("BlendEnable").set_value(desc.bBlendEnable ? "true" : "false");

				if (desc.blendOP != defaultDesc.blendOP)
					renderState.append_attribute("BlendOP").set_value(s_BlendOperation[desc.blendOP].c_str());

				if (desc.srcBlend != defaultDesc.srcBlend)
					renderState.append_attribute("SrcBlend").set_value(s_BlendFactor[desc.srcBlend].c_str());

				if (desc.dstBlend != defaultDesc.dstBlend)
					renderState.append_attribute("DstBlend").set_value(s_BlendFactor[desc.dstBlend].c_str());

				if (desc.alphaBlendOP != defaultDesc.alphaBlendOP)
					renderState.append_attribute("AlphaBlendOP").set_value(s_BlendOperation[desc.alphaBlendOP].c_str());

				if (desc.srcAlphaBlend != defaultDesc.srcAlphaBlend)
					renderState.append_attribute("SrcAlphaBlend").set_value(s_BlendFactor[desc.srcAlphaBlend].c_str());

				if (desc.dstAlphaBlend != defaultDesc.dstAlphaBlend)
					renderState.append_attribute("DstAlphaBlend").set_value(s_BlendFactor[desc.dstAlphaBlend].c_str());

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

					renderState.append_attribute("ColorWriteMask").set_value(s_ColorMask[colorIndex].c_str());
				}

				if (desc.bA2CEnable != defaultDesc.bA2CEnable)
					renderState.append_attribute("A2CEnable").set_value(desc.bA2CEnable ? "true" : "false");

				if (desc.bIndependBlendEnable != defaultDesc.bIndependBlendEnable)
					renderState.append_attribute("IndependBlendEnable").set_value(desc.bIndependBlendEnable ? "true" : "false");

				if (desc.blendFactor != defaultDesc.blendFactor)
					renderState.append_attribute("BlendFactor").set_value(StringUtil::ToString(desc.blendFactor).c_str());
			}

			if (m_rasterizerState)
			{
				RasterizerState::RasterizerDesc defaultDesc;
				const RasterizerState::RasterizerDesc& desc = m_rasterizerState->getDesc();
				pugi::xml_node rasterizerState = MaterialRoot.append_child("RasterizerState");

				if (desc.polygonMode != defaultDesc.polygonMode)
					rasterizerState.append_attribute("PolygonMode").set_value(s_PolygonMode[desc.polygonMode].c_str());

				if (desc.shadeModel != defaultDesc.shadeModel)
					rasterizerState.append_attribute("ShadeModel").set_value(s_ShadeModel[desc.shadeModel].c_str());

				if (desc.cullMode != defaultDesc.cullMode)
					rasterizerState.append_attribute("CullMode").set_value(s_CullMode[desc.cullMode].c_str());

				if (desc.bFrontFaceCCW != defaultDesc.bFrontFaceCCW)
					rasterizerState.append_attribute("FrontFaceCCW").set_value("true");

				if (desc.depthBias != defaultDesc.depthBias)
					rasterizerState.append_attribute("DepthBias").set_value(StringUtil::ToString(desc.depthBias).c_str());

				if (desc.depthBiasFactor != defaultDesc.depthBiasFactor)
					rasterizerState.append_attribute("DepthBiasFactor").set_value(StringUtil::ToString(desc.depthBiasFactor).c_str());

				if (desc.bDepthClip != defaultDesc.bDepthClip)
					rasterizerState.append_attribute("DepthClip").set_value("false");

				if (desc.bScissor != defaultDesc.bScissor)
					rasterizerState.append_attribute("Scissor").set_value("true");

				if (desc.bMultisample != defaultDesc.bMultisample)
					rasterizerState.append_attribute("Multisample").set_value("true");
			}

			if (m_depthStencil)
			{
				DepthStencilState::DepthStencilDesc defaultDesc;
				const DepthStencilState::DepthStencilDesc& desc = m_depthStencil->getDesc();
				pugi::xml_node depthStencilState = MaterialRoot.append_child( "DepthStencilState");

				if (desc.bDepthEnable != defaultDesc.bDepthEnable)
					depthStencilState.append_attribute("DepthEnable").set_value(desc.bDepthEnable ? "true" : "false");

				if (desc.bWriteDepth != defaultDesc.bWriteDepth)
					depthStencilState.append_attribute("WriteDepth").set_value(desc.bWriteDepth ? "true" : "false");

				if (desc.depthFunc != defaultDesc.depthFunc)
					depthStencilState.append_attribute("DepthFunc").set_value(s_ComparisonFunc[desc.depthFunc].c_str());

				if (desc.bFrontStencilEnable != defaultDesc.bFrontStencilEnable)
					depthStencilState.append_attribute("FrontStencilEnable").set_value(desc.bFrontStencilEnable ? "true" : "false");

				if (desc.frontStencilFunc != defaultDesc.frontStencilFunc)
					depthStencilState.append_attribute("FrontStencilFunc").set_value(s_ComparisonFunc[desc.frontStencilFunc].c_str());

				if (desc.frontStencilReadMask != defaultDesc.frontStencilReadMask)
					depthStencilState.append_attribute("FrontStencilReadMask").set_value(StringUtil::ToString(desc.frontStencilReadMask).c_str());

				if (desc.frontStencilWriteMask != defaultDesc.frontStencilWriteMask)
					depthStencilState.append_attribute("FrontStencilWriteMask").set_value(StringUtil::ToString(desc.frontStencilWriteMask).c_str());

				if (desc.frontStencilFailOP != defaultDesc.frontStencilFailOP)
					depthStencilState.append_attribute("FrontStencilFailOP").set_value(s_StencilOperation[desc.frontStencilFailOP].c_str());

				if (desc.frontStencilDepthFailOP != defaultDesc.frontStencilDepthFailOP)
					depthStencilState.append_attribute("FrontStencilDepthFailOP").set_value(s_StencilOperation[desc.frontStencilDepthFailOP].c_str());

				if (desc.frontStencilPassOP != defaultDesc.frontStencilPassOP)
					depthStencilState.append_attribute("FrontStencilPassOP").set_value(s_StencilOperation[desc.frontStencilPassOP].c_str());

				if (desc.frontStencilRef != defaultDesc.frontStencilRef)
					depthStencilState.append_attribute("FrontStencilRef").set_value(StringUtil::ToString(desc.frontStencilRef).c_str());

				if (desc.bBackStencilEnable != defaultDesc.bBackStencilEnable)
					depthStencilState.append_attribute("BackStencilEnable").set_value(StringUtil::ToString(desc.bBackStencilEnable).c_str());

				if (desc.backStencilFunc != defaultDesc.backStencilFunc)
					depthStencilState.append_attribute("BackStencilFunc").set_value(s_ComparisonFunc[desc.backStencilFunc].c_str());

				if (desc.backStencilReadMask != defaultDesc.backStencilReadMask)
					depthStencilState.append_attribute("BackStencilReadMask").set_value(StringUtil::ToString(desc.backStencilReadMask).c_str());

				if (desc.backStencilWriteMask != defaultDesc.backStencilWriteMask)
					depthStencilState.append_attribute("BackStencilWriteMask").set_value(StringUtil::ToString(desc.backStencilWriteMask).c_str());

				if (desc.backStencilFailOP != defaultDesc.backStencilFailOP)
					depthStencilState.append_attribute("BackStencilFailOP").set_value(s_StencilOperation[desc.backStencilFailOP].c_str());

				if (desc.backStencilDepthFailOP != defaultDesc.backStencilDepthFailOP)
					depthStencilState.append_attribute("BackStencilDepthFailOP").set_value(s_StencilOperation[desc.backStencilDepthFailOP].c_str());

				if (desc.backStencilPassOP != defaultDesc.backStencilPassOP)
					depthStencilState.append_attribute("BackStencilPassOP").set_value(s_StencilOperation[desc.backStencilPassOP].c_str());

				if (desc.backStencilRef != defaultDesc.backStencilRef)
					depthStencilState.append_attribute("BackStencilRef").set_value(StringUtil::ToString(desc.backStencilRef).c_str());
			}
		}

		pugi::xml_node Uniform = MaterialRoot.append_child("Uniform");
		for (const auto& item : m_unifromParamSet)
		{
			pugi::xml_node propertyNode = Uniform.append_child("Property");
			propertyNode.append_attribute("name") = item.second->name.c_str();
			propertyNode.append_attribute("type") = ShaderParamType2S(item.second->type).c_str();
			propertyNode.append_attribute("count") = StringUtil::ToString(item.second->count).c_str();

			String str; void2s(item.second, str);
			propertyNode.append_attribute("value") = str.c_str();
		}

		doc.save_file(name.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	// 复制材质实例
	void MaterialInst::cloneFromTemplate(MaterialInst* _template)
	{
		// 拷贝名称，材质
		m_name = _template->m_name;
		m_renderStage = _template->m_renderStage;
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

	TextureRes* MaterialInst::setTexture(int index, const String& name)
	{
		if (name.empty())
		{
			return NULL;
		}

		TextureMapItor it = m_textures.find(index);
		if (it != m_textures.end())
		{
			TextureResManager::instance()->releaseResource(it->second);
			m_textures.erase(it);
		}

		TextureRes* pTexture = prepareTextureImp(name);
		pTexture->load();
		m_textures[index] = pTexture;
		m_TexturesName[index] = name;
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

	static bool MappingStringArrayIdx(const String* arry, int count, const String& value, int& idx)
	{
		for (int i = 0; i < count; i++)
		{
			if (value == arry[i])
			{
				idx = i;
				return true;
			}
		}

		return false;
	}

	void MaterialInst::LoadBlendState(void* pNode)
	{
		m_blendDesc.reset();

		pugi::xml_node* pSubNode = (pugi::xml_node*)(pNode);
		for (pugi::xml_attribute elementAttr = pSubNode->first_attribute(); elementAttr; elementAttr = elementAttr.next_attribute())
		{
			int idx = 0;
			String name = elementAttr.name();
			if (name == "BlendEnable")
				m_blendDesc.bBlendEnable = elementAttr.as_bool( m_blendDesc.bBlendEnable);
			else if (name == "SrcBlend")
			{
				if(MappingStringArrayIdx(s_BlendFactor, BlendState::BF_MAX, elementAttr.as_string(""), idx))
					m_blendDesc.srcBlend = (BlendState::BlendFactor)idx;
			}
			else if (name == "DstBlend")
			{
				if (MappingStringArrayIdx(s_BlendFactor, BlendState::BF_MAX, elementAttr.as_string(""), idx))
					m_blendDesc.dstBlend = (BlendState::BlendFactor)idx;
			}
			else if (name == "BlendOP")
			{
				if (MappingStringArrayIdx(s_BlendOperation, 6, elementAttr.as_string(""), idx))
					m_blendDesc.blendOP = (BlendState::BlendOperation)idx;
			}
			else if (name == "SrcAlphaBlend")
			{
				if (MappingStringArrayIdx(s_BlendFactor, 6, elementAttr.as_string(""), idx))
					m_blendDesc.srcAlphaBlend = (BlendState::BlendFactor)idx;
			}
			else if (name == "DstAlphaBlend")
			{
				if (MappingStringArrayIdx(s_BlendFactor, 6, elementAttr.as_string(""), idx))
					m_blendDesc.dstAlphaBlend = (BlendState::BlendFactor)idx;
			}
			else if (name == "AlphaBlendOP")
			{
				if (MappingStringArrayIdx(s_BlendOperation, 6, elementAttr.as_string(""), idx))
					m_blendDesc.alphaBlendOP = (BlendState::BlendOperation)idx;
			}
			else if (name == "ColorWriteMask")
				m_blendDesc.colorWriteMask = MappingColorMask(elementAttr.as_string(""));
			else if (name == "A2CEnable")
				m_blendDesc.bA2CEnable = elementAttr.as_bool(m_blendDesc.bA2CEnable);
			else if (name == "IndependBlendEnable")
				m_blendDesc.bIndependBlendEnable = elementAttr.as_bool(m_blendDesc.bIndependBlendEnable);
			else if (name == "BlendFactor")
				m_blendDesc.blendFactor = StringUtil::ParseColor(elementAttr.as_string(""));
		}
	}

	void MaterialInst::LoadRasterizerState(void* pNode)
	{
		m_rasterizerStateDesc.reset();

		pugi::xml_node* pSubNode = static_cast<pugi::xml_node*>(pNode);
		for (pugi::xml_attribute elementAttr = pSubNode->first_attribute(); elementAttr; elementAttr = elementAttr.next_attribute())
		{
			int idx = 0;
			m_isHaveCustomRasterizer = true;
			String strName = elementAttr.name();
			if (strName == "PolygonMode")
			{
				if (MappingStringArrayIdx(s_PolygonMode, 3, elementAttr.as_string(""), idx))
					m_rasterizerStateDesc.polygonMode = (RasterizerState::PolygonMode)idx;
			}
			else if (strName == "ShadeModel")
			{
				if (MappingStringArrayIdx(s_ShadeModel, 2, elementAttr.as_string(""), idx))
					m_rasterizerStateDesc.shadeModel = (RasterizerState::ShadeModel)idx;
			}
			else if (strName == "CullMode")
			{
				if (MappingStringArrayIdx(s_CullMode, 2, elementAttr.as_string(""), idx))
					m_rasterizerStateDesc.cullMode = (RasterizerState::CullMode)idx;
			}
			else if (strName == "FrontFaceCCW")
				m_rasterizerStateDesc.bFrontFaceCCW = elementAttr.as_bool(m_rasterizerStateDesc.bFrontFaceCCW);
			else if (strName == "DepthBias")
				m_rasterizerStateDesc.depthBias = elementAttr.as_float(m_rasterizerStateDesc.depthBias);
			else if (strName == "DepthBiasFactor")
				m_rasterizerStateDesc.depthBiasFactor = elementAttr.as_float(m_rasterizerStateDesc.depthBiasFactor);
			else if (strName == "DepthClip")
				m_rasterizerStateDesc.bDepthClip = elementAttr.as_bool(m_rasterizerStateDesc.bDepthClip);
			else if (strName == "Scissor")
				m_rasterizerStateDesc.bScissor = elementAttr.as_bool(m_rasterizerStateDesc.bScissor);
			else if (strName == "Multisample")
				m_rasterizerStateDesc.bMultisample = elementAttr.as_bool(m_rasterizerStateDesc.bMultisample);
		}
	}

	void MaterialInst::LoadDepthStencilState(void* pNode)
	{
		m_depthStencilDesc.reset();

		pugi::xml_node* pSubNode = static_cast<pugi::xml_node*>(pNode);
		for (pugi::xml_attribute elementAttr = pSubNode->first_attribute(); elementAttr; elementAttr = elementAttr.next_attribute())
		{
			int idx = 0;
			m_isHaveCustomDepthStencil = true;
			String strName = elementAttr.name();
			if (strName == "DepthEnable")
				m_depthStencilDesc.bDepthEnable = elementAttr.as_bool(m_depthStencilDesc.bDepthEnable);
			else if (strName == "WriteDepth")
				m_depthStencilDesc.bWriteDepth = elementAttr.as_bool(m_depthStencilDesc.bWriteDepth);
			else if (strName == "DepthFunc")
			{
				if (MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementAttr.as_string(""), idx))
					m_depthStencilDesc.depthFunc = (RenderState::ComparisonFunc)idx;
			}
			else if (strName == "FrontStencilEnable")
				m_depthStencilDesc.bFrontStencilEnable = elementAttr.as_bool();
			else if (strName == "FrontStencilFunc")
			{
				if (MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementAttr.as_string(""), idx))
					m_depthStencilDesc.frontStencilFunc = (RenderState::ComparisonFunc)idx;
			}
			else if (strName == "FrontStencilReadMask")
				m_depthStencilDesc.frontStencilReadMask = elementAttr.as_uint();
			else if (strName == "FrontStencilWriteMask")
				m_depthStencilDesc.frontStencilWriteMask = elementAttr.as_uint();
			else if (strName == "FrontStencilFailOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.frontStencilFailOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "FrontStencilDepthFailOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.frontStencilDepthFailOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "FrontStencilPassOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.frontStencilPassOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "FrontStencilRef")
				m_depthStencilDesc.frontStencilRef = elementAttr.as_uint();
			else if (strName == "BackStencilEnable")
				m_depthStencilDesc.bBackStencilEnable = elementAttr.as_bool();
			else if (strName == "BackStencilFunc")
			{
				if (MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementAttr.as_string(""), idx))
					m_depthStencilDesc.backStencilFunc = (RenderState::ComparisonFunc)idx;
			}
			else if (strName == "BackStencilReadMask")
				m_depthStencilDesc.backStencilReadMask = elementAttr.as_uint();
			else if (strName == "BackStencilWriteMask")
				m_depthStencilDesc.backStencilWriteMask = elementAttr.as_int();
			else if (strName == "BackStencilFailOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.backStencilFailOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "BackStencilDepthFailOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.backStencilDepthFailOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "BackStencilPassOP")
			{
				if (MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementAttr.as_string(""), idx))
					m_depthStencilDesc.backStencilPassOP = (DepthStencilState::StencilOperation)idx;
			}
			else if (strName == "BackStencilRef")
				m_depthStencilDesc.backStencilRef = elementAttr.as_int();
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