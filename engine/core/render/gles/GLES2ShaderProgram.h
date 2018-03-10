#ifndef __ECHO_GLES2SHADERPROGRAM_H__
#define __ECHO_GLES2SHADERPROGRAM_H__

#include "GLES2Shader.h"
#include "Render/ShaderProgram.h"
#include "Render/Renderable.h"
#include <engine/core/Util/Array.hpp>

namespace Echo
{
	class GLES2ShaderProgramGPUProxy;

	/**
	 * 着色器程序
	 */
	class ECHO_GLES2RENDER_API GLES2ShaderProgram: public ShaderProgram
	{
	public:
		typedef array<GLint, RenderInput::VS_MAX> AttribLocationArray;
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

		// 获取顶点属性物理位置
		i32 getAtrribLocation(RenderInput::VertexSemantic vertexSemantic) override;

		// 获取材质名
		const String& getMaterialName();
	
	private:
		RenderInput*		m_preRenderInput;				// 几何体数据流
		AttribLocationArray	m_attribLocationMapping;		// 属性物理位置映射

#ifdef ECHO_RENDER_THREAD
		GLES2ShaderProgramGPUProxy* m_gpu_proxy;
#else
		GLuint			m_hProgram;
#endif
	};

}

#endif
