#include <gtest/gtest.h>
#include <engine/core/render/interface/glslcc/GLSLCrossCompiler.h>

TEST(GLSLCrossCompiler, GLSLToSPIRV)
{
    Echo::GLSLCrossCompiler glslCompiler;
    
    // set input
    glslCompiler.setInput( nullptr, nullptr, nullptr);
}
