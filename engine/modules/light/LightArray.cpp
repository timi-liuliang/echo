#include "LightArray.h"
#include <engine/core/resource/DataStream.h>
#include "Engine/core/Scene/Scene_Manager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_helper.hpp"

namespace Echo
{
	// 构造函数
	LightArray::LightArray()
		: m_isDirty(false)
	{
		m_info.assign(LT_Unknown);
		m_data.assign(Vector4::ZERO);
	}

	// 析构函数
	LightArray::~LightArray()
	{
		deleteAllLights();
	}

	// 更新
	void LightArray::update()
	{
		if (m_isDirty)
		{
			// 更新光源数据
			m_info.assign(LT_Unknown);
			m_data.assign(Vector4::ZERO);
			for (size_t i = 0; i < m_lights.size(); i++)
			{
				int base = i * 2 + 0;
				Light* light = m_lights[i];
				m_info[i] = light->m_type;
				switch (light->m_type)
				{
					case LT_Ambient:
					{
						m_data[base + 0] = ECHO_DOWN_CAST<AmbientLight*>(light)->m_color;
						m_data[base + 1] = Vector4::ZERO;
					}
					break;
					case LT_Directional:
					{
						m_data[base + 0] = ECHO_DOWN_CAST<DirectionalLight*>(light)->m_color;
						m_data[base + 1] = ECHO_DOWN_CAST<DirectionalLight*>(light)->m_direction;
					}
					break;
					case LT_Point:
					{
						m_data[base + 0] = ECHO_DOWN_CAST<PointLight*>(light)->m_colorAddFalloff;
						m_data[base + 1] = ECHO_DOWN_CAST<PointLight*>(light)->m_positionAndInvRadius;
					}
					break;
					case LT_PBRLight:
					{
						PBRLight* pbrLight = ECHO_DOWN_CAST<PBRLight*>(light);
					#ifdef ECHO_EDITOR_MODE
						if (pbrLight->m_castShadow && pbrLight->m_selfType == Echo::LT_Directional)
						{
							SceneManager::instance()->getShadowCamera().setLightDir(pbrLight->m_direction);
						}
					#endif

						size_t size = m_pbrLightPos.size();
						if (size <= i)
						{
							m_pbrLightPos.push_back(pbrLight->m_position);
							m_pbrLightDir.push_back(pbrLight->m_direction);
							m_pbrLightSpot.push_back(pbrLight->m_spot);
							m_pbrLightColor.push_back(pbrLight->m_color * pbrLight->m_intensity);
							m_pbrLightParam.push_back(pbrLight->m_lightParam);
						}
						else
						{
							m_pbrLightPos[i] = pbrLight->m_position;
							m_pbrLightDir[i] = pbrLight->m_direction;
							m_pbrLightSpot[i] = pbrLight->m_spot;
							m_pbrLightColor[i] = pbrLight->m_color * pbrLight->m_intensity;
							m_pbrLightParam[i] = pbrLight->m_lightParam;
						}

						if (pbrLight->m_castShadow)
						{
							std::swap(m_pbrLightPos[0], m_pbrLightPos[i]);
							std::swap(m_pbrLightDir[0], m_pbrLightDir[i]);
							std::swap(m_pbrLightSpot[0], m_pbrLightSpot[i]);
							std::swap(m_pbrLightColor[0], m_pbrLightColor[i]);
							std::swap(m_pbrLightParam[0], m_pbrLightParam[i]);
						}
					}
					break;

					default: EchoAssert(false); break;
				}

				// 应用光强度
				m_data[base].x *= light->m_intensity;
				m_data[base].y *= light->m_intensity;
				m_data[base].z *= light->m_intensity;
			}

			m_isDirty = false;
		}
	}

	void LightArray::updatePointLightPos(const Vector4& pos)
	{
		for (size_t i = 0; i < m_lights.size(); i++)
		{
			int base = i * 2;
			Light* light = m_lights[i];
			switch (light->m_type)
			{
				case LT_Point:
				{
					PointLight* pointLight = ECHO_DOWN_CAST<PointLight*>(light);
					if (pointLight && pointLight->m_followModel)
					{
						m_data[base + 1] = pointLight->m_positionAndInvRadius + pos;
					}
				}
				break;
				case  LT_PBRLight:
				{
					PBRLight* pbrLight = ECHO_DOWN_CAST<PBRLight*>(light);
					if (pbrLight->m_selfType == Echo::LT_Point || pbrLight->m_selfType == Echo::LT_Spot)
					{
						m_pbrLightPos[i] = pbrLight->m_position + pos;
					}
				}
				break;

				default:
					break;
			}
		}
	}

