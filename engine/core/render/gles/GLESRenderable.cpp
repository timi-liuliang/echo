#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESRenderable.h"
#include "GLESShaderProgram.h"
#include "GLESMapping.h"
#include "GLESGPUBuffer.h"
#include <engine/core/util/AssertX.h>
#include <engine/core/util/Exception.h>
#include "engine/core/scene/render_node.h"
#include "interface/image/PixelFormat.h"
#include "interface/Renderer.h"
#include "GLESMapping.h"


namespace Echo
{
	extern GLES2Renderer* g_renderer;

	GLES2Renderable::GLES2Renderable(const String& renderStage, ShaderProgram* shader, int identifier)
		: Renderable( renderStage, shader, identifier)
		, m_vao( -1)
	{
	}

	GLES2Renderable::~GLES2Renderable()
	{
		m_shaderParams.clear();
	}

	void GLES2Renderable::bindShaderParams()
	{
		bindTextures();

		if (m_shaderProgram)
		{
			for (auto& it : m_shaderParams)
			{
				ShaderParam& param = it.second;
				switch (param.type)
				{
				case SPT_VEC4:
				case SPT_MAT4:
				case SPT_INT:
				case SPT_FLOAT:
				case SPT_VEC2:
				case SPT_VEC3:
				case SPT_TEXTURE:	m_shaderProgram->setUniform(param.name.c_str(), param.data, param.type, param.length);	break;
				default:			EchoLogError("unknow shader param format! %s", m_node->getName().c_str());				break;
				}
			}
		}
	}

	void GLES2Renderable::setMesh(Mesh* mesh)
	{
		m_mesh = mesh;

		bindVertexStream(m_mesh->getVertexElements(), m_mesh->getVertexBuffer());
	}

	bool GLES2Renderable::bindVertexStream(const VertexElementList& vertElements, GPUBuffer* vertexBuffer, int flag)
	{
		if (flag & BS_BEGIN)
			m_vertexStreams.clear();

		StreamUnit unit;
		unit.m_vertElements = vertElements;
		unit.m_buffer = vertexBuffer;
		buildVertStreamDeclaration(&unit);

		m_vertexStreams.push_back(unit);

		// generate hash value
		if (flag & BS_END)
			generateVertexStreamHash();

		return true;
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

		GLES2ShaderProgram* gles2Program = ECHO_DOWN_CAST<GLES2ShaderProgram*>(m_shaderProgram.ptr());
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

		if (false == m_is_muti_stream)
		{
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
		}

		stream->m_vertStride = elmOffset;

		return true;
	}

	void GLES2Renderable::generateVertexStreamHash()
	{
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
		unsigned int hash = 0;
		for (int i = (int)(m_vertexStreams.size() - 1); i >= 0; i--)
		{
			const StreamUnit& streamUnit = m_vertexStreams[i];

			hash = hash * seed + (size_t)(streamUnit.m_buffer);

			size_t declarationSize = streamUnit.m_vertDeclaration.size();
			for (size_t i = 0; i < declarationSize; ++i)
			{
				const VertexDeclaration& declaration = streamUnit.m_vertDeclaration[i];
				if (declaration.m_attribute != -1)
				{
					// Enable the vertex array attributes.
					hash = hash * seed + declaration.m_attribute;
					hash = hash * seed + declaration.count;
					hash = hash * seed + declaration.type;
					hash = hash * seed + declaration.bNormalize;
					hash = hash * seed + streamUnit.m_vertStride;
					hash = hash * seed + declaration.elementOffset;
				}
			}
		}

		m_vertexStreamsHash = (hash & 0x7FFFFFFF);
	}
}
