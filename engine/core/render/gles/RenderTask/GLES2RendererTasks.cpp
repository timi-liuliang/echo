#include "GLES2RenderStd.h"
#include "engine/core/Math/Rect.h"
#include "GLES2RendererTasks.h"
#include "GLES2RendererProxy.h"
#include "GLES2Texture.h"
#include "GLES2TextureGPUProxy.h"
#include "GLES2Renderer.h"
#include "Render/RenderThread.h"


#ifdef ECHO_PLATFORM_WINDOWS
extern void present();
#elif defined(ECHO_PLATFORM_MAC_IOS)
extern void PresentRenderBuffer();
#endif

extern void makeContextCurrent();

namespace Echo
{
	GLES2RenderTaskVertexAttribPointer::GLES2RenderTaskVertexAttribPointer(ui32 index, int count, ui32 type, bool normalized, size_t stride, size_t offset)
		: m_index(index)
		, m_count(count)
		, m_type(type)
		, m_normalized(normalized)
		, m_stride(stride)
		, m_offset(offset)
	{
	}

	void GLES2RenderTaskVertexAttribPointer::Execute()
	{
		vertexAttribPointer(m_index, m_count, m_type, m_normalized, m_stride, m_offset);
	}

	GLES2RenderTaskEnableVertexAttribArray::GLES2RenderTaskEnableVertexAttribArray(ui32 index)
		: m_index(index)
	{
	}

	void GLES2RenderTaskEnableVertexAttribArray::Execute()
	{
		enableVertexAttribArray(m_index);
	}

	GLES2RenderTaskDisableVertexAttribArray::GLES2RenderTaskDisableVertexAttribArray(ui32 index)
		: m_index(index)
	{
	}

	void GLES2RenderTaskDisableVertexAttribArray::Execute()
	{
		disableVertexAttribArray(m_index);
	}


	void GLES2RenderTaskPresent::Execute()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		present();
#elif defined(ECHO_PLATFORM_MAC_IOS)
        PresentRenderBuffer();
#elif defined(ECHO_PLATFORM_ANDROID)
		g_render_thread->OnFrameEnd();
#endif
	}

	GLES2RenderTaskDrawElements::GLES2RenderTaskDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
		: m_mode(mode)
		, m_count(count)
		, m_type(type)
		, m_indices(indices)
	{
	}

	void GLES2RenderTaskDrawElements::Execute()
	{
		DrawElements(m_mode, m_count, m_type, m_indices);
	}


	GLES2RenderTaskDrawArrays::GLES2RenderTaskDrawArrays(GLenum mode, GLint first, GLsizei count)
		: m_mode(mode)
		, m_first(first)
		, m_count(count)
	{
	}

	void GLES2RenderTaskDrawArrays::Execute()
	{
		DrawArrays(m_mode, m_first, m_count);
	}


	GLES2RenderTaskPolygonMode::GLES2RenderTaskPolygonMode(GLenum face, GLenum mode)
		: m_face(face)
		, m_mode(mode)
	{
	}

	void GLES2RenderTaskPolygonMode::Execute()
	{
		PolygonMode(m_face, m_mode);
	}

	GLES2RenderTaskBindTexture::GLES2RenderTaskBindTexture(ui32 slot, GLenum target, GLES2TextureGPUProxy* proxy)
		: m_slot(slot)
		, m_target(target)
		, m_proxy(proxy)
	{
	}

	void GLES2RenderTaskBindTexture::Execute()
	{
		BindTexture(m_slot, m_target, m_proxy ? m_proxy->m_hTexture : 0);
	}


	GLES2RenderTaskScissor::GLES2RenderTaskScissor(GLint x, GLint y, GLsizei width, GLsizei height)
		: m_x(x)
		, m_y(y)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2RenderTaskScissor::Execute()
	{
		Scissor(m_x, m_y, m_width, m_height);
	}

	void GLES2RenderTaskEndScissor::Execute()
	{
		EndScissor();
	}


	GLES2RenderTaskSetViewport::GLES2RenderTaskSetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
		: m_x(x)
		, m_y(y)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2RenderTaskSetViewport::Execute()
	{
		SetViewport(m_x, m_y, m_width, m_height);
	}

	GLES2RenderTaskGetViewport::GLES2RenderTaskGetViewport(const IRect* rc)
		: m_rect(nullptr)
	{
		m_rect = const_cast<IRect*>(rc);
	}

	void GLES2RenderTaskGetViewport::Execute()
	{
		GLint viewPort[4];
		GetViewport(viewPort);
		m_rect->left = viewPort[0];
		m_rect->top = viewPort[1];
		m_rect->width = viewPort[2];
		m_rect->height = viewPort[3];
	}
	
	void GLES2RenderTaskMakeContextCurrent::Execute()
	{
		makeContextCurrent();
	}

	GLES2RenderTaskCheckExtension::GLES2RenderTaskCheckExtension(GLES2Renderer* renderer)
		: m_renderer(renderer)
	{
	}

	void GLES2RenderTaskCheckExtension::Execute()
	{
		m_renderer->checkOpenGLExtensions();
	}

}