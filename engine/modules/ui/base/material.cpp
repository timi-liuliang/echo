#include "material.h"

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

uniform sampler2D u_BaseColorSampler;

varying mediump vec2 texCoord;

void main(void)
{
    mediump vec4 textureColor = texture2D(u_BaseColorSampler, texCoord);
    gl_FragColor = textureColor;
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
    // get shader
    const char* UiMaterial::getDefault()
    {
        return g_textDefaultMaterial;
    }
}
