#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/scene/node.h"
#include "engine/core/resource/Res.h"
#include "base/texture/texture.h"
#include "base/texture/texture_render.h"

namespace Echo
{
    class FrameBuffer : public Res
    {
        ECHO_CLASS(FrameBuffer, Res)

	public:
		// Type
		enum Attachment : ui8
		{
			ColorA = 0,
			ColorB,
			ColorC,
			ColorD,
			ColorE,
			ColorF,
			ColorG,
			ColorH,
			DepthStencil
		};

		// Pixels
		struct Pixels
		{
			i32					m_width = 0;
			i32					m_height = 0;
			PixelFormat			m_format = PixelFormat::PF_UNKNOWN;
			vector<Byte>::type	m_data;

			void set(i32 width, i32 height, PixelFormat format)
			{
				m_width = width;
				m_height = height;
				m_format = format;
				m_data.resize(PixelUtil::GetPixelSize(m_format) * m_width * m_height);
			}
		};

    public:
		// begin|end render
		virtual bool begin() { return false; }
		virtual bool end() { return false; }

		// on resize
		virtual void onSize(ui32 width, ui32 height) {}

	public:
		// clear color
		bool isClearColor() const { return m_isClearColor; }
		void setClearColor(bool isClearColor) { m_isClearColor = isClearColor; }

		// background color
		const Color& getClearColorValue() const { return m_clearColor; }
		void setClearColorValue(const Color& color) { m_clearColor = color; }

		// clear depth
		bool isClearDepth() const { return m_isClearDepth; }
		void setClearDepth(bool isClearDepth) { m_isClearDepth = isClearDepth; }

		// read pixels
		virtual bool readPixels(Attachment attach, Pixels& pixels) { return false; }

    protected:
		bool	m_isClearColor = true;
		Color	m_clearColor = Color(0.298f, 0.298f, 0.322f);
		bool	m_isClearDepth = true;
		float	m_clearDepth = 1.f;
		bool	m_isClearStencil = true; 
		ui8		m_clearStencil = 0;
    };
    typedef ResRef<FrameBuffer> FrameBufferPtr;

	class FrameBufferOffScreen : public FrameBuffer
	{
        ECHO_RES(FrameBufferOffScreen, FrameBuffer, ".fbos", FrameBufferOffScreen::create, Res::load)

	public:
        FrameBufferOffScreen();
        FrameBufferOffScreen(ui32 width, ui32 height);
		virtual ~FrameBufferOffScreen();

		// create fun
		static Res* create();

    public:
        // Attachment colorA
        ResourcePath getColorA();
        void setColorA(const ResourcePath& path);

		// Attachment colorB
		ResourcePath getColorB();
		void setColorB(const ResourcePath& path);

        // Attachment depth
        ResourcePath getDepth();
        void setDepth(const ResourcePath& path);

        // has depth attachment | color
        bool hasColorAttachment() { return m_views[int(Attachment::ColorA)]; }
        bool hasDepthAttachment() { return m_views[int(Attachment::DepthStencil)]; }

	protected:
        array<TextureRenderPtr, 9>  m_views;
	};

    class FrameBufferWindow : public FrameBuffer
    {
        ECHO_RES(FrameBufferWindow, FrameBuffer, ".fbws", FrameBufferWindow::create, Res::load)

    public:
        FrameBufferWindow() {}
        virtual ~FrameBufferWindow() {}

		// create fun
		static Res* create();

	protected:
    };
}
