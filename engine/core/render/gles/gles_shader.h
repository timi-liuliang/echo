#pragma once

#include "base/shader_program.h"
#include "gles_render_base.h"

namespace Echo
{
	class GLESShaderProgram;
	class GLESShader
	{
		friend class GLESShaderProgram;

	public:
		GLESShader(ShaderProgram::ShaderType type, const String& filename);
		GLESShader(ShaderProgram::ShaderType type, const char* srcBuffer, ui32 size);
		~GLESShader();

	private:
		void create(const String& filename);

		// insert macros
		void insertMacros();

		// replace include
		void replaceInclude();

		const String& getName() { return m_filename; }

		// shader program
		void setShaderProgram(GLESShaderProgram* pProgram);
		GLESShaderProgram* getShaderProgram() const;

		// shader type
		ShaderProgram::ShaderType getShaderType() const;

		// shader type desc
		static String GetShaderTypeDesc(ShaderProgram::ShaderType type);

		// validate
		bool validate() const;

		// mapping
		GLenum GLenumMap(const String& name);

	public:
		GLuint getShaderHandle() const;

	private:
		GLESShaderProgram*			m_program = nullptr;
		ShaderProgram::ShaderType	m_shaderType;
		String						m_filename;
		String						m_srcData;
		ui32						m_shaderSize;
		bool						m_validata;
		GLuint						m_glesShader;
	};
}