#include "AttachmentLoader.h"
#include <spine/extension.h>
#include "engine/core/render/TextureRes.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"

// 默认材质
static const char* g_spinDefaultMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
<material>
<vs>#version 100

attribute vec3 a_Position;
attribute vec2 a_UV;

uniform mat4 u_WVPMatrix;

varying vec2 texCoord;

void main(void)
{
	vec4 position = u_WVPMatrix * vec4(a_Position, 1.0);
	gl_Position = position;

	texCoord = a_UV;
}
</vs>
<ps>#version 100

uniform sampler2D u_BaseColorSampler;
varying mediump vec2 texCoord;

void main(void)
{
	mediump vec4 textureColor = texture2D(u_BaseColorSampler, texCoord);
	gl_FragColor = textureColor;
}
	</ps>
	<BlendState>
		<BlendEnable value = "true" />
		<SrcBlend value = "BF_SRC_ALPHA" />
		<DstBlend value = "BF_INV_SRC_ALPHA" />
	</BlendState>
	<RasterizerState>
		<CullMode value = "CULL_NONE" />
	</RasterizerState>
	<DepthStencilState>
		<DepthEnable value = "false" />
		<WriteDepth value = "false" />
	</DepthStencilState>
	<SamplerState>
		<BiLinearMirror>
			<MinFilter value = "FO_LINEAR" />
			<MagFilter value = "FO_LINEAR" />
			<MipFilter value = "FO_NONE" />
			<AddrUMode value = "AM_CLAMP" />
			<AddrVMode value = "AM_CLAMP" />
		</BiLinearMirror>
	</SamplerState>
	<Texture>
		<stage no = "0" sampler = "BiLinearMirror" />
	</Texture>
</material>
)";

using namespace Echo;

static ui16 quadTriangles[6] = { 0, 1, 2, 2, 3, 0 };

spAttachment* EchoAttachmentLoader_createAttachment(spAttachmentLoader* loader, spSkin* skin, spAttachmentType type, const char* name, const char* path)
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
		AttachmentVertices* attachmentVertices = new AttachmentVertices((Echo::TextureRes*)region->page->rendererObject, 4, quadTriangles, 6);

		SpineVertexFormat* vertices = attachmentVertices->m_verticesData.data();
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
		AttachmentVertices* attachmentVertices = new AttachmentVertices((Echo::TextureRes*)region->page->rendererObject, meshAttachment->super.worldVerticesLength >> 1, meshAttachment->triangles, meshAttachment->trianglesCount);

		SpineVertexFormat* vertices = attachmentVertices->m_verticesData.data();
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

namespace Echo
{
	AttachmentVertices::AttachmentVertices(TextureRes* texture, int verticesCount, ui16* triangles, int indicesCount)
		: m_renderable(nullptr)
	{
		m_texture = texture;

		// 顶点数据内存分配
		m_verticesData.resize(verticesCount);

		// 记录索引数据
		m_indicesData.resize(indicesCount);
		for (int i = 0; i < indicesCount; i++)
			m_indicesData[i] = triangles[i];

	}

	AttachmentVertices::~AttachmentVertices()
	{
	}

	// render
	void AttachmentVertices::submitToRenderQueue(Node* node)
	{
		if (!m_renderable)
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(m_indicesData.size(), m_indicesData.data());
			m_mesh->updateVertexs(define, m_verticesData.size(), (const Byte*)m_verticesData.data(), Box());

			m_materialInst = MaterialInst::create();
			m_materialInst->setOfficialMaterialContent(g_spinDefaultMaterial);
			m_materialInst->setRenderStage("Transparent");
			m_materialInst->applyLoadedData();

			m_materialInst->setTexture("u_BaseColorSampler", m_texture);

			m_renderable = Renderable::create(m_mesh, m_materialInst, node);
			m_renderable->submitToRenderQueue();
		}
		else
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices(m_indicesData.size(), m_indicesData.data());
			m_mesh->updateVertexs(define, m_verticesData.size(), (const Byte*)m_verticesData.data(), Box());

			m_renderable->submitToRenderQueue();
		}
	}
}