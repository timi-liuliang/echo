#ifndef __LORD_GLES2SHADERPROGRAM_H__
#define __LORD_GLES2SHADERPROGRAM_H__

#include "GLES2Shader.h"
#include "Render/ShaderProgram.h"
#include "Render/Renderable.h"
#include <Foundation/Util/Array.hpp>

namespace LORD
{
	class GLES2ShaderProgramGPUProxy;

	/**
	 * 着色器程序
	 */
	class LORD_GLES2RENDER_API GLES2ShaderProgram: public ShaderProgram
	{
	public:
		GLES2ShaderProgram( Material* material);
		~GLES2ShaderProgram();

		// 附加shader
		bool attachShader(Shader* pShader);

		// 移除shader
		Shader*	detachShader(Shader::ShaderType type);

		// 链接shader
		bool linkShaders();

		// 应用变量
		virtual void bindUniforms();

		// 绑定
		void bind();

		// 解绑
		void unbind();

		// 绑定几何体数据
		virtual void bindRenderInput(RenderInput* renderInput);
	
	private:
		RenderInput*		m_preRenderInput;				// 几何体数据流
		GLES2ShaderProgramGPUProxy* m_gpu_proxy;
	};

}

#endif
