//#ifndef __ECHO_GLES2SHADERPROGRAM_H__
//#define __ECHO_GLES2SHADERPROGRAM_H__
//
//#include "GLES2Shader.h"
//#include "render/ShaderProgram.h"
//#include "render/Renderable.h"
//#include <engine/core/util/Array.hpp>
//
//namespace Echo
//{
//	class GLES2ShaderProgramGPUProxy;
//	class GLES2ShaderProgram : public ShaderProgram
//	{
//	public:
//		typedef array<GLint, RenderInput::VS_MAX> AttribLocationArray;
//	public:
//		GLES2ShaderProgram( ShaderProgramRes* material);
//		~GLES2ShaderProgram();
//
//		// 附加shader
//		virtual bool attachShader(Shader* pShader) override;
//
//		// 移除shader
//		virtual Shader*	detachShader(Shader::ShaderType type) override;
//
//		// 链接shader
//		virtual bool linkShaders() override;
//
//		// 应用变量
//		virtual void bindUniforms() override;
//
//		// 绑定
//		virtual void bind() override;
//
//		// 解绑
//		virtual void unbind() override;
//
//		// 绑定几何体数据
//		virtual void bindRenderInput(RenderInput* renderInput) override;
//
//		// 获取顶点属性物理位置
//		i32 getAtrribLocation(RenderInput::VertexSemantic vertexSemantic) override;
//
//		// 获取材质名
//		const String& getMaterialName();
//	
//	private:
//		RenderInput*		m_preRenderInput;				// 几何体数据流
//		AttribLocationArray	m_attribLocationMapping;		// 属性物理位置映射
//
//#ifdef ECHO_RENDER_THREAD
//		GLES2ShaderProgramGPUProxy* m_gpu_proxy;
//#else
//		GLuint			m_hProgram;
//#endif
//	};
//
//}
//
//#endif
