#pragma once

#include <engine/core/util/Array.hpp>
#include "base/ShaderProgram.h"
#include "base/Renderable.h"
#include "base/mesh/mesh.h"
#include "GLESShader.h"
#include "GLESRenderable.h"

namespace Echo
{
	class GLES2ShaderProgram : public ShaderProgram
	{
	public:
		typedef array<GLint, VS_MAX> AttribLocationArray;
		typedef array<Shader*, Shader::ST_SHADERCOUNT> ShaderArray;

	public:
		GLES2ShaderProgram();
		~GLES2ShaderProgram();

		// attch|detach shader
		bool attachShader(Shader* pShader);
		Shader*	detachShader(Shader::ShaderType type);

		// link shader
		bool linkShaders();

		// bind Uniforms
		void bindUniforms();

		// bind|unbind
		virtual void bind() override;
		virtual void unbind() override;

		// bind Renderable
		void bindRenderable(Renderable* renderable);

		// get attribute location
		i32 getAtrribLocation(VertexSemantic vertexSemantic);

		// Create
		virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;
		void clearShaderProgram();

	public:
		// get shader
		Shader* getShader(Shader::ShaderType type) const { return m_shaders[(ui32)type]; }
	
	private:
        bool                m_isLinked = false;
		ShaderArray			m_shaders;
		GLES2Renderable*	m_preRenderable;				// Geomerty
		AttribLocationArray	m_attribLocationMapping;		// Attribute location
		GLuint				m_glesProgram = 0;
	};
}
