#ifndef __ECHO_GLES2SHADER_H__
#define __ECHO_GLES2SHADER_H__

#include "Shader.h"

namespace Echo
{
	class GLES2ShaderGPUProxy;
	class GLES2Shader: public Shader
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const String& filename);
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size);
		~GLES2Shader();

	private:
		void create(const String& filename);

		// ≤Â»Î∫Í∂®“Â
		void insertMacros();

		// ÃÊªªinclude
		void replaceInclude();

		GLenum GLenumMap(const String& name);

#ifdef ECHO_RENDER_THREAD
	private:
		GLES2ShaderGPUProxy* m_proxy;
#else
	public:
		void setProgramHandle(ui32 hProgram);
		GLuint getShaderHandle() const;

	private:
		GLuint			m_hShader;
		GLuint			m_hProgram;
#endif
	};

}

#endif
