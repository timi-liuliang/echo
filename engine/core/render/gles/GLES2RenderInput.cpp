#include "GLES2RenderBase.h"
#include "GLES2Renderer.h"
#include "GLES2RenderInput.h"
#include "GLES2ShaderProgram.h"
#include "GLES2Mapping.h"
#include "GLES2GPUBuffer.h"
#include <engine/core/util/AssertX.h>
#include <engine/core/util/Exception.h>
#include "render/PixelFormat.h"
#include "GLES2Mapping.h"


namespace Echo
{
	extern GLES2Renderer* g_renderer;

	// 构造函数
	GLES2RenderInput::GLES2RenderInput( ShaderProgram* pProgram)
		: RenderInput( pProgram)
		, m_vao( -1)
	{
	}
	
	// 析构函数
	GLES2RenderInput::~GLES2RenderInput()
	{
	}
	
	// 绑定到GPU
	void GLES2RenderInput::bind(RenderInput* pre)
	{
		// 1.确定顶点流索引流是否需要设置
		bool isNeedSetVertexBuffer;
		bool isNeedSetIdxBuffer;
		if (!pre)
		{
			isNeedSetVertexBuffer = true;
			isNeedSetIdxBuffer = m_pIdxBuff ? true : false;
		}
		else
		{
#ifdef ECHO_PLATFORM_MAC_IOS
			isNeedSetVertexBuffer = pre->getVertexStreamHash() != m_vertexStreamsHash ? true : false;
#else
			isNeedSetVertexBuffer = true;
#endif
			isNeedSetIdxBuffer = (!m_pIdxBuff || pre->getIndexBuffer() == m_pIdxBuff) ? false : true;
		}

		// 绑定顶点流
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

		// 绑定索引流
		if ( isNeedSetIdxBuffer)
		{
			// Bind the index buffer and load the index data into it.
			((GLES2GPUBuffer*)m_pIdxBuff)->bindBuffer();
		}
	}
	
	// 去除绑定
	void GLES2RenderInput::unbind()
	{
#ifndef ECHO_PLATFORM_MAC_IOS
		// 绑定顶点流
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

		//OGLESDebug(glBindBuffer(GL_ARRAY_BUFFER, 0));

		//if(m_pIdxBuff)
		//	OGLESDebug(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}


	// 计算顶点流声明
	bool GLES2RenderInput::buildVertStreamDeclaration(StreamUnit* stream)
	{
		ui32 numVertElms = stream->m_vertElements.size();
		if (numVertElms == 0)
		{
			EchoLogError("Vertex elements size error, buildVertStreamDeclaration failed.");
			return false;
		}
			
		stream->m_vertDeclaration.reserve(numVertElms);
		stream->m_vertDeclaration.resize(numVertElms);
		
		GLES2ShaderProgram* gles2Program = ECHO_DOWN_CAST<GLES2ShaderProgram*>(m_program);
		ui32 elmOffset = 0;
		for (size_t i = 0; i < numVertElms; ++i)
		{
			stream->m_vertDeclaration[i].m_attribute = gles2Program->getAtrribLocation(stream->m_vertElements[i].m_semantic);
			stream->m_vertDeclaration[i].count = PixelUtil::GetChannelCount(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].type = GLES2Mapping::MapDataType(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].bNormalize = PixelUtil::IsNormalized(stream->m_vertElements[i].m_pixFmt);
			stream->m_vertDeclaration[i].elementOffset = elmOffset;
			elmOffset += PixelUtil::GetPixelSize(stream->m_vertElements[i].m_pixFmt);

			if (stream->m_vertDeclaration[i].m_attribute == -1)
			{
				String name = GLES2Mapping::MapVertexSemanticString(stream->m_vertElements[i].m_semantic);
//				EchoLogError("Vertex Attribute [%s] is not enable in [%s] Material", name.c_str(), gles2Program->getMaterialName().c_str());
			}
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
						String errorInfo = StringUtil::Format("Vertex Attribute [%s] name is NOT in Vertex Stream", GLES2Mapping::MapVertexSemanticString((VertexSemantic)i).c_str()) + m_ownerInfo;
						EchoLogFatal(errorInfo.c_str());
						EchoAssertX(false, errorInfo.c_str());
					}
				}
			}
		}


		stream->m_vertStride = elmOffset;

		return true;
	}

	// 生成顶点流Hash值(BKDR Hash)
	void GLES2RenderInput::generateVertexStreamHash()
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
