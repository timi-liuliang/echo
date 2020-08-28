#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESRenderable.h"
#include "GLESShaderProgram.h"
#include "GLESMapping.h"
#include "GLESGPUBuffer.h"
#include <engine/core/util/AssertX.h>
#include <engine/core/util/Exception.h>
#include "engine/core/scene/render_node.h"
#include "base/image/PixelFormat.h"
#include "base/Renderer.h"
#include "GLESMapping.h"


namespace Echo
{
	extern GLES2Renderer* g_renderer;

	GLES2Renderable::GLES2Renderable(int identifier)
		: Renderable(identifier)
	{
	}

	GLES2Renderable::~GLES2Renderable()
	{
	}

	void GLES2Renderable::bindShaderParams()
	{
		ShaderProgram* shaderProgram = m_material->getShader();
		if (shaderProgram)
		{
			i32 textureCount = 0;
			for (auto& it : shaderProgram->getUniforms())
			{
				ShaderProgram::UniformPtr uniform = it.second;
				Material::UniformValue* uniformValue = m_material->getUniform(uniform->m_name);
				if (uniform->m_type != SPT_TEXTURE)
				{
					const void* value = m_node ? m_node->getGlobalUniformValue(uniform->m_name) : nullptr;
					if (!value) value = uniformValue->getValue();

					shaderProgram->setUniform(uniform->m_name.c_str(), value, uniform->m_type, uniform->m_count);
				}
				else
				{
					if (uniformValue)
					{
						Texture* texture = uniformValue->getTexture();
						if (texture)
						{
							Renderer::instance()->setTexture(textureCount, texture);
						}
					}

					shaderProgram->setUniform(uniform->m_name.c_str(), &textureCount, uniform->m_type, uniform->m_count);
					textureCount++;
				}
			}
		}
	}

	void GLES2Renderable::setMesh(MeshPtr mesh)
	{
		m_mesh = mesh;

		bindVertexStream();
	}

	void GLES2Renderable::setMaterial(Material* material)
	{
		m_material = material;

		bindVertexStream();

		material->onShaderChanged.connectClassMethod(this, createMethodBind(&GLES2Renderable::bindVertexStream));
	}

	void GLES2Renderable::bindVertexStream()
	{
		if (m_mesh && m_material && m_material->getShader())
		{
			m_vertexStreams.clear();

			StreamUnit unit;
			unit.m_vertElements = m_mesh->getVertexElements();
			unit.m_buffer = m_mesh->getVertexBuffer();
			buildVertStreamDeclaration(&unit);

			m_vertexStreams.push_back(unit);
		}
	}

	void GLES2Renderable::bind(Renderable* pre)
	{
		GPUBuffer* idxBuffer = m_mesh->getIndexBuffer();
		GPUBuffer* preIdxBuffer = pre ? pre->getMesh()->getIndexBuffer() : nullptr;

		// 1. is need set vertex buffer
		bool isNeedSetVertexBuffer;
		bool isNeedSetIdxBuffer;
		if (!pre)
		{
			isNeedSetVertexBuffer = true;
			isNeedSetIdxBuffer = idxBuffer ? true : false;
		}
		else
		{
#ifdef ECHO_PLATFORM_IOS
            isNeedSetVertexBuffer = true;//pre->getVertexStreamHash() != m_vertexStreamsHash ? true : false;
#else
			isNeedSetVertexBuffer = true;
#endif
			isNeedSetIdxBuffer = (!idxBuffer || preIdxBuffer == idxBuffer) ? false : true;
		}

		// bind vertex stream
		if (isNeedSetVertexBuffer)
		{
			for (i32 i = (i32)(m_vertexStreams.size() - 1); i >= 0; i--)
			{
				const StreamUnit& streamUnit = m_vertexStreams[i];

				((GLES2GPUBuffer*)streamUnit.m_buffer)->bindBuffer();

				size_t declarationSize = streamUnit.m_vertDeclaration.size();
				for (size_t i = 0; i < declarationSize; ++i)
				{
					const VertexDeclaration& declaration = streamUnit.m_vertDeclaration[i];
					if (declaration.m_attribute != -1)
					{
						// Enable the vertex array attributes.
						OGLESDebug(glVertexAttribPointer(declaration.m_attribute, declaration.count, declaration.type, declaration.bNormalize, streamUnit.m_vertStride, (GLvoid*)declaration.elementOffset));
						g_renderer->enableAttribLocation(declaration.m_attribute);
					}
				}
			}
		}

		// bind index buffer
		if ( isNeedSetIdxBuffer)
		{
			// Bind the index buffer and load the index data into it.
			((GLES2GPUBuffer*)idxBuffer)->bindBuffer();
		}
	}

	// unbind
	void GLES2Renderable::unbind()
	{
#ifndef ECHO_PLATFORM_IOS
		// bind vertex stream
  		for (size_t i = 0; i < m_vertexStreams.size(); i++)
  		{
  			const StreamUnit& streamUnit = m_vertexStreams[i];
  			size_t declarationSize = streamUnit.m_vertDeclaration.size();
  			for (size_t i = 0; i < declarationSize; ++i)
  			{
  				const VertexDeclaration& declaration = streamUnit.m_vertDeclaration[i];
 				if (declaration.m_attribute != -1)
 				{
 					// Enable the vertex array attributes.
					g_renderer->disableAttribLocation(declaration.m_attribute);
 				}
  			}
  		}
#endif
	}

	bool GLES2Renderable::buildVertStreamDeclaration(StreamUnit* stream)
	{
		ui32 numVertElms = static_cast<ui32>(stream->m_vertElements.size());
		if (numVertElms == 0)
		{
			EchoLogError("Vertex elements size error, buildVertStreamDeclaration failed.");
			return false;
		}

		stream->m_vertDeclaration.reserve(numVertElms);
		stream->m_vertDeclaration.resize(numVertElms);

		GLES2ShaderProgram* gles2Program = ECHO_DOWN_CAST<GLES2ShaderProgram*>(m_material->getShader());
		ui32 elmOffset = 0;
		for (size_t i = 0; i < numVertElms; ++i)
		{
			stream->m_vertDeclaration[i].m_attribute = gles2Program->getAtrribLocation(stream->m_vertElements[i].m_semantic);
			stream->m_vertDeclaration[i].count = PixelUtil::GetChannelCount(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].type = GLES2Mapping::MapDataType(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].bNormalize = PixelUtil::IsNormalized(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].elementOffset = elmOffset;
			elmOffset += PixelUtil::GetPixelSize(stream->m_vertElements[i].m_pixFmt);
		}

		// check
		for (i32 i = 0; i < VS_MAX; ++i)
		{
			i32 loc = gles2Program->getAtrribLocation((VertexSemantic)i);
			if (loc >= 0)
			{
				bool found = false;
				for (size_t size = 0; size < numVertElms; ++size)
				{
					if (stream->m_vertElements[size].m_semantic == i)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					String errorInfo = StringUtil::Format("Vertex Attribute [%s] name is NOT in Vertex Stream", GLES2Mapping::MapVertexSemanticString((VertexSemantic)i).c_str());
					EchoLogFatal(errorInfo.c_str());
					EchoAssertX(false, errorInfo.c_str());
				}
			}
		}

		stream->m_vertStride = elmOffset;

		return true;
	}
}
