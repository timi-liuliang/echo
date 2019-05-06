#include <gtest/gtest.h>
#include <engine/core/render/interface/glslcc/GLSLCrossCompiler.h>

// glsl vs for test
static const char* glslVS =R"(#version 450

layout (location = POSITION) in vec3 aPos;
layout (location = COLOR0) in vec4 aColor;
layout (location = TEXCOORD0) in vec2 aCoord;

layout (std140) uniform matrices
{
    mat4 projection;
    mat4 view;
    mat4 model;
};

layout (location = COLOR0) out flat vec4 outColor;
layout (location = TEXCOORD0) out vec2 outCoord;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    outColor = aColor;
    outCoord = aCoord;
}
)";

// glsl fragment for test
static const char* glslPS = R"(#version 450

precision mediump float;

layout (location = COLOR0) in flat vec4 inColor;
layout (location = TEXCOORD0) in vec2 inCoord;

layout (location = SV_Target0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D colorMap;

void main()
{
    lowp vec4 c = texture(colorMap, inCoord);
    fragColor = inColor * c;
}
)";

TEST(GLSLCrossCompiler, GLSLToSPIRV)
{
    Echo::GLSLCrossCompiler glslCompiler;
    
    // set input
    glslCompiler.setInput( glslVS, glslPS, nullptr);
    
    // get spirv
    const Echo::vector<Echo::ui32>::type& spirvVS = glslCompiler.getSPIRV(Echo::GLSLCrossCompiler::ShaderType::VS);
	const Echo::vector<Echo::ui32>::type& spirvFS = glslCompiler.getSPIRV(Echo::GLSLCrossCompiler::ShaderType::FS);

    // expect
    EXPECT_EQ(spirvVS.size()>0, true);
	EXPECT_EQ(spirvFS.size() > 0, true);
}
