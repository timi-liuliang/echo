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
	{
        m_views.assign(nullptr);
	}

	FrameBufferOffScreen::~FrameBufferOffScreen()
	{
	}

	void FrameBufferOffScreen::bindMethods()
	{
		CLASS_BIND_METHOD(FrameBufferOffScreen, getColor0, DEF_METHOD("getColor0"));
		CLASS_BIND_METHOD(FrameBufferOffScreen, setColor0, DEF_METHOD("setColor0"));

		CLASS_REGISTER_PROPERTY(FrameBufferOffScreen, "Color0", Variant::Type::ResourcePath, "getColor0", "setColor0");
	}

	Res* FrameBufferOffScreen::create()
	{
		static i32 idx = 0; idx++;
		return Renderer::instance()->createFrameBufferOffScreen(Renderer::instance()->getWindowWidth(), Renderer::instance()->getWindowHeight());
	}

	ResourcePath FrameBufferOffScreen::getColor0()
	{
		return m_views[0] ? ResourcePath(m_views[0]->getPath(), ".rt") : ResourcePath("", ".rt");
	}

	void FrameBufferOffScreen::setColor0(const ResourcePath& path)
	{
		m_views[0] = ECHO_DOWN_CAST<TextureRender*>(Res::get(path));
		attach(Attachment::Color0, m_views[0]);
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
