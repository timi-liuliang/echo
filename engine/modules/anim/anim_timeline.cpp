#include "anim_timeline.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include <thirdparty/pugixml/pugixml_ext.hpp>

namespace Echo
{
	Timeline::Timeline()
		: m_animations("")
		, m_playState(PlayState::Stop)
		, m_isAnimDataDirty(false)
	{
	}

	Timeline::~Timeline()
	{
	}

	void Timeline::bindMethods()
	{
		CLASS_BIND_METHOD(Timeline, getAnim, DEF_METHOD("getAnim"));
		CLASS_BIND_METHOD(Timeline, setAnim, DEF_METHOD("setAnim"));
		CLASS_BIND_METHOD(Timeline, getAnimData, DEF_METHOD("getAnimData"));
		CLASS_BIND_METHOD(Timeline, setAnimData, DEF_METHOD("setAnimData"));

		CLASS_REGISTER_PROPERTY(Timeline, "Anim", Variant::Type::StringOption, "getAnim", "setAnim");
		CLASS_REGISTER_PROPERTY(Timeline, "AnimData", Variant::Type::Base64String, "getAnimData", "setAnimData");
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

			m_clips.push_back(clip);
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

	void Timeline::update_self()
	{
		if (m_animations.isValid() && m_playState==PlayState::Playing)
		{
			float deltaTime = Engine::instance()->getFrameTime();
			AnimClip* clip = m_clips[m_animations.getIdx()];
			if (clip)
			{
				clip->update(deltaTime);

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

	// get anim data
	const Base64String& Timeline::getAnimData()
	{ 
		static const char* AnimCurveInterpolationTypeStr[] = { "Linear", "Discrete" };
		static const char* AnimPropertyTypeStr[] = { "Unknown", "Float", "Vector3", "Vector4", "Quaternion" };
		static const char* ObjectTypeStr[] = { "Node", "Setting", "Resource" };

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
						const String& propertyName = any_cast<String>(animProperty->m_userData);
						pugi::xml_node propertyXmlNode = objectXmlNode.append_child("property");
						propertyXmlNode.append_attribute("name").set_value(propertyName.c_str());
						propertyXmlNode.append_attribute("type").set_value(AnimPropertyTypeStr[int(animProperty->m_type)]);
						propertyXmlNode.append_attribute("interpolation_type").set_value(AnimCurveInterpolationTypeStr[int(animProperty->m_interpolationType)]);

						// keys
						for (int keyIdx = 0; keyIdx < animProperty->getKeyNumber(); keyIdx++)
						{
							pugi::xml_node keyXmlNode = propertyXmlNode.append_child("key");
							keyXmlNode.append_attribute("time").set_value(animProperty->getKeyTime( keyIdx));
							keyXmlNode.append_attribute("value").set_value(animProperty->getKeyValueStr(keyIdx).c_str());
						}
					}
				}
			}

			m_animData.encode( pugi::get_doc_string(doc).c_str());
		}

		return m_animData; 
	}

	// set anim data
	void Timeline::setAnimData(const Base64String& data)
	{ 
		m_animData = data;
		
		// parse clips


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
				clip->m_time = 0.f;
				clip->update( 0.f);
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

			clip->m_objects.push_back(animNode);

			m_isAnimDataDirty = true;
		}
	}

	void Timeline::addProperty(const String& animName, Object* object, const String& propertyName)
	{
		Echo::Node* node = dynamic_cast<Echo::Node*>(object);
		AnimClip* clip = getClip(animName.c_str());
		if (clip && node)
		{
			for (AnimObject* animNode : clip->m_objects)
			{
				const ObjectUserData& userData = any_cast<ObjectUserData>(animNode->m_userData);
				if (userData.m_path == node->getNodePathRelativeTo(this))
				{
					AnimProperty* property = animNode->addProperty( propertyName, AnimProperty::Type::Vector3);
					if (property)
					{
						property->setInterpolationType(AnimCurve::InterpolationType::Linear);

						// test
						addKey(animName, object, propertyName, 0.f, Vector3::ZERO);
						addKey(animName, object, propertyName, 3.f, Vector3(500.f, 0.f, 0.f));

						clip->m_length = 3.f;
					}

					break;
				}
			}
		}

		// dirty flag
		m_isAnimDataDirty = true;
	}

	void Timeline::addKey(const String& animName, Object* object, const String& propertyName, float time, const Variant& value)
	{
		Echo::Node* node = dynamic_cast<Echo::Node*>(object);
		AnimClip* clip = getClip(animName.c_str());
		if (clip && node)
		{
			for (AnimObject* animNode : clip->m_objects)
			{
				const ObjectUserData& userData = any_cast<ObjectUserData>(animNode->m_userData);
				if (userData.m_path == node->getNodePathRelativeTo(this))
				{
					for (AnimProperty* property : animNode->m_properties)
					{
						if (any_cast<String>(property->m_userData) == propertyName)
						{
							AnimPropertyVec3* vec3Prop = ECHO_DOWN_CAST<AnimPropertyVec3*>(property);
							vec3Prop->addKey(time, value.toVector3());
						}

						break;
					}

					break;
				}
			}
		}

		// dirty flag
		m_isAnimDataDirty = true;
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
				Echo::Node* node = this->getNode(objUserData.m_path.c_str());
				if (node)
				{
					for (AnimProperty* property : animNode->m_properties)
					{
						const Echo::String& propertyName = any_cast<String>(property->m_userData);

						// swith case
						Class::setPropertyValue( node, propertyName, ((AnimPropertyVec3*)property)->getValue());
					}
				}
			}
		}
	}
}