	// 添加光源
	Light* LightArray::addLight(LightType type)
	{
		switch (type)
		{
			case LT_Ambient:		m_lights.push_back(EchoNew(AmbientLight));		break;
			case LT_Directional:	m_lights.push_back(EchoNew(DirectionalLight)); break;
			case LT_Point:			m_lights.push_back(EchoNew(PointLight));		break;
			case LT_Spot:			m_lights.push_back(EchoNew(SpotLight));		break;
			case LT_PBRLight:		m_lights.push_back(EchoNew(PBRLight));			break;
		}

		setDirty();

		return m_lights.back();
	}

	// 删除光源
	void LightArray::deleteLight(int idx)
	{
		EchoSafeDelete(m_lights[idx], Light);

		m_lights.erase(m_lights.begin() + idx);

		setDirty();
	}

	// 删除所有光源
	void LightArray::deleteAllLights()
	{
		EchoSafeDeleteContainer(m_lights, Light);

		setDirty();
	}

	// 加载
	void LightArray::load(const char* fileName)
	{
		m_name = fileName;

		deleteAllLights();

		try
		{
			MemoryReader memReader(fileName);
			if (!memReader.getData<char*>())
			{
				throw false;
			}
			using namespace rapidxml;

			xml_document<> doc;
			doc.parse<0>(memReader.getData<char*>());

			xml_node<>* lightsNode = doc.first_node("Lights");
			if (lightsNode)
			{
				xml_node<>* lightNode = lightsNode->first_node("Light");
				while (lightNode)
				{
					String type = lightNode->first_attribute("Type")->value();
					if (type == "Ambient")
					{
						AmbientLight* light = ECHO_DOWN_CAST<AmbientLight*>(addLight(LT_Ambient));
						light->m_color = Echo::StringUtil::ParseVec4(lightNode->first_attribute("Color")->value());
						light->m_intensity = Echo::StringUtil::ParseFloat(lightNode->first_attribute("Intensity")->value());
					}
					else if (type == "Directional")
					{
						DirectionalLight* light = ECHO_DOWN_CAST<DirectionalLight*>(addLight(LT_Directional));
						light->m_color = Echo::StringUtil::ParseVec4(lightNode->first_attribute("Color")->value());
						light->m_direction = Echo::StringUtil::ParseVec4(lightNode->first_attribute("Direction")->value());
						light->m_intensity = Echo::StringUtil::ParseFloat(lightNode->first_attribute("Intensity")->value());
						xml_attribute<>* posAttrib = lightNode->first_attribute("Position");
						if (posAttrib)
						{
							light->m_position = Echo::StringUtil::ParseVec3(posAttrib->value());
						}
					}
					else if (type == "Point")
					{
						PointLight* light = ECHO_DOWN_CAST<PointLight*>(addLight(LT_Point));
						light->m_colorAddFalloff = Echo::StringUtil::ParseVec4(lightNode->first_attribute("ColorAndFalloff")->value());
						light->m_positionAndInvRadius = Echo::StringUtil::ParseVec4(lightNode->first_attribute("PosAndInvRadius")->value());
						light->m_intensity = Echo::StringUtil::ParseFloat(lightNode->first_attribute("Intensity")->value());

						rapidxml::xml_attribute<>* follow = lightNode->first_attribute("FollowModel");
						light->m_followModel = Echo::StringUtil::ParseBool(follow ? lightNode->first_attribute("FollowModel")->value() : "true");
					}
					else if (type == "PBRLight")
					{
						PBRLight* light = ECHO_DOWN_CAST<PBRLight*>(addLight(LT_PBRLight));

						light->m_selfType = (LightType)Echo::StringUtil::ParseI32(lightNode->first_attribute("SelfType")->value());
						light->m_intensity = Echo::StringUtil::ParseFloat(lightNode->first_attribute("Intensity")->value());
						light->m_direction = Echo::StringUtil::ParseVec3(lightNode->first_attribute("Direction")->value());
						light->m_position = Echo::StringUtil::ParseVec4(lightNode->first_attribute("Position")->value());
						light->m_color = Echo::StringUtil::ParseVec3(lightNode->first_attribute("Color")->value());
						light->m_spot = Echo::StringUtil::ParseVec3(lightNode->first_attribute("Spot")->value());
						light->m_lightParam = Echo::StringUtil::ParseVec3(lightNode->first_attribute("LightParam")->value());

#ifdef ECHO_EDITOR_MODE
						rapidxml::xml_attribute<>* logicPos = lightNode->first_attribute("LogicPos");
						if (logicPos)
						{
							light->m_logicPos = Echo::StringUtil::ParseVec4(logicPos->value());
						}
#endif

						rapidxml::xml_attribute<>* follow = lightNode->first_attribute("FollowModel");
						light->m_followModel = Echo::StringUtil::ParseBool(follow ? lightNode->first_attribute("FollowModel")->value() : "true");
						rapidxml::xml_attribute<>* castShadow = lightNode->first_attribute("CastShadow");
						light->m_castShadow = Echo::StringUtil::ParseBool(castShadow ? lightNode->first_attribute("CastShadow")->value() : "false");
					}
					else if (type == "Spot")
					{
						EchoAssert(false);
					}

					lightNode = lightNode->next_sibling("Light");
				}
			}
		}
		catch (...)
		{
			EchoLogError("LightArray [%s] load failed...", fileName);
		}

		update();
	}

