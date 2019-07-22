#pragma once

#include "engine/core/render/interface/ShaderProgram.h"
#include "mt_render_base.h"

namespace Echo
{
    /**
     * https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Prog-Func/Prog-Func.html
     */
	class MTShaderProgram : public ShaderProgram
	{
    public:        
        // get mtl function
        id<MTLFunction> getMetalVertexFunction() { return m_metalVertexShader; }
        id<MTLFunction> getMetalFragmentFunction() { return m_metalFragmentShader; }
        
        // bind
        virtual void bindUniforms() override;
        virtual void bindRenderable(Renderable* renderable);
        
    public:
        // parse uniforms
        void parseUniforms(MTLRenderPipelineReflection* reflection);
        
    private:
        // create shader library
        virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;
        
        // add uniform
        void addUniform(MTLArgument* arg);
        
    private:
        id<MTLLibrary>      m_metalLibrary;
        id<MTLFunction>     m_metalVertexShader;
        id<MTLFunction>     m_metalFragmentShader;
	};
}
