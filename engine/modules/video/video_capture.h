#pragma once

#include "engine/core/scene/node.h"
#include "video_base.h"
#include <thirdparty/jplayer/player.h>

namespace Echo
{
	class VideoCapture : public Node
	{
		ECHO_CLASS(VideoCapture, Node)

	public:
		VideoCapture();
		virtual ~VideoCapture();
	};
}
