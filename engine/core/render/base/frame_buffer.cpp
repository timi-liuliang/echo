#include "renderer.h"
#include "base/frame_buffer.h"
#include "base/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	void FrameBuffer::bindMethods()
	{
		CLASS_BIND_METHOD(FrameBuffer, isClearColor, DEF_METHOD("isClearColor"));
		CLASS_BIND_METHOD(FrameBuffer, setClearColor, DEF_METHOD("setClearColor"));
		CLASS_BIND_METHOD(FrameBuffer, isClearDepth, DEF_METHOD("isClearDepth"));
		CLASS_BIND_METHOD(FrameBuffer, setClearDepth, DEF_METHOD("setClearDepth"));

		CLASS_REGISTER_PROPERTY(FrameBuffer, "IsClearColor", Variant::Type::Bool, "isClearColor", "setClearColor");
		CLASS_REGISTER_PROPERTY(FrameBuffer, "IsClearDepth", Variant::Type::Bool, "isClearDepth", "setClearDepth");
	}

	FrameBufferOffScreen::FrameBufferOffScreen()
    {
        m_views.assign(nullptr);
    }

	FrameBufferOffScreen::FrameBufferOffScreen(ui32 width, ui32 height)
		: m_width(width)
        , m_height(height)
	{
        m_views.assign(nullptr);
	}

	FrameBufferOffScreen::~FrameBufferOffScreen()
	{
	}

	void FrameBufferOffScreen::bindMethods()
	{

	}

	Res* FrameBufferOffScreen::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createFrameBufferOffScreen(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight());
	}

	Res* FrameBufferWindow::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createFrameBufferWindow();
	}

	void FrameBufferWindow::bindMethods()
	{

	}
}
