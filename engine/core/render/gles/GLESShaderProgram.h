#pragma once

#include <engine/core/util/Array.hpp>
#include "interface/ShaderProgram.h"
#include "interface/Renderable.h"
#include "interface/mesh/Mesh.h"
#include "GLESShader.h"
#include "GLESRenderable.h"

namespace Echo
{
	class GLES2ShaderProgram : public ShaderProgram
	{
	public:
		typedef array<GLint, VS_MAX> AttribLocationArray;
	public:
		GLES2ShaderProgram( ShaderProgramRes* material);
		~GLES2ShaderProgram();

		// 附加shader
		virtual bool attachShader(Shader* pShader) override;

		// 移除shader
		virtual Shader*	detachShader(Shader::ShaderType type) override;

		// 链接shader
		virtual bool linkShaders() override;

		// 应用变量
		virtual void bindUniforms() override;

		// 绑定
		virtual void bind() override;

		// 解绑
		virtual void unbind() override;

		// 绑定几何体数据
		virtual void bindRenderable(Renderable* renderable) override;

		// 获取顶点属性物理位置
		i32 getAtrribLocation(VertexSemantic vertexSemantic) override;

		// 获取材质名
		const String& getMaterialName();
	
	private:
		GLES2Renderable*	m_preRenderable;				// 几何体数据流
		AttribLocationArray	m_attribLocationMapping;		// 属性物理位置映射
		GLuint			m_hProgram;
	};

}
