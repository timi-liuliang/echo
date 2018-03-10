#pragma once

namespace Echo
{
	void vertexAttribPointer(ui32 index, int count, ui32 type, bool normalized, size_t stride, size_t offset);

	void enableVertexAttribArray(ui32 index);

	void disableVertexAttribArray(ui32 index);

	void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

	void DrawArrays(GLenum mode, GLint first, GLsizei count);

	void PolygonMode(GLenum face, GLenum mode);

	void BindTexture(ui32 slot, GLenum target, GLuint texture);

	void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

	void EndScissor();

	void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);

	void GetViewport(GLint viewPort[4]);
}