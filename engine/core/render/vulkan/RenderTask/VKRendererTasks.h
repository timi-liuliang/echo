#pragma once

namespace LORD
{
	class GLES2TextureGPUProxy;

	class GLES2RenderTaskVertexAttribPointer
	{
	public:
		GLES2RenderTaskVertexAttribPointer(uint index, int count, uint type, bool normalized, size_t stride, size_t offset);
		void Execute();
	private:
		uint m_index;
		int m_count;
		uint m_type;
		bool m_normalized; 
		size_t m_stride;
		size_t m_offset;
	};

	class GLES2RenderTaskEnableVertexAttribArray
	{
	public:
		GLES2RenderTaskEnableVertexAttribArray(uint index);
		void Execute();
	private:
		uint m_index;
	};

	class GLES2RenderTaskDisableVertexAttribArray
	{
	public:
		GLES2RenderTaskDisableVertexAttribArray(uint index);
		void Execute();
	private:
		uint m_index;
	};

	class GLES2RenderTaskPresent
	{
	public:
		void Execute();
	};

	class GLES2RenderTaskDrawElements
	{
	public:
		GLES2RenderTaskDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
		void Execute();
	private:
		GLenum m_mode;
		GLsizei m_count;
		GLenum m_type;
		const GLvoid* m_indices;
	};

	class GLES2RenderTaskDrawArrays
	{
	public:
		GLES2RenderTaskDrawArrays(GLenum mode, GLint first, GLsizei count);
		void Execute();
	private:
		GLenum m_mode;
		GLint m_first;
		GLsizei m_count;
	};

	class GLES2RenderTaskPolygonMode
	{
	public:
		GLES2RenderTaskPolygonMode(GLenum face, GLenum mode);
		void Execute();
	private:
		GLenum m_face;
		GLenum m_mode;
	};

	class GLES2RenderTaskBindTexture
	{
	public:
		GLES2RenderTaskBindTexture(uint slot, GLenum target, GLES2TextureGPUProxy* proxy);
		void Execute();
	private:
		uint m_slot;
		GLenum m_target;
		GLES2TextureGPUProxy* m_proxy;
	};

	class GLES2RenderTaskScissor
	{
	public:
		GLES2RenderTaskScissor(GLint x, GLint y, GLsizei width, GLsizei height);
		void Execute();
	private:
		GLint m_x;
		GLint m_y;
		GLsizei m_width;
		GLsizei m_height;
	};

	class GLES2RenderTaskEndScissor
	{
	public:
		void Execute();
	};

	class GLES2RenderTaskSetViewport
	{
	public:
		GLES2RenderTaskSetViewport(GLint x, GLint y, GLsizei width, GLsizei height);
		void Execute();
	private:
		GLint m_x; 
		GLint m_y;
		GLsizei m_width;
		GLsizei m_height;
	};

	typedef TRect<int> IRect;
	class GLES2RenderTaskGetViewport
	{
	public:
		GLES2RenderTaskGetViewport(const IRect* rc);
		void Execute();
	private:
		IRect* m_rect;
	};
}