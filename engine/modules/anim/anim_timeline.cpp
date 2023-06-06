#include "anim_timeline.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include <thirdparty/pugixml/pugixml_ext.hpp>
#include "engine/core/util/magic_enum.hpp"

namespace Echo
{
	Timeline::Timeline()
		: m_playState(PlayState::Stop)
		, m_isAnimDataDirty(false)
	{
	}

	Timeline::~Timeline()
	{
        EchoSafeDeleteContainer(m_clips, AnimClip);
	}

	void Timeline::bindMethods()
	{
		CLASS_BIND_METHOD(Timeline, play);
		CLASS_BIND_METHOD(Timeline, getAnim);
		CLASS_BIND_METHOD(Timeline, setAnim);
		CLASS_BIND_METHOD(Timeline, getAnimData);
		CLASS_BIND_METHOD(Timeline, setAnimData);
		CLASS_BIND_METHOD(Timeline, getTimeScale);
		CLASS_BIND_METHOD(Timeline, setTimeScale);

		CLASS_REGISTER_PROPERTY(Timeline, "TimeScale", Variant::Type::Real, getTimeScale, setTimeScale);
		CLASS_REGISTER_PROPERTY(Timeline, "Anim", Variant::Type::StringOption, getAnim, setAnim);
		CLASS_REGISTER_PROPERTY(Timeline, "AnimData", Variant::Type::String, getAnimData, setAnimData);

		CLASS_REGISTER_PROPERTY_HINT(Timeline, "AnimData", PropertyHintType::XmlCData, "true");
	}

	void Timeline::setAnim(const StringOption& animName)
	{
		m_animations.setValue(animName.getValue());
	}

	void Timeline::addClip(AnimClip* clip)
	{
		if (clip)
		{
			if (clip->m_name.empty())
				generateUniqueAnimName("Anim ", clip->m_name);

			m_clips.emplace_back(clip);
			m_animations.addOption(clip->m_name);

			m_isAnimDataDirty = true;
		}
	}

	int Timeline::getClipIndex(const char* animName)
	{
		for (size_t i = 0; i < m_clips.size(); i++)
		{
			if (m_clips[i]->m_name == animName)
			{
				return (int)i;
			}
		}

		return -1;
	}

	// get clip by name
	AnimClip* Timeline::getClip(const char* animName)
	{
		int index = getClipIndex(animName);
		if (index != -1)
		{
			return m_clips[index];
		}

		return nullptr;
	}

	void Timeline::deleteClip(const char* animName)
	{
		for (size_t i = 0; i < m_clips.size(); i++)
		{
			if (m_clips[i]->m_name == animName)
			{
				m_clips.erase(m_clips.begin() + i);
				m_animations.removeOption(animName);

				m_isAnimDataDirty = true;

				break;
			}
		}
	}

	void Timeline::renameClip(int idx, const char* newName)
	{
		m_clips[idx]->m_name = newName;
		m_animations.m_options[idx] = newName;

		m_isAnimDataDirty = true;
	}

	void Timeline::updateInternal(float elapsedTime)
	{
		if (m_animations.isValid() && m_playState == PlayState::Playing)
		{
			ui32 deltaTime = Engine::instance()->getFrameTimeMS();
			AnimClip* clip = m_clips[m_animations.getIdx()];
			if (clip)
			{
				clip->update(ui32(deltaTime * m_timeScale));

				extractClipData(clip);
			}
		}
	}

	bool Timeline::isAnimExist(const char* animName)
	{
		for (AnimClip* clip : m_clips)
		{
			if (clip->m_name == animName)
				return true;
		}

		return false;
	}

	static const char* AnimCurveInterpolationTypeStr[] = { "Linear", "Discrete" };
	static const char* ObjectTypeStr[] = { "Node", "Setting", "Resource" };

