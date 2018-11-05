#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class AudioListener : public Node
	{
		ECHO_CLASS(AudioListener, Node)

	public:
		AudioListener();
		virtual ~AudioListener();
	};
}