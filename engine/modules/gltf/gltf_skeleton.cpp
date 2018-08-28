#include "gltf_skeleton.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	GltfSkeleton::GltfSkeleton()
		: m_animations("")
	{
	}

	GltfSkeleton::~GltfSkeleton()
	{
	}

	void GltfSkeleton::bindMethods()
	{
		CLASS_BIND_METHOD(GltfSkeleton, getAnim, DEF_METHOD("getAnim"));
		CLASS_BIND_METHOD(GltfSkeleton, setAnim, DEF_METHOD("setAnim"));

		CLASS_REGISTER_PROPERTY(GltfSkeleton, "Anim", Variant::Type::StringOption, "getAnim", "setAnim");
	}

	// play anim
	void GltfSkeleton::setAnim(const StringOption& animName)
	{
		m_animations.setValue(animName.getValue());
	}

	// add clip
	void GltfSkeleton::addClip(AnimClip* clip)
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
	void GltfSkeleton::update_self()
	{
		if (m_animations.isValid())
		{
			float deltaTime = Engine::instance()->getFrameTime();
			AnimClip* clip = m_clips[m_animations.getIdx()];
			if (clip)
			{
				clip->update(deltaTime, this);
			}
		}
	}

	// is anim exist
	bool GltfSkeleton::isAnimExist(const char* animName)
	{
		for (AnimClip* clip : m_clips)
		{
			if (clip->m_name == animName)
				return true;
		}

		return false;
	}

	// generate unique name
	void GltfSkeleton::generateUniqueName(String& oName)
	{
		char name[128] = "anim_";
		for (i32 i = 0; i < 65535; i++)
		{
			itoa(i, name + 5, 10);
			if (!isAnimExist(name))
			{
				oName = name;
				break;
			}
		}
	}
}