	const String& Timeline::getAnimData()
	{ 
		if (m_isAnimDataDirty)
		{
			m_isAnimDataDirty = false;

			// pugi xml doc
			pugi::xml_document doc;

			// declaration
			pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
			dec.append_attribute("version") = "1.0";
			dec.append_attribute("encoding") = "utf-8";

			// root node
			pugi::xml_node rootXmlNode = doc.append_child("clips");

			for (AnimClip* animClip : m_clips)
			{
				pugi::xml_node clipXmlNode = rootXmlNode.append_child("clip");
				clipXmlNode.append_attribute("name").set_value( animClip->m_name.c_str());
				clipXmlNode.append_attribute("length").set_value(animClip->m_length);

				for (AnimObject* animObject : animClip->m_objects)
				{
					const ObjectUserData& userData = any_cast<ObjectUserData>(animObject->m_userData);
					pugi::xml_node objectXmlNode = clipXmlNode.append_child("object");
					objectXmlNode.append_attribute("type").set_value(ObjectTypeStr[userData.m_type]);
					objectXmlNode.append_attribute("path").set_value(userData.m_path.c_str());

					for (AnimProperty* animProperty : animObject->m_properties)
					{
						pugi::xml_node propertyXmlNode = objectXmlNode.append_child("property");
						propertyXmlNode.append_attribute("name").set_value(animProperty->m_name.c_str());
						propertyXmlNode.append_attribute("type").set_value(std::string(magic_enum::enum_name(animProperty->m_type)).c_str());
						propertyXmlNode.append_attribute("interpolation_type").set_value(AnimCurveInterpolationTypeStr[int(animProperty->m_interpolationType)]);

						AnimPropertyBool*  boolProperty = dynamic_cast<AnimPropertyBool*>(animProperty);
						if (boolProperty)
						{
							for (auto curveKey : boolProperty->m_keys)
							{
								pugi::xml_node keyXmlNode = propertyXmlNode.append_child("key");
								keyXmlNode.append_attribute("time").set_value(curveKey.first);
								keyXmlNode.append_attribute("value").set_value(curveKey.second);
							}
						}

						AnimPropertyString* stringProperty = dynamic_cast<AnimPropertyString*>(animProperty);
						if (stringProperty)
						{
							for (auto curveKey : stringProperty->m_keys)
							{
								pugi::xml_node keyXmlNode = propertyXmlNode.append_child("key");
								keyXmlNode.append_attribute("time").set_value(curveKey.first);
								keyXmlNode.append_attribute("value").set_value(curveKey.second.c_str());
							}
						}

						AnimPropertyCurve* curveProperty = dynamic_cast<AnimPropertyCurve*>(animProperty);
						if (curveProperty)
						{
							i32 curveCount = i32(curveProperty->m_curves.size());
							for (i32 curveIdx = 0; curveIdx < curveCount; curveIdx++)
							{
								AnimCurve* curve = curveProperty->m_curves[curveIdx];
								pugi::xml_node curveXmlNode = propertyXmlNode.append_child("curve");
								curveXmlNode.append_attribute("index").set_value(curveIdx);
								for (auto curveKey : curve->m_keys)
								{
									pugi::xml_node keyXmlNode = curveXmlNode.append_child("key");
									keyXmlNode.append_attribute("time").set_value(curveKey.first);
									keyXmlNode.append_attribute("value").set_value(curveKey.second);
								}
							}
						}
					}
				}
			}

			m_animData = pugi::get_doc_string(doc).c_str();
		}

		return m_animData; 
	}

