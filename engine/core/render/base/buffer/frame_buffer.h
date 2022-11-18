#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/scene/node.h"
#include "engine/core/resource/Res.h"
#include "base/texture/texture.h"
#include "base/texture/texture_render_target_2d.h"

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
			DepthStencil,
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
				m_data.resize(PixelUtil::GetPixelBytes(m_format) * m_width * m_height);
			}
		};

    public:
		FrameBuffer();

		// begin|end render
		virtual bool begin();
		virtual bool end() { return false; }

		// on resize
		virtual void onSize(ui32 width, ui32 height) {}

	public:
		// clear color
		bool isClearColor() const { return m_isClearColor[Attachment::ColorA]; }
		void setClearColor(bool isClearColor) { m_isClearColor[Attachment::ColorA] = isClearColor; }

		// background color
		const Color& getClearColorValue() const { return m_clearColor[Attachment::ColorA]; }
		void setClearColorValue(const Color& color) { m_clearColor[Attachment::ColorA] = color; }

		// clear depth
		bool isClearDepth() const { return m_isClearDepth; }
		void setClearDepth(bool isClearDepth) { m_isClearDepth = isClearDepth; }

		// read pixels
		virtual bool readPixels(Attachment attach, Pixels& pixels) { return false; }

	public:
		// view index
		virtual i32 getViewIndex(Texture* view) { return -1; }
		virtual Texture* getViewCopy(i32 index) { return nullptr; }

    protected:
		static ResRef<FrameBuffer>				g_current;
		array<bool, Attachment::DepthStencil>	m_isClearColor;
		array<Color, Attachment::DepthStencil>	m_clearColor;

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
		virtual ~FrameBufferOffScreen();

		// create fun
		static Res* create();

    public:
        // Attachment colorA
        ResourcePath getColorA();
        void setColorA(const ResourcePath& path);

		// clear color
		bool isClearColorB() const { return m_isClearColor[Attachment::ColorB]; }
		void setClearColorB(bool isClearColor) { m_isClearColor[Attachment::ColorB] = isClearColor; }

		// background color
		const Color& getClearColorBValue() const { return m_clearColor[Attachment::ColorB]; }
		void setClearColorBValue(const Color& color) { m_clearColor[Attachment::ColorB] = color; }

		// Attachment colorB
		ResourcePath getColorB();
		void setColorB(const ResourcePath& path);

		// clear color
		bool isClearColorC() const { return m_isClearColor[Attachment::ColorC]; }
		void setClearColorC(bool isClearColor) { m_isClearColor[Attachment::ColorC] = isClearColor; }

		// background color
		const Color& getClearColorCValue() const { return m_clearColor[Attachment::ColorC]; }
		void setClearColorCValue(const Color& color) { m_clearColor[Attachment::ColorC] = color; }

		// Attachment colorB
		ResourcePath getColorC();
		void setColorC(const ResourcePath& path);

		// clear color D
		bool isClearColorD() const { return m_isClearColor[Attachment::ColorD]; }
		void setClearColorD(bool isClearColor) { m_isClearColor[Attachment::ColorD] = isClearColor; }

		// background color
		const Color& getClearColorDValue() const { return m_clearColor[Attachment::ColorD]; }
		void setClearColorDValue(const Color& color) { m_clearColor[Attachment::ColorD] = color; }

		// Attachment colorB
		ResourcePath getColorD();
		void setColorD(const ResourcePath& path);

        // Attachment depth
        ResourcePath getDepth();
        void setDepth(const ResourcePath& path);

        // has depth attachment | color
        bool hasColorAttachment() { return m_views[int(Attachment::ColorA)]; }
        bool hasDepthAttachment() { return m_views[int(Attachment::DepthStencil)]; }

	protected:
		// Get view index
		virtual i32 getViewIndex(Texture* view);
		virtual Texture* getViewCopy(i32 index) { return nullptr; }

	protected:
        array<TextureRenderTarget2DPtr, 9>	m_views;
	};
	typedef ResRef<FrameBufferOffScreen> FrameBufferOffScreenPtr;

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
