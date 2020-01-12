#include "terrain_material.h"

static const char* g_terrainDefaultMaterial = R"(<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
<VS>#version 100

attribute vec3 a_Position;
attribute vec4 a_Normal;
attribute vec2 a_UV;

uniform mat4 u_WorldMatrix;
uniform mat4 u_WorldViewProjMatrix;

varying vec2 v_UV;
varying vec3 v_Normal;

void main(void)
{
    vec4 position = u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    v_UV = a_UV;
    v_Normal = normalize(vec3(u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
}
</VS>
<PS>#version 100

varying mediump vec2 v_UV;
varying mediump vec3 v_Normal;

void main(void)
{
    mediump vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    mediump vec3 lightColor = vec3(0.8, 0.8, 0.8);
    mediump vec3 textureColor = max(dot(v_Normal, lightDir), 0.0) * lightColor + vec3(0.2, 0.2, 0.2);
    
    gl_FragColor = vec4(textureColor, 1.0);
}
</PS>
<BlendState>
    <BlendEnable value = "false" />
</BlendState>
<RasterizerState>
    <CullMode value = "CULL_NONE" />
</RasterizerState>
<DepthStencilState>
    <DepthEnable value = "true" />
    <WriteDepth value = "true" />
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

