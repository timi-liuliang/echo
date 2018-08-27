#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/util/base64.h"
#include "anim_clip.h"

namespace Echo
{
	class AnimPlayer : public Node
	{
		ECHO_CLASS(AnimPlayer, Node)

	public:
		AnimPlayer();
		virtual ~AnimPlayer();

		// play
		void play(const char* animName);

		// get anim data
		const Base64String& getAnimData() const { return m_animData; }

		// set anim data
		void setAnimData(const Base64String& data) { m_animData = data; }

	public:
		// add clip
		void addClip(AnimClip* clip);

	private:
		Base64String			m_animData;
		vector<AnimClip*>::type	m_clips;
	};
}
