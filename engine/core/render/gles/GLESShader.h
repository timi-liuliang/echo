#pragma once

#include "interface/Shader.h"

namespace Echo
{
	class GLES2Shader: public Shader
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const String& filename);
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size);
		~GLES2Shader();

	private:
		void create(const String& filename);

		// insert macros
		void insertMacros();

		// replace include
		void replaceInclude();

		// mapping
		GLenum GLenumMap(const String& name);

	public:
		void setProgramHandle(ui32 hProgram);
		GLuint getShaderHandle() const;

	private:
		GLuint			m_hShader;
		GLuint			m_hProgram;
	};
}