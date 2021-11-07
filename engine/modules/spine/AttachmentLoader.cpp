#include "AttachmentLoader.h"
#include <spine/extension.h>
#include "engine/core/log/Log.h"
#include "engine/core/render/base/texture/texture.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/shader/material.h"

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
		AttachmentVertices* attachmentVertices = new AttachmentVertices((Echo::Texture*)region->page->rendererObject, 4, quadTriangles, 6);

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
		AttachmentVertices* attachmentVertices = new AttachmentVertices((Echo::Texture*)region->page->rendererObject, meshAttachment->super.worldVerticesLength >> 1, meshAttachment->triangles, meshAttachment->trianglesCount);

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
	AttachmentVertices::AttachmentVertices()
		: m_texture(nullptr)
	{

	}

	AttachmentVertices::AttachmentVertices(Texture* texture, int verticesCount, ui16* triangles, int indicesCount)
	{
		m_texture = texture;

		m_verticesData.resize(verticesCount);
		m_indicesData.resize(indicesCount);
		for (int i = 0; i < indicesCount; i++)
			m_indicesData[i] = triangles[i];
	}

	AttachmentVertices::~AttachmentVertices()
	{
	}

	// render
	void AttachmentVertices::merge(const AttachmentVertices& other)
	{
		// indices
		ui16 vertOffset = static_cast<ui16>(m_verticesData.size());
		for (size_t i = 0; i < other.m_indicesData.size(); i++)
		{
			m_indicesData.push_back(vertOffset + other.m_indicesData[i]);
		}

		// vertices
		m_verticesData.insert(m_verticesData.end(), other.m_verticesData.begin(), other.m_verticesData.end());

		// texture
		if (!m_texture)
		{
			m_texture = other.m_texture;
		}
		else if (m_texture != other.m_texture)
		{
			EchoLogError("Spine merge failed. used different texture");
		}
	}

	// clear
	void AttachmentVertices::clear()
	{
		m_verticesData.clear();
		m_indicesData.clear();
	}
}
