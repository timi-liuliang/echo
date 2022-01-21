#pragma once

#include "engine/core/scene/node.h"
#include "video_base.h"
#include "capture/video_capture_interface.h"
#include "base/texture/texture_render_target_2d.h"

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
		const ResourcePath& getRenderTarget() const { return m_renderTargetResPath; }

		// Save
		void setSavePath(const ResourcePath& path);
		const ResourcePath& getSavePath() const { return m_savePath; }

		// Update
		virtual void updateInternal(float elapsedTime) override;

	private:
		IVideoCapture*				m_implement = nullptr;
		ResourcePath				m_renderTargetResPath = ResourcePath("", ".rt");
		TextureRenderTarget2DPtr	m_renderTarget;
		ResourcePath				m_savePath;
	};
}
