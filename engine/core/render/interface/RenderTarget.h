#pragma once

#include "engine/core/scene/node.h"
#include "Texture.h"

namespace Echo
{
	class RenderTarget : public Node
	{
		ECHO_CLASS(RenderTarget, Node)

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

	public:
		RenderTarget() {}
		RenderTarget(ui32 id, ui32 width, ui32 height, PixelFormat format, const Options& option = Options());
		virtual ~RenderTarget();

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

		// is cubemap
		bool isCubemap() const { return m_isCubemap; }

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
		bool					m_isCubemap;
		RenderTarget*			m_depthTarget;
	};
	typedef map<ui32, RenderTarget*>::type	RenderTargetMap;
}
