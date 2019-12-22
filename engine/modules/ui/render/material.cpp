#include "material.h"
#include "base/Renderer.h"

static const char* g_textDefaultMaterial = R"(<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
<VS>#version 100

attribute vec3 a_Position;
attribute vec2 a_UV;

uniform mat4 u_WorldViewProjMatrix;

varying vec2 texCoord;

void main(void)
{
    vec4 position = u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    texCoord = a_UV;
}
</VS>
<PS>#version 100

uniform mediump float	u_UiAlpha;
uniform sampler2D		u_BaseColorSampler;

varying mediump vec2 texCoord;

void main(void)
{
    mediump vec4 textureColor = texture2D(u_BaseColorSampler, texCoord);
    gl_FragColor.rgb = textureColor.rgb;

	gl_FragColor.a = textureColor.a * u_UiAlpha;
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

static const char* g_textDefaultMaterialGLSL = R"(<?xml version = "1.0" encoding = "utf-8"?>
<Shader type="glsl">
<VS>#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldViewProjMatrix;
} ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

// outputs
layout(location = 0) out vec2 v_TexCoord;

void main(void)
{
    vec4 position = ubo.u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    v_TexCoord = a_UV;
}
</VS>
<PS>#version 450

precision mediump float;

// uniforms
// uniforms
layout(binding = 0) uniform UBO
{
    float u_UiAlpha;
} ubo;

layout(binding = 3) uniform sampler2D u_BaseColorSampler;

// inputs
layout(location = 0) in vec2  v_TexCoord;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
    mediump vec4 textureColor = texture(u_BaseColorSampler, v_TexCoord);
    o_FragColor = vec4(textureColor.xyz, textureColor.a * ubo.u_UiAlpha);
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
        return Renderer::instance()->getType()==Renderer::Type::OpenGLES ? g_textDefaultMaterial : g_textDefaultMaterialGLSL;
    }
}
