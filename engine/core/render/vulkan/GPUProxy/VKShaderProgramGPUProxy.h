#pragma once

namespace LORD
{

	class GLES2ShaderProgramGPUProxy
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2ShaderProgramGPUProxy();
		~GLES2ShaderProgramGPUProxy();

		void linkShaders();
		void uniformsReflection(ShaderProgram::UniformArray* uniform_array);
		void bindUniforms(ShaderParamType type, int location, int count, Byte* value);
		void createProgram();
		void deleteProgram();
		void attachShader(uint shader);
		void detachShader(uint shader);
		void useProgram();

	private:
		GLuint				m_hProgram;
		GLint				m_link_status;
	};
}