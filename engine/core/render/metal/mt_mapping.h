#pragma once

#include "base/mesh/mesh.h"
#include "base/ShaderProgram.h"
#include "mt_render_base.h"

namespace Echo
{
    class MTMapping
    {
    public:
        // Mapping Primitive Type
        static MTLPrimitiveType MapPrimitiveTopology(Mesh::TopologyType type);
        
        // Mapping vertex semantic string
        static String MapVertexSemanticString(VertexSemantic semantic);
        
        // Mapping MTLVertexFormat
        static MTLVertexFormat MapVertexFormat(PixelFormat pixelFormat);
        
        // Mapping pixel format
        static MTLPixelFormat MapPixelFormat(PixelFormat pixFmt);
        
        // Mapping Uniform type
        static ShaderParamType MapUniformType( MTLDataType uniformType);
        
        // Mapping DepthComparisonFunc
        static MTLCompareFunction MapComparisonFunc(RenderState::ComparisonFunc func);
    };
}
