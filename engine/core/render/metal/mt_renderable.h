#pragma once

#include "engine/core/render/interface/Renderable.h"
#include "mt_render_base.h"

namespace Echo
{
	class VKRenderable : public Renderable
	{
	public:
		VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
        virtual ~VKRenderable() {}

        // link shader and program
        virtual void link() override {}
        
    private:
        MTLRenderPipelineDescriptor*    m_metalRenderPipelineDescriptor = nullptr;
        id<MTLRenderPipelineState>      m_metalRenderPipelineState;
        id<MTLCommandBuffer>            m_metalCommandBuffer;
	};
}
