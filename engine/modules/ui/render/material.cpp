#include "material.h"
#include "base/Renderer.h"

static const char* g_textDefaultMaterialGLSL = R"(<?xml version = "1.0" encoding = "utf-8"?>
<Shader type="glsl">
<VS>#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

// outputs
layout(location = 0) out vec2 v_TexCoord;

void main(void)
{
    vec4 position = vs_ubo.u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    v_TexCoord = a_UV;
}
</VS>
<PS>#version 450

precision mediump float;

// uniforms
layout(binding = 0) uniform UBO
{
    float u_UiAlpha;
} fs_ubo;

layout(binding = 3) uniform sampler2D u_BaseColorSampler;

// inputs
layout(location = 0) in vec2  v_TexCoord;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
    mediump vec4 textureColor = texture(u_BaseColorSampler, v_TexCoord);
    o_FragColor = vec4(textureColor.xyz, textureColor.a * fs_ubo.u_UiAlpha);
}

</PS>
<BlendState>
    <BlendEnable value = "true" />
    <SrcBlend value = "BF_SRC_ALPHA" />
    <DstBlend value = "BF_INV_SRC_ALPHA" />
</BlendState>
<RasterizerState>
    <CullMode value = "CULL_NONE" />
</RasterizerState>
<DepthStencilState>
    <DepthEnable value = "false" />
    <WriteDepth value = "false" />
</DepthStencilState>
<SamplerState>
    <BiLinearMirror>
        <MinFilter value = "FO_LINEAR" />
        <MagFilter value = "FO_LINEAR" />
        <MipFilter value = "FO_NONE" />
        <AddrUMode value = "AM_CLAMP" />
        <AddrVMode value = "AM_CLAMP" />
    </BiLinearMirror>
</SamplerState>
<Texture>
    <stage no = "0" sampler = "BiLinearMirror" />
</Texture>
</Shader>
)";

namespace Echo
{
    const char* UiMaterial::getDefault()
    {
        return g_textDefaultMaterialGLSL;
    }
}
