#pragma once

namespace LORD
{
	void vertexAttribPointer(uint index, int count, uint type, bool normalized, size_t stride, size_t offset);

	void enableVertexAttribArray(uint index);

	void disableVertexAttribArray(uint index);

	void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

	void DrawArrays(GLenum mode, GLint first, GLsizei count);

	void PolygonMode(GLenum face, GLenum mode);

	void BindTexture(uint slot, GLenum target, GLuint texture);

	void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

	void EndScissor();

	void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);

	void GetViewport(GLint viewPort[4]);
}