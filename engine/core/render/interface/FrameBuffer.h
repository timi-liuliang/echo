#pragma once

#include "engine/core/scene/node.h"
#include "Texture.h"
#include "RenderView.h"

namespace Echo
{
	class FrameBuffer : public Node
	{
		ECHO_CLASS(FrameBuffer, Node)

	public:
		struct Options
		{
			bool					depth;
			bool					msaa;
			bool					cubemap;

			Options()
				: depth(false)
				, msaa(false)
				, cubemap(false)
			{}
		};

        // Type
        enum class Attachment
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
		FrameBuffer() {}
		FrameBuffer(ui32 id, ui32 width, ui32 height, PixelFormat format, const Options& option = Options());
		virtual ~FrameBuffer();

		// get id
		ui32 id() const { return m_id; }

		// width and height
		ui32 width() const { return m_width; }
		ui32 height() const { return m_height; }

		// has depth
		bool isHasDepth() const { return m_isHasDepth; }
		bool isHasMSAA() const { return m_isHasMSAA; }

		// pixel format
		PixelFormat pixelFormat() const { return m_pixelFormat; }

		// get bind texture
		Texture* getBindTexture() { return m_bindTexture; }
		Texture* getDepthTexture() { return m_depthTexture; }

		// create
		virtual bool create() { return false; }

		// begin render
		virtual bool beginRender(bool clearColor, const Color& bgColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) { return false; }

		// clear
		virtual void clear(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue) {}

		// end render
		virtual bool endRender() { return false; }
        
		// disable frame buffer
		virtual bool invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil) { return false; }

		// on resize
		virtual void onResize(ui32 width, ui32 height) {}

		// save target
		virtual bool save(const char* file) { return false; }

        // get memory size
		ui32 getMemorySize();

	protected:
        vector<RenderView*>     m_colorViews;       // color views
        RenderView*             m_dsView;           // depth stencial views

		bool					m_isHasMSAA;
		ui32					m_id;
        PixelFormat             m_pixelFormat;
		bool					m_isHasDepth;
		ui32					m_width;
		ui32					m_height;
		ui32					m_clearFlags;
		Texture*				m_bindTexture;
		Texture*				m_depthTexture;
		ui32					m_RenderFrameCount;
	};
	typedef map<ui32, FrameBuffer*>::type	FramebufferMap;
}
