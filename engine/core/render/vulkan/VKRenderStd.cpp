#include "GLES2RenderStd.h"

LORD_COMPILER_ASSERT(LORD_STL_MEMORY_ALLOCATOR == LORD_STL_MEMORY_ALLOCATOR_STANDARD);

namespace LORD
{
	// Debug GL º¯Êýµ÷ÓÃ
	void DebugGLError(const char* filename, int lineNum)
	{
		GLenum err = glGetError();
		switch (err)
		{
		case GL_INVALID_ENUM:
			LordLogError("File Line Number[%s:%d] Invalid enum.", filename, lineNum);
			break;
		case GL_INVALID_VALUE:
			LordLogError("File Line Number[%s:%d] Invalid value.", filename, lineNum);
			break;
		case GL_INVALID_OPERATION:
			LordLogError("File Line Number[%s:%d] Invalid operation.", filename, lineNum);
			break;
		case GL_OUT_OF_MEMORY:
			LordLogError("File Line Number[%s:%d] Out of memory.", filename, lineNum);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			LordLogError("File Line Number[%s:%d] Invalid FrameBuffer operation.", filename, lineNum);
			break;
		}
	}
}