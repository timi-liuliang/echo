#pragma once

#include <Foundation/Math/Matrix4.h>
#include <Render/GPUBuffer.h>
#include <Render/RenderInput.h>
#include <Render/Renderable.h>
#include <Engine/Render/TextureRes.h>
#include "spine/AtlasAttachmentLoader.h"

extern "C"
{
	typedef struct EchoAttachmentLoader 
	{
		spAttachmentLoader super;
		spAtlasAttachmentLoader* atlasAttachmentLoader;
	} EchoAttachmentLoader;

	/* The EchoAttachmentLoader must not be disposed until after the skeleton data has been disposed. */
	EchoAttachmentLoader* EchoAttachmentLoader_create(spAtlas* atlas);
}

namespace Spine
{
	/**
	 * 顶点格式定义
	 */
	struct VertexFormat
	{
		ECHO::Vector3	m_position;		// 位置
		ECHO::ui32		m_diffuse;		// 颜色值
		ECHO::Vector2	m_uv;			// UV坐标
	};

	/**
	 * Spine AttachmentVertices
	 */
	class AttachmentVertices 
	{
	public:
		AttachmentVertices( ECHO::TextureRes* texture, int verticesCount, ECHO::ui16* triangles, int indicesCount);
		virtual ~AttachmentVertices();

		// 更新内存数据到GPU
		void submitToRenderQueue( const ECHO::Matrix4& matWVP);

	private:
		// 初始化
		void init();

	public:
		ECHO::TextureRes*						m_texture;				// 纹理
		ECHO::vector<VertexFormat>::type		m_verticesData;			// 顶点数据
		ECHO::vector<ECHO::ui16>::type			m_indicesData;			// 索引数据
		ECHO::GPUBuffer*						m_vertexBuffer;			// 顶点流(表示点光源几何体)
		ECHO::GPUBuffer*						m_indexBuffer;			// 索引流
		ECHO::RenderInput*						m_renderInput;			// 输入
		ECHO::RenderInput::VertexElementList	m_verElementLists;		// 顶点格式定义
		ECHO::Renderable*						m_renderable;			// 渲染对象
		ECHO::Matrix4							m_matWVP;				// 世界观察投影矩阵
	};
}