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

		// Start
		void openDevice();

		// Render target
		void setRenderTarget(const ResourcePath& res);
		const ResourcePath& getRenderTarget() const { return m_renderTarget; }

		// Update
		virtual void updateInternal(float elapsedTime) override;

	private:
		IVideoCapture*	m_implement = nullptr;
		ResourcePath	m_renderTarget = ResourcePath("", ".rt");
	};
}
