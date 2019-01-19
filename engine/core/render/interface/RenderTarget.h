#pragma once

#include "Texture.h"

namespace Echo
{
	class RenderTarget
	{
	public:
		struct Options
		{
			bool					depth;
			bool					msaa;
			bool					cubemap;
			RenderTarget*			depthTarget;

			Options()
				: depth(false)
				, msaa(false)
				, cubemap(false)
				, depthTarget(nullptr)
			{}
		};

	public:
		RenderTarget(ui32 id, ui32 width, ui32 height, PixelFormat format, const Options& option = Options());
		virtual ~RenderTarget();

		// get id
		ui32 id() const { return m_id; }

		// width and height
		ui32 width() const { return m_width; }
		ui32 height() const { return m_height; }

		// has depth
		bool hasDepth() const { return m_bHasDepth; }
		bool hasMSAA() const { return m_bHasMSAA; }

		// pixel format
		PixelFormat pixelFormat() const { return m_pixelFormat; }

		// get bind texture
		Texture* getBindTexture() { return m_bindTexture; }
		Texture* getDepthTexture() { return m_depthTexture; }

		// 是否为立方体贴图
		bool isCubemap() const { return m_isCubemap; }

		// create
		virtual bool create()=0;

		// begin render
		virtual bool beginRender(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)=0;

		// clear
		virtual void clear(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)=0;

		// end render
		virtual bool endRender() = 0;
        
		// disable frame buffer
        virtual bool invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)=0;

		// on resize
		virtual void onResize(ui32 width, ui32 height) = 0;

		// save target
		virtual bool save(const char* file) = 0;

		// 此方法待移除
		virtual bool doStoreDefaultRenderTarget()=0;

		// 此方法待移除
		virtual bool doRestoreDefaultRenderTarget()=0;

		void reusageDepthTarget(RenderTarget* depthTarget);

		ui32 getMemorySize();

	protected:
		bool					m_bHasMSAA;
		ui32					m_id;
        PixelFormat             m_pixelFormat;
		bool					m_bHasDepth;
		ui32					m_width;
		ui32					m_height;
		ui32					m_clearFlags;
		Texture*				m_bindTexture;
		Texture*				m_depthTexture;
		ui32					m_RenderFrameCount;
		bool					m_isCubemap;
		RenderTarget*			m_depthTarget;
	public:
		bool					m_isFrameBufferChange;
		bool					m_isViewportChange;
	};
	typedef map<ui32, RenderTarget*>::type	RenderTargetMap;
}
