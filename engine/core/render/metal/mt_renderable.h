#pragma once

#include "base/Renderable.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTRenderable : public Renderable
	{
	public:
		MTRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
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
        virtual void setMesh(Mesh* mesh) override;
        
        // build vertex descriptor
        void buildVertexDescriptor();

        // get vertex attribute by semantic
        MTLVertexAttribute* getMTLVertexAttributeBySemantic(VertexSemantic semantic);
        
        // build render pipeline state
        void buildRenderPipelineState();
        
    private:
        MTLVertexDescriptor*            m_metalVertexDescriptor = nullptr;
        MTLRenderPipelineDescriptor*    m_metalRenderPipelineDescriptor = nullptr;
        id<MTLRenderPipelineState>      m_metalRenderPipelineState = nullptr;
        MTLRenderPipelineReflection*    m_metalRenderPipelineReflection = nil;
	};
}
