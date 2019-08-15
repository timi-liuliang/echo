#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/scene/node.h"
#include "Texture.h"
#include "RenderView.h"

namespace Echo
{
	class FrameBuffer : public Node
	{
		ECHO_CLASS(FrameBuffer, Node)

	public:
        // Type
        enum class Attachment : ui8
        {
            Color0 = 0,
            Color1,
            Color2,
            Color3,
            Color4,
            Color5,
            Color6,
            Color7,
            DepthStencil
        };

	public:
        FrameBuffer();
		FrameBuffer(ui32 id, ui32 width, ui32 height);
		virtual ~FrameBuffer();

		// get id
		ui32 id() const { return m_id; }

        // attach render view
        virtual void attach(Attachment attachment, RenderView* renderView) {}
        virtual void detach(Attachment attachment) {}

		// begin|end render
		virtual bool begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue) { return false; }
        virtual bool end() { return false; }

		// on resize
        virtual void onSize(ui32 width, ui32 height) {}

	protected:
        ui32					    m_id = 0;
        ui32                        m_width = 0;
        ui32                        m_height = 0;
        array<RenderView*, 9>       m_views;
	};
	typedef map<ui32, FrameBuffer*>::type	FramebufferMap;
}
