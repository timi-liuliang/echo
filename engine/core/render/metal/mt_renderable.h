#pragma once

#include "engine/core/render/interface/Renderable.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTRenderable : public Renderable
	{
	public:
		MTRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
        virtual ~MTRenderable() {}

        // link shader and program
        virtual void link() override {}
        
    public:
        // get render pipelinestate
        id<MTLRenderPipelineState> getMetalRenderPipelineState() { return m_metalRenderPipelineState; }
        
        // get metal buffer
        id<MTLBuffer> getMetalIndexBuffer();
        id<MTLBuffer> getMetalVertexBuffer();
        
    private:
        MTLRenderPipelineDescriptor*    m_metalRenderPipelineDescriptor = nullptr;
        id<MTLRenderPipelineState>      m_metalRenderPipelineState;
	};
}