	// 保存
	void LightArray::save(const char* fullPath)
	{
#ifdef ECHO_EDITOR_MODE
		try
		{
			using namespace rapidxml;
			xml_document<> doc;

			rapidxml_helper saveHelper(&doc);

			// 版本
			xml_node<>* xmlnode = doc.allocate_node(rapidxml::node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
			xml_node<>* rootnode = doc.allocate_node(node_element, "Lights");

			doc.append_node(xmlnode);
			doc.append_node(rootnode);

			// 保存光源数据
			for (Light* light : m_lights)
			{
				xml_node<>* lightnode = doc.allocate_node(node_element, "Light");
				rootnode->append_node(lightnode);

				lightnode->append_attribute(doc.allocate_attribute("Type", light->getType()));
				lightnode->append_attribute(doc.allocate_attribute("Intensity", saveHelper.tostr<float>(light->m_intensity)));

				switch (light->m_type)
				{
					case LT_Ambient:
					{
						lightnode->append_attribute(doc.allocate_attribute("Color", saveHelper.tostr<Vector4>(ECHO_DOWN_CAST<AmbientLight*>(light)->m_color)));
					}
					break;

					case LT_Directional:
					{
						lightnode->append_attribute(doc.allocate_attribute("Color", saveHelper.tostr<Vector4>(ECHO_DOWN_CAST<DirectionalLight*>(light)->m_color)));
						lightnode->append_attribute(doc.allocate_attribute("Direction", saveHelper.tostr<Vector4>(ECHO_DOWN_CAST<DirectionalLight*>(light)->m_direction)));
						lightnode->append_attribute(doc.allocate_attribute("Position", saveHelper.tostr<Vector3>(ECHO_DOWN_CAST<DirectionalLight*>(light)->m_position)));
					}
					break;

					case LT_Point:
					{
						lightnode->append_attribute(doc.allocate_attribute("ColorAndFalloff", saveHelper.tostr<Vector4>(ECHO_DOWN_CAST<PointLight*>(light)->m_colorAddFalloff)));
						lightnode->append_attribute(doc.allocate_attribute("PosAndInvRadius", saveHelper.tostr<Vector4>(ECHO_DOWN_CAST<PointLight*>(light)->m_positionAndInvRadius)));
						lightnode->append_attribute(doc.allocate_attribute("FollowModel", saveHelper.tostr(ECHO_DOWN_CAST<PointLight*>(light)->m_followModel)));
					}
					break;
					case LT_PBRLight:
					{
						PBRLight* pbrlight = ECHO_DOWN_CAST<PBRLight*>(light);
						lightnode->append_attribute(doc.allocate_attribute("Color", saveHelper.tostr<Vector3>(pbrlight->m_color)));
						lightnode->append_attribute(doc.allocate_attribute("Direction", saveHelper.tostr<Vector3>(pbrlight->m_direction)));
						if (pbrlight->m_selfType == LT_Directional)
						{
							pbrlight->m_position = Vector4(-pbrlight->m_direction.x, -pbrlight->m_direction.y, -pbrlight->m_direction.z, 0.f);
						}
						lightnode->append_attribute(doc.allocate_attribute("Position", saveHelper.tostr<Vector4>(pbrlight->m_position)));
						lightnode->append_attribute(doc.allocate_attribute("SelfType", saveHelper.tostr(pbrlight->m_selfType)));
						lightnode->append_attribute(doc.allocate_attribute("Spot", saveHelper.tostr<Vector3>(pbrlight->m_spot)));
						lightnode->append_attribute(doc.allocate_attribute("LightParam", saveHelper.tostr<Vector3>(pbrlight->m_lightParam)));
						lightnode->append_attribute(doc.allocate_attribute("FollowModel", saveHelper.tostr(pbrlight->m_followModel)));
						lightnode->append_attribute(doc.allocate_attribute("CastShadow", saveHelper.tostr(pbrlight->m_castShadow)));
						lightnode->append_attribute(doc.allocate_attribute("LogicPos", saveHelper.tostr(pbrlight->m_logicPos)));
					}
					break;

					case LT_Spot:
					{}
					break;
				}
			}

			// 保存
			std::ofstream out(fullPath);
			out << doc;
		}
		catch (...)
		{
			EchoLogError("LightArray::save [%s] failed", fullPath);
		}
#else
		EchoLogError("Undefined function LightArray::save()");
#endif
	}
}