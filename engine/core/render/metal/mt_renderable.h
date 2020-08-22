#pragma once

#include "base/Renderable.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTRenderable : public Renderable
	{
	public:
		MTRenderable(int identifier);
        virtual ~MTRenderable() {}
        
        // get render pipelinestate
        id<MTLRenderPipelineState> getMetalRenderPipelineState() { return m_metalRenderPipelineState; }
        
        // get metal buffer
        id<MTLBuffer> getMetalIndexBuffer();
        id<MTLBuffer> getMetalVertexBuffer();
        
        // bind shader uniforms
        void bindShaderParams();
        
    private:
        // link
        virtual void setMesh(MeshPtr mesh);
        
        // build vertex descriptor
        void buildVertexDescriptor();
        
        // build render pipeline state
        void buildRenderPipelineState();
        
    private:
        MTLVertexDescriptor*            m_metalVertexDescriptor = nullptr;
        id<MTLRenderPipelineState>      m_metalRenderPipelineState = nullptr;
	};
}
