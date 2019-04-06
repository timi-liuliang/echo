#include "terrain_material.h"

static const char* g_terrainDefaultMaterial = R"(<?xml version = "1.0" encoding = "utf-8"?>
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

varying mediump vec2 texCoord;

void main(void)
{
    mediump vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
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
</Shader>
)";

namespace Echo
{
    // get shader
    const char* TerrainMaterial::getDefault()
    {
        return g_terrainDefaultMaterial;
    }
}

