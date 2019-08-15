#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLESRenderBase.h"
#include "GLESMapping.h"
#include "GLESRenderView.h"
#include "GLESTexture2D.h"

namespace Echo
{
    GLESRenderView::GLESRenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
		: RenderView(width, height, pixelFormat)
	{
		SamplerState::SamplerDesc desc;
		desc.addrUMode = SamplerState::AM_CLAMP;
		desc.addrVMode = SamplerState::AM_CLAMP;
		desc.addrWMode = SamplerState::AM_CLAMP;
		desc.mipFilter = SamplerState::FO_NONE;

        static ui32 id = 0;
		m_bindTexture = Renderer::instance()->createTexture2D("rv_" + StringUtil::ToString(id++));
		m_bindTexture->setSamplerState(desc);

        create();
	}

    GLESRenderView::~GLESRenderView()
	{
		m_bindTexture->subRefCount();
	}

	bool GLESRenderView::create()
	{
        bool isDepthForamt = PixelUtil::IsDepth(m_format);

        GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(m_bindTexture);
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;
		texture->m_pixFmt = m_format;
		texture->m_isCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

        OGLESDebug(glGenTextures(1, &texture->m_glesTexture));
        OGLESDebug(glBindTexture(GL_TEXTURE_2D, texture->m_glesTexture));
        OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(m_format), m_width, m_height, 0, GLES2Mapping::MapFormat(m_format), GLES2Mapping::MapDataType(m_format), (GLvoid*)0));

        const SamplerState* sampleState = m_bindTexture->getSamplerState();
        if(sampleState)
            sampleState->active(NULL);

        return true;
	}

	void GLESRenderView::onSize( ui32 _width, ui32 _height )
	{
		m_width = _width;
		m_height = _height;

        static ui32 id = 0;
		m_bindTexture->subRefCount();
		m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(id++));

		// recreate
		create();
	}
}
