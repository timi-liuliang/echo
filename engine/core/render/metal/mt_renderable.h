#pragma once

#include "engine/core/render/interface/Renderable.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTRenderable : public Renderable
	{
    public:
        // vertex stream bind state(for multi stream)
        enum BindState
        {
            BS_NORMAL = 0,
            BS_BEGIN = 1 << 0,
            BS_END = 1 << 1,
        };
        
        // stream unit
        struct StreamUnit
        {
            VertexElementList        m_vertElements;
            GPUBuffer*               m_buffer;
            
            StreamUnit()
            : m_buffer(NULL)
            {}
        };
        
	public:
		MTRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
        virtual ~MTRenderable() {}
        
        // param operate
        virtual void setShaderParam(const String& name, ShaderParamType type, const void* data, size_t num=1) override;
        
    public:
        // get render pipelinestate
        id<MTLRenderPipelineState> getMetalRenderPipelineState() { return m_metalRenderPipelineState; }
        
        // get metal buffer
        id<MTLBuffer> getMetalIndexBuffer();
        id<MTLBuffer> getMetalVertexBuffer();
        
    private:
        // link
        virtual void link() override;
        
        // bind shader uniforms
        virtual void bindShaderParams() override;
        
        // bind vertex stream
        bool bindVertexStream(const VertexElementList& vertElements, GPUBuffer* vertexBuffer, int flag = BS_BEGIN | BS_END);
        
        // build vertex descriptor
        void buildVertexDescriptor(StreamUnit* stream);

        // get vertex attribute by semantic
        MTLVertexAttribute* getMTLVertexAttributeBySemantic(VertexSemantic semantic);
        
        // build render pipeline state
        void buildRenderPipelineState();
        
    private:
        vector<StreamUnit>::type        m_vertexStreams;
        MTLVertexDescriptor*            m_metalVertexDescriptor = nullptr;
        MTLRenderPipelineDescriptor*    m_metalRenderPipelineDescriptor = nullptr;
        id<MTLRenderPipelineState>      m_metalRenderPipelineState = nullptr;
        MTLRenderPipelineReflection*    m_metalRenderPipelineReflection = nil;
	};
}
