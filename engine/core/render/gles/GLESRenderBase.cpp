#include "GLESRenderBase.h"

ECHO_COMPILER_ASSERT(ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_STANDARD);

namespace Echo
{
	void DebugGLError(const char* filename, int lineNum)
	{
		GLenum err = glGetError();
		switch (err)
		{
		case GL_INVALID_ENUM:
			EchoLogError("File Line Number[%s:%d] Invalid enum.", filename, lineNum);
			break;
		case GL_INVALID_VALUE:
			EchoLogError("File Line Number[%s:%d] Invalid value.", filename, lineNum);
			break;
		case GL_INVALID_OPERATION:
			EchoLogError("File Line Number[%s:%d] Invalid operation.", filename, lineNum);
			break;
		case GL_OUT_OF_MEMORY:
			EchoLogError("File Line Number[%s:%d] Out of memory.", filename, lineNum);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			EchoLogError("File Line Number[%s:%d] Invalid FrameBuffer operation.", filename, lineNum);
			break;
		}
	}
}
