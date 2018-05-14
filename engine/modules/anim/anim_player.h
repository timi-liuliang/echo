#pragma once

#include "engine/core/scene/Node.h"
#include "anim_clip.h"

namespace Echo
{
	class AnimPlayer : public Node
	{
		ECHO_CLASS(AnimPlayer, Node)

	public:
		AnimPlayer();
		virtual ~AnimPlayer();

		// bind class methods to script
		static void bindMethods();

		// add node


	private:
		vector<AnimClip*>		m_clips;
	};
}