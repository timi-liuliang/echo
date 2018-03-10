#ifndef __LORD_GLES2SHADER_H__
#define __LORD_GLES2SHADER_H__

#include "Render/Shader.h"

namespace LORD
{
	class GLES2ShaderGPUProxy;

	class LORD_GLES2RENDER_API GLES2Shader: public Shader
	{
		friend class GLES2ShaderProgram;
	public:
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const String& filename);
		GLES2Shader(ShaderType type, const ShaderDesc& desc, const Buffer& buff);
		~GLES2Shader();

	private:
		void create(const String& filename);

	private:
		GLES2ShaderGPUProxy* m_proxy;
	};

}

#endif