	void Timeline::setAnimData(const String& data)
	{ 
		// clear
		EchoSafeDeleteContainer(m_clips, AnimClip);
		m_animData = data;

		// parse clips
		pugi::xml_document doc; 
		doc.load(data.c_str());

		// root node
		pugi::xml_node rootXmlNode = doc.child("clips");
		if (rootXmlNode)
		{
			for (pugi::xml_node clipNode = rootXmlNode.child("clip"); clipNode; clipNode = clipNode.next_sibling("clip"))
			{
				AnimClip* animClip = EchoNew(AnimClip);
				animClip->m_name = clipNode.attribute("name").as_string();
				animClip->m_length = clipNode.attribute("length").as_int();
				addClip(animClip);

				for (pugi::xml_node objectNode = clipNode.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
				{
					Echo::String typeStr = objectNode.attribute("type").as_string();
					ObjectType type =  typeStr== "Node" ? ObjectType::Node : ( typeStr=="Setting" ? ObjectType::Setting : ObjectType::Resource);
					Echo::String path = objectNode.attribute("path").as_string();
					addObject(animClip->m_name, type, path);

					for (pugi::xml_node propertyNode = objectNode.child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
					{
						Echo::StringArray propertyChain;
						Echo::String propertyName = propertyNode.attribute("name").as_string();
						Echo::String typeStr = propertyNode.attribute("type").as_string();
						Echo::String interpolationType = propertyNode.attribute("interpolation_type").as_string();
						AnimProperty::Type propertyType = magic_enum::enum_cast<AnimProperty::Type>(typeStr.c_str()).value_or(AnimProperty::Type::Unknown);

						if (propertyType != AnimProperty::Type::Unknown)
						{
							propertyChain.emplace_back(propertyName);
							addProperty(animClip->m_name, path, propertyChain, propertyType);

							if (propertyType == AnimProperty::Type::Bool)
							{
								for (pugi::xml_node keyNode = propertyNode.child("key"); keyNode; keyNode = keyNode.next_sibling("key"))
								{
									ui32 time = keyNode.attribute("time").as_uint();
									bool value = keyNode.attribute("value").as_bool();

									addKey(animClip->m_name, path, propertyName, time, value);
								}
							}
							else if (propertyType == AnimProperty::Type::String)
							{
								for (pugi::xml_node keyNode = propertyNode.child("key"); keyNode; keyNode = keyNode.next_sibling("key"))
								{
									ui32 time = keyNode.attribute("time").as_uint();
									String value = keyNode.attribute("value").as_string();

									addKey(animClip->m_name, path, propertyName, time, value);
								}
							}
							else if (propertyType == AnimProperty::Type::Vector3)
							{
								for (pugi::xml_node curveNode = propertyNode.child("curve"); curveNode; curveNode = curveNode.next_sibling("curve"))
								{
									i32 curveIdx = curveNode.attribute("index").as_int();
									for (pugi::xml_node keyNode = curveNode.child("key"); keyNode; keyNode = keyNode.next_sibling("key"))
									{
										ui32 time = keyNode.attribute("time").as_uint();
										float value = keyNode.attribute("value").as_float();

										addKey(animClip->m_name, path, propertyName, curveIdx, time, value);
									}
								}
							}
						}
					}
				}
			}
		}

		m_isAnimDataDirty = false;
	}

	void Timeline::play(const char* animName)
	{
		setAnim(animName);

		m_playState = PlayState::Playing;
	}

	void Timeline::pause()
	{
		m_playState = PlayState::Pause;
	}

	void Timeline::stop()
	{
		m_playState = PlayState::Stop;
		if (m_animations.isValid())
		{
			AnimClip* clip = m_clips[m_animations.getIdx()];
			if (clip)
			{
				clip->m_time = 0;
				clip->update(0);
				extractClipData(clip);
			}
		}
	}

	void Timeline::addObject(const String& animName, ObjectType type, const String& path)
	{
		AnimClip* clip = getClip(animName.c_str());
		if (clip)
		{
			AnimObject* animNode = EchoNew(AnimObject);
			animNode->m_userData = ObjectUserData( type, path);

			clip->m_objects.emplace_back(animNode);

			m_isAnimDataDirty = true;
		}
	}

	AnimProperty* Timeline::getProperty(const String& animName, const String& objectPath, const StringArray& propertyChain)
	{
		AnimClip* clip = getClip(animName.c_str());
		if (clip)
		{
			String propertyName = StringUtil::ToString(propertyChain);
			for (AnimObject* animNode : clip->m_objects)
			{
				const ObjectUserData& userData = any_cast<ObjectUserData>(animNode->m_userData);
				if (userData.m_path == objectPath)
				{
					for (AnimProperty* property : animNode->m_properties)
					{
						if (property->m_name == propertyName)
						{
							return property;
						}
					}
				}
			}
		}

		return nullptr;
	}

	bool Timeline::addProperty(const String& animName, const String& objectPath, const StringArray& propertyChain, AnimProperty::Type propertyType)
	{
		AnimClip* clip = getClip(animName.c_str());
		if (clip)
		{
			for (AnimObject* animObject : clip->m_objects)
			{
				const ObjectUserData& userData = any_cast<ObjectUserData>(animObject->m_userData);
				if (userData.m_path == objectPath)
				{		
					String propertyName = StringUtil::ToString(propertyChain);
					if (!animObject->isExist(propertyName))
					{
						animObject->addProperty(propertyName, propertyType);
						m_isAnimDataDirty = true;

						return true;
					}
				}
			}
		}

		return false;
	}

	void Timeline::addKey(const String& animName, const String& objectPath, const String& propertyName, ui32 time, bool value)
	{
		AnimProperty* animProperty = getProperty(animName, objectPath, StringUtil::Split(propertyName));
		if (animProperty && animProperty->getType() == AnimProperty::Type::Bool)
		{
			AnimPropertyBool* strProp = ECHO_DOWN_CAST<AnimPropertyBool*>(animProperty);
			strProp->addKey(time, value);
		}
	}

	void Timeline::addKey(const String& animName, const String& objectPath, const String& propertyName, ui32 time, const String& value)
	{
		AnimProperty* animProperty = getProperty(animName, objectPath, StringUtil::Split(propertyName));
		if (animProperty && animProperty->getType() == AnimProperty::Type::String)
		{
			AnimPropertyString* boolProp = ECHO_DOWN_CAST<AnimPropertyString*>(animProperty);
			boolProp->addKey(time, value);
		}
	}

	void Timeline::addKey(const String& animName, const String& objectPath, const String& propertyName, int curveIdx, ui32 time, float value)
	{
		AnimProperty* animProperty = getProperty(animName, objectPath, StringUtil::Split(propertyName));
		if (animProperty)
		{
			if (animProperty->getType() == AnimProperty::Type::Vector3)
			{
				AnimPropertyVec3* vec3Prop = ECHO_DOWN_CAST<AnimPropertyVec3*>(animProperty);
				vec3Prop->addKeyToCurve(curveIdx, time, value);
			}
		}

		// dirty flag
		m_isAnimDataDirty = true;
	}

	void Timeline::setKey(const String& animName, const String& objectPath, const String& propertyName, int curveIdx, int keyIdx, float value)
	{
		AnimProperty* animProperty = getProperty(animName, objectPath, StringUtil::Split(propertyName));
		if (animProperty)
		{
			if (animProperty->getType() == AnimProperty::Type::Vector3)
			{
				AnimPropertyVec3* vec3Property = ECHO_DOWN_CAST<AnimPropertyVec3*>(animProperty);
				if (vec3Property)
					vec3Property->setKeyValue(curveIdx, keyIdx, value);
			}
		}
	}

	void Timeline::generateUniqueAnimName(const String& prefix, String& oName)
	{
		for (i32 i = 0; i < 65535; i++)
		{
			String name = StringUtil::Format("%s%d", prefix.c_str(), i);
			if (!isAnimExist(name.c_str()))
			{
				oName = name;
				break;
			}
		}
	}

	void Timeline::extractClipData(AnimClip* clip)
	{
		if (clip)
		{
			for (AnimObject* animNode : clip->m_objects)
			{
				const ObjectUserData& objUserData = any_cast<ObjectUserData>(animNode->m_userData);
				for (AnimProperty* property : animNode->m_properties)
				{
					const Echo::StringArray propertyChain = StringUtil::Split(property->m_name);
					Echo::Object* node = getLastObject(objUserData.m_path.c_str(), propertyChain);
					if (node)
					{
						switch (property->getType())
						{
						case AnimProperty::Type::Bool:
						{
							AnimPropertyBool* boolProperty = ECHO_DOWN_CAST<AnimPropertyBool*>(property);
							if (boolProperty->isActive())
								Class::setPropertyValue(node, propertyChain.back(), boolProperty->getValue());
						}
						break;
						case AnimProperty::Type::Vector3:
						{
							Class::setPropertyValue(node, propertyChain.back(), ((AnimPropertyVec3*)property)->getValue());
						}
						break;
						case AnimProperty::Type::String:
						{
							Variant::Type vType = getAnimPropertyVariableType(objUserData.m_path.c_str(), propertyChain);
							if (vType == Variant::Type::String)
							{
								Class::setPropertyValue(node, propertyChain.back(), ((AnimPropertyString*)property)->getValue());
							}
							else if (vType == Variant::Type::ResourcePath)
							{
								ResourcePath resPath = ((AnimPropertyString*)property)->getValue();
								Class::setPropertyValue(node, propertyChain.back(), resPath);
							}
						}
						break;
						default: break;
						}
					}
				}
			}
		}
	}

	AnimProperty::Type Timeline::getAnimPropertyType(const String& objectPath, const StringArray& propertyChain)
	{
		Variant::Type type = getAnimPropertyVariableType(objectPath, propertyChain);
		switch (type)
		{
		case Variant::Type::Bool:			return AnimProperty::Type::Bool;
		case Variant::Type::Vector3:		return AnimProperty::Type::Vector3;
		case Variant::Type::ResourcePath:	return AnimProperty::Type::String;
		case Variant::Type::Object:			return AnimProperty::Type::Object;
		default:							return AnimProperty::Type::Unknown;
		}
	}

	Variant::Type Timeline::getAnimPropertyVariableType(const String& objectPath, const StringArray& propertyChain)
	{
		Echo::Object* object = getLastObject(objectPath, propertyChain);
		if (object)
		{
			Variant::Type type = Class::getPropertyType(object, propertyChain.back());
			return type;
		}

		return Variant::Type::Unknown;
	}

	Object* Timeline::getLastObject(const String& objectPath, const StringArray& propertyChain)
	{
		Echo::Node* node = getNode(objectPath.c_str());
		Echo::Object* result = node;

		for (i32 i = 0; i < i32(propertyChain.size()) - 1; i++)
		{
			Echo::Variant propertyValue;
			Class::getPropertyValue(result, propertyChain[i], propertyValue);

			result = propertyValue.toObj();
		}

		return result;
	}
}
