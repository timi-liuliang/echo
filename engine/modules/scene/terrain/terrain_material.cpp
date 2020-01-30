#include "terrain_material.h"

static const char* g_terrainVsCode = R"(
#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_WorldViewProjMatrix;
} vs_ubo;

// input
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Normal;
layout(location = 2) in vec2 a_UV;

// outputs
layout(location = 0) out vec2 v_UV;
layout(location = 1) out vec2 v_Normal;

void main(void)
{
    vec4 position = vs_ubo.u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    v_UV = a_UV;
    v_Normal = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
}
)";

static const char* g_terrainPsCode = R"(
#version 450

precision mediump float;

// inputs
layout(location = 0) in vec2  v_UV;
layout(location = 0) in vec3  v_Normal;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
    mediump vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    mediump vec3 lightColor = vec3(0.8, 0.8, 0.8);
    mediump vec3 textureColor = max(dot(v_Normal, lightDir), 0.0) * lightColor + vec3(0.2, 0.2, 0.2);
    
    o_FragColor = vec4(textureColor, 1.0);
}
)";

namespace Echo
{
    ShaderProgramPtr TerrainMaterial::getDefaultShader()
    {
        ResourcePath shaderVirtualPath = ResourcePath("echo_terrain_default_shader");
        ShaderProgramPtr shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
        if(!shader)
        {
            shader = ECHO_CREATE_RES(ShaderProgram);
            shader->setPath(shaderVirtualPath.getPath());
            shader->setType("glsl");
            shader->setVsCode(g_terrainVsCode);
            shader->setPsCode(g_terrainPsCode);
        }
        
        return shader;
    }
}

