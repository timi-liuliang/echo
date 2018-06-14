#pragma once

#include "engine/core/scene/Node.h"
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

		// get anim data
		const Base64String& getAnimData() const { return m_animData; }

		// set anim data
		void setAnimData(const Base64String& data) { m_animData = data; }

		// add node


	private:
		Base64String			m_animData;
		vector<AnimClip*>		m_clips;
	};
}