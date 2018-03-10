#include "EchoSpineAttachment.h"
#include "spine/extension.h"
#include <Render/Renderer.h>
#include <Render/Material.h>
#include <Engine/Scene/SceneManager.h>
#include <Engine/Render/RenderStage/RenderStageManager.h>
#include <Engine/Render/RenderStage/SpineRenderStage.h>

using namespace Spine;

static ECHO::ui16 quadTriangles[6] = { 0, 1, 2, 2, 3, 0 };

// 创建Attachment
spAttachment* EchoAttachmentLoader_createAttachment(spAttachmentLoader* loader, spSkin* skin, spAttachmentType type,const char* name, const char* path) 
{
	EchoAttachmentLoader* self = SUB_CAST(EchoAttachmentLoader, loader);
	return spAttachmentLoader_createAttachment(SUPER(self->atlasAttachmentLoader), skin, type, name, path);
}

void EchoAttachmentLoader_configureAttachment(spAttachmentLoader* loader, spAttachment* attachment) 
{
	attachment->attachmentLoader = loader;

	switch (attachment->type) 
	{
	case SP_ATTACHMENT_REGION:
		{
			spRegionAttachment* regionAttachment = SUB_CAST(spRegionAttachment, attachment);
			spAtlasRegion* region = (spAtlasRegion*)regionAttachment->rendererObject;
			AttachmentVertices* attachmentVertices = new AttachmentVertices((ECHO::TextureRes*)region->page->rendererObject, 4, quadTriangles, 6);
			
			Spine::VertexFormat* vertices = attachmentVertices->m_verticesData.data();
			for (int i = 0, ii = 0; i < 4; ++i, ii += 2) 
			{
			   vertices[i].m_uv.x = regionAttachment->uvs[ii];
			   vertices[i].m_uv.y = regionAttachment->uvs[ii + 1];
			}
			regionAttachment->rendererObject = attachmentVertices;
			break;
		}
	case SP_ATTACHMENT_MESH:
		{
			spMeshAttachment* meshAttachment = SUB_CAST(spMeshAttachment, attachment);
			spAtlasRegion* region = (spAtlasRegion*)meshAttachment->rendererObject;
			AttachmentVertices* attachmentVertices = new AttachmentVertices((ECHO::TextureRes*)region->page->rendererObject, meshAttachment->super.worldVerticesLength >> 1, meshAttachment->triangles, meshAttachment->trianglesCount);
			
			Spine::VertexFormat* vertices = attachmentVertices->m_verticesData.data();
			for (int i = 0, ii = 0, nn = meshAttachment->super.worldVerticesLength; ii < nn; ++i, ii += 2) 
			{
				 vertices[i].m_uv.x = meshAttachment->uvs[ii];
				 vertices[i].m_uv.y = meshAttachment->uvs[ii + 1];
			 }
								
			meshAttachment->rendererObject = attachmentVertices;
			break;
		}
	default:;
	}
}

// Dispose Attachment
void EchoAttachmentLoader_disposeAttachment(spAttachmentLoader* loader, spAttachment* attachment) 
{
	switch (attachment->type) 
	{
	case SP_ATTACHMENT_REGION: 
		{
			spRegionAttachment* regionAttachment = SUB_CAST(spRegionAttachment, attachment);
			delete (AttachmentVertices*)regionAttachment->rendererObject;
		}
		break;
	case SP_ATTACHMENT_MESH: 
		{
			spMeshAttachment* meshAttachment = SUB_CAST(spMeshAttachment, attachment);
			delete (AttachmentVertices*)meshAttachment->rendererObject;		
		}
		break;
	default:;
	}
}

// dispose AttachmentLoader
void EchoAttachmentLoader_dispose(spAttachmentLoader* loader) 
{
	EchoAttachmentLoader* self = SUB_CAST(EchoAttachmentLoader, loader);
	spAttachmentLoader_dispose(SUPER_CAST(spAttachmentLoader, self->atlasAttachmentLoader));
	_spAttachmentLoader_deinit(loader);
}

/* The EchoAttachmentLoader must not be disposed until after the skeleton data has been disposed. */
EchoAttachmentLoader* EchoAttachmentLoader_create(spAtlas* atlas)
{
	EchoAttachmentLoader* self = new(EchoAttachmentLoader);
	_spAttachmentLoader_init(SUPER(self), EchoAttachmentLoader_dispose, EchoAttachmentLoader_createAttachment, EchoAttachmentLoader_configureAttachment, EchoAttachmentLoader_disposeAttachment);
	self->atlasAttachmentLoader = spAtlasAttachmentLoader_create(atlas);
	return self;
}

