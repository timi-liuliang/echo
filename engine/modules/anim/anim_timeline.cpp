#include "anim_timeline.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Timeline::Timeline()
		: m_animations("")
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
		if (m_animations.isValid())
		{
			float deltaTime = Engine::instance()->getFrameTime();
			AnimClip* clip = m_clips[m_animations.getIdx()];
			if (clip)
			{
				clip->update(deltaTime);
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

	void Timeline::addObject(ObjectType type, const String& path)
	{

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
}
