#include "anim_timeline.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node.h"

namespace Echo
{
	Timeline::Timeline()
		: m_animations("")
		, m_playState(PlayState::Stop)
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

				break;
			}
		}
	}

	void Timeline::renameClip(int idx, const char* newName)
	{
		m_clips[idx]->m_name = newName;
		m_animations.m_options[idx] = newName;
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

	// play animation
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
			AnimNode* animNode = EchoNew(AnimNode);
			animNode->m_userData = ObjectUserData( type, path);

			clip->m_nodes.push_back(animNode);
		}
	}

	void Timeline::addProperty(const String& animName, Object* object, const String& propertyName)
	{
		Echo::Node* node = dynamic_cast<Echo::Node*>(object);
		AnimClip* clip = getClip(animName.c_str());
		if (clip && node)
		{
			for (AnimNode* animNode : clip->m_nodes)
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
	}

	void Timeline::addKey(const String& animName, Object* object, const String& propertyName, float time, const Variant& value)
	{
		Echo::Node* node = dynamic_cast<Echo::Node*>(object);
		AnimClip* clip = getClip(animName.c_str());
		if (clip && node)
		{
			for (AnimNode* animNode : clip->m_nodes)
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
			for (AnimNode* animNode : clip->m_nodes)
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