namespace Spine
{
	// 构造函数
	AttachmentVertices::AttachmentVertices(ECHO::TextureRes* texture, int verticesCount, ECHO::ui16* triangles, int indicesCount)
		: m_vertexBuffer(nullptr)
		, m_indexBuffer(nullptr)
		, m_renderInput(nullptr)
		, m_renderable(nullptr)
	{
		m_texture = texture;

		// 顶点数据内存分配
		m_verticesData.resize(verticesCount);

		// 记录索引数据
		m_indicesData.resize(indicesCount);
		for (int i = 0; i < indicesCount; i++)
		{
			m_indicesData[i] = triangles[i];
		}

		init();
	}

	// 析构函数
	AttachmentVertices::~AttachmentVertices()
	{
	}

	// 初始化
	void AttachmentVertices::init()
	{
		ECHO::Renderer* lordrender = ECHO::Renderer::instance();

		// 顶点格式
		m_verElementLists.push_back(ECHO::RenderInput::VertexElement(ECHO::RenderInput::VS_POSITION, ECHO::PF_RGB32_FLOAT));
		m_verElementLists.push_back(ECHO::RenderInput::VertexElement(ECHO::RenderInput::VS_COLOR, ECHO::PF_RGBA8_UNORM));
		m_verElementLists.push_back(ECHO::RenderInput::VertexElement(ECHO::RenderInput::VS_TEXCOORD0, ECHO::PF_RG32_FLOAT));

		//创建GPUBuffer
		if (!m_vertexBuffer)
		{
			ECHO::Buffer vertexBuffer(sizeof(VertexFormat)*m_verticesData.size(), m_verticesData.data());
			ECHO::Buffer indexBuffer(sizeof(ECHO::ui16)*m_indicesData.size(), m_indicesData.data());
			m_vertexBuffer = lordrender->createVertexBuffer(ECHO::GPUBuffer::GBU_GPU_READ, vertexBuffer);
			m_indexBuffer = lordrender->createIndexBuffer(ECHO::GPUBuffer::GBU_GPU_READ, indexBuffer);
		}

		ECHO::RenderQueue*   renderQueue = ECHO::SceneManager::instance()->getRenderQueue("Spine");
		//renderQueue->setIsSort(false);
		m_renderable = ECHO::Renderer::instance()->createRenderable(renderQueue, NULL);
		ECHO::Material*      material = renderQueue->getMaterial();
		ECHO::ShaderProgram* shaderProgram = material->getShaderProgram();
		m_renderInput = lordrender->createRenderInput(material->getShaderProgram());
		m_renderInput->bindVertexStream(m_verElementLists, m_vertexBuffer);
		m_renderInput->bindIndexStream(m_indexBuffer, sizeof(ECHO::ui16));
		m_renderable->beginShaderParams(2);
		m_renderable->setShaderParam(shaderProgram->getParamPhysicsIndex("matWVP"), ECHO::SPT_MAT4, (void*)&m_matWVP);
		m_renderable->setShaderParam(shaderProgram->getParamPhysicsIndex("DiffuseSampler"), ECHO::SPT_TEXTURE, &ECHO::SHADER_TEXTURE_SLOT0, 1);
		m_renderable->setTexture(ECHO::SHADER_TEXTURE_SLOT0, m_texture->getTexture(), material->getSamplerState( 0));
		m_renderable->endShaderParams();
		m_renderable->setRenderInput(m_renderInput);
	}

	// 更新内存数据到GPU
	void AttachmentVertices::submitToRenderQueue( const ECHO::Matrix4& matWVP)
	{
		m_matWVP = matWVP;

		ECHO::Buffer vertexBuffer(sizeof(VertexFormat)*m_verticesData.size(), m_verticesData.data());
		m_vertexBuffer->updateData(vertexBuffer);

		ECHO::SpineRenderStage* stage = ECHO_DOWN_CAST<ECHO::SpineRenderStage*>(ECHO::RenderStageManager::instance()->getRenderStageByID(ECHO::RSI_Spine));
		stage->addRenderable(m_renderable->getIdentifier());
	}
}