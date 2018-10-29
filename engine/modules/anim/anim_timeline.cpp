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
				generateUniqueName(clip->m_name);

			m_clips.push_back(clip);
			m_animations.addOption(clip->m_name);
		}
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

	void Timeline::generateUniqueName(String& oName)
	{
		char name[128] = "anim_";
		for (i32 i = 0; i < 65535; i++)
		{
            sprintf(name+5, "%d", i);
			if (!isAnimExist(name))
			{
				oName = name;
				break;
			}
		}
	}
}
