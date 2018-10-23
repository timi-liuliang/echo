#include "anim_player.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	AnimPlayer::AnimPlayer()
		: m_animations("")
	{
	}

	AnimPlayer::~AnimPlayer()
	{
	}

	void AnimPlayer::bindMethods()
	{
		CLASS_BIND_METHOD(AnimPlayer, getAnim, DEF_METHOD("getAnim"));
		CLASS_BIND_METHOD(AnimPlayer, setAnim, DEF_METHOD("setAnim"));
		CLASS_BIND_METHOD(AnimPlayer, getAnimData, DEF_METHOD("getAnimData"));
		CLASS_BIND_METHOD(AnimPlayer, setAnimData, DEF_METHOD("setAnimData"));

		CLASS_REGISTER_PROPERTY(AnimPlayer, "Anim", Variant::Type::StringOption, "getAnim", "setAnim");
		CLASS_REGISTER_PROPERTY(AnimPlayer, "AnimData", Variant::Type::Base64String, "getAnimData", "setAnimData");
	}

	// play anim
	void AnimPlayer::setAnim(const StringOption& animName)
	{
		m_animations.setValue(animName.getValue());
	}

	// add clip
	void AnimPlayer::addClip(AnimClip* clip)
	{
		if (clip)
		{
			if (clip->m_name.empty())
				generateUniqueName(clip->m_name);

			m_clips.push_back(clip);
			m_animations.addOption(clip->m_name);
		}
	}

	// update self
	void AnimPlayer::update_self()
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

	// is anim exist
	bool AnimPlayer::isAnimExist(const char* animName)
	{
		for (AnimClip* clip : m_clips)
		{
			if (clip->m_name == animName)
				return true;
		}

		return false;
	}

	// generate unique name
	void AnimPlayer::generateUniqueName(String& oName)
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
