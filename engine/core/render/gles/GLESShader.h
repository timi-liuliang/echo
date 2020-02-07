#pragma once

#include "Shader.h"

namespace Echo
{
	class GLES2Shader: public Shader
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2Shader(ShaderType type, const String& filename);
		GLES2Shader(ShaderType type, const char* srcBuffer, ui32 size);
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
		GLuint getShaderHandle() const;

	private:
		GLuint			m_hShader;
	};
}