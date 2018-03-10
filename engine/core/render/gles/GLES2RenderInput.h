#pragma once

#include "Render/RenderInput.h"

namespace Echo
{
	/**
	 * 渲染输入
	 */
	class ECHO_GLES2RENDER_API GLES2RenderInput: public RenderInput
	{
	public:
		GLES2RenderInput( ShaderProgram* pProgram);
		~GLES2RenderInput();

		// 绑定几何体数据
		virtual void bind( RenderInput* pre);

		// 解绑几何体数据
		void unbind();

	private:
		// 构建顶点属性
		void buildVertexAttributes();

		// 计算顶点流声明
		virtual bool buildVertStreamDeclaration(StreamUnit* stream);

		// 生成顶点流Hash值(BKDR Hash)
		virtual void generateVertexStreamHash();

	private:
		GLuint			m_vao;							// 顶点数组(OpenGLES 3.0)
	};
}
