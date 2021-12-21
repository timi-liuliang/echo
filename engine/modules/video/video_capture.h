#pragma once

#include "engine/core/scene/node.h"
#include "video_base.h"
#include "capture/video_capture_interface.h"

namespace Echo
{
	class VideoCapture : public Node
	{
		ECHO_CLASS(VideoCapture, Node)

	public:
		VideoCapture();
		virtual ~VideoCapture();

	private:
		IVideoCapture* m_implement = nullptr;
	};
}
