#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class AudioPlayer : public Node 
	{
		ECHO_CLASS(AudioPlayer, Node)

	public:
		AudioPlayer();
		virtual ~AudioPlayer();
	};
}