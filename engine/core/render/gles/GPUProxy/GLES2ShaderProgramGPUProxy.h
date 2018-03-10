#pragma once
#include "GLES2ShaderProgram.h"

namespace Echo
{

	class GLES2ShaderProgramGPUProxy
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2ShaderProgramGPUProxy();
		~GLES2ShaderProgramGPUProxy();

		void linkShaders();
		void uniformsReflection(ShaderProgram::UniformArray* uniform_array);
		void attribReflection(GLES2ShaderProgram::AttribLocationArray* attrib_array);
		void bindUniforms(ShaderParamType type, int location, int count, Byte* value);
		void createProgram();
		void deleteProgram();
		void attachShader(ui32 shader);
		void detachShader(ui32 shader);
		void useProgram();

	private:
		GLuint				m_hProgram;
		GLint				m_link_status;
	};
}