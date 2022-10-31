<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Lighting" CullMode="CULL_BACK" BlendMode="Opaque" Uniforms.GBuffer_Normal="Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferNormal.rt" Uniforms.GBuffer_Position="Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferPosition.rt">
	<property name="VertexShader"><![CDATA[#version 450

struct Position
{
    vec3 local;
    vec3 world;
    vec3 view;
};

layout(binding = 0, std140) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

layout(location = 0) in vec3 a_Position;
layout(location = 0) out Position v_Position;
layout(location = 4) out vec3 v_NormalLocal;
layout(location = 1) in vec3 a_Normal;
layout(location = 3) out vec3 v_Normal;
layout(location = 6) out vec4 v_Color;
layout(location = 3) in vec4 a_Color;
layout(location = 7) out vec2 v_UV;
layout(location = 4) in vec2 a_UV;

void main()
{
    vec4 worldPosition = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    vec4 clipPosition = vs_ubo.u_ViewProjMatrix * worldPosition;
    v_Position.local = a_Position;
    v_Position.world = worldPosition.xyz;
    v_Position.view = ((clipPosition.xyz / vec3(clipPosition.w)) + vec3(1.0)) * 0.5;
    gl_Position = clipPosition;
    v_NormalLocal = a_Normal;
    v_Normal = normalize(vec3((vs_ubo.u_WorldMatrix * vec4(a_Normal, 0.0)).xyz));
    v_Color = a_Color;
    v_UV = a_UV;
}

]]></property>
	<property name="FragmentShader"><![CDATA[#version 450

struct Position
{
    vec3 local;
    vec3 world;
    vec3 view;
};

layout(binding = 2) uniform sampler2D GBuffer_Normal;
layout(binding = 3) uniform sampler2D GBuffer_Position;

layout(location = 7) in vec2 v_UV;
layout(location = 3) in vec3 v_Normal;
layout(location = 6) in vec4 v_Color;
layout(location = 0) out vec4 o_FragDiffuse;
layout(location = 1) out vec4 o_FragSpecular;
layout(location = 0) in Position v_Position;
layout(location = 4) in vec3 v_NormalLocal;

vec3 Diffuse(vec3 InNormal, vec3 InLightDir, vec4 InLightColor)
{
    vec3 Normal = (InNormal * 2.0) - vec3(1.0);
    return InLightColor.xyz * dot(InNormal, -InLightDir);
}

void main()
{
    vec4 Color_627_Value = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 GBuffer_Normal_Color = texture(GBuffer_Normal, v_UV);
    vec4 GBuffer_Position_Color = texture(GBuffer_Position, v_UV);
    vec3 param = GBuffer_Normal_Color.xyz;
    vec3 param_1 = v_Normal;
    vec4 param_2 = v_Color;
    vec3 GLSL_569 = Diffuse(param, param_1, param_2);
    float Length_324 = length(GBuffer_Position_Color.xyz);
    vec3 Multiplication_276 = GLSL_569 * Length_324;
    vec3 _Diffuse = Multiplication_276;
    vec3 _Specular = Color_627_Value.xyz;
    o_FragDiffuse = vec4(_Diffuse, 1.0);
    o_FragSpecular = vec4(_Specular, 1.0);
}

]]></property>
	<property name="Graph"><![CDATA[{
    "connections": [
        {
            "in_id": "{5dbf7943-a2c1-470c-8dec-7aa0e0817f98}",
            "in_index": 0,
            "out_id": "{1b18812f-5203-4858-9e11-64f3a3df9a09}",
            "out_index": 1
        },
        {
            "in_id": "{5dbf7943-a2c1-470c-8dec-7aa0e0817f98}",
            "in_index": 1,
            "out_id": "{be8c8bd3-7694-4adc-8762-9f2645122d0f}",
            "out_index": 0
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "B"
                },
                "out": {
                    "id": "float",
                    "name": "float"
                }
            },
            "in_id": "{02fd125d-ef50-4254-a2f9-019e5d7467a3}",
            "in_index": 1,
            "out_id": "{d99a6cae-0b93-434c-81fd-e5d51e8513ea}",
            "out_index": 0
        },
        {
            "in_id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "in_index": 0,
            "out_id": "{02fd125d-ef50-4254-a2f9-019e5d7467a3}",
            "out_index": 0
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "A"
                },
                "out": {
                    "id": "vec3",
                    "name": "vec3"
                }
            },
            "in_id": "{02fd125d-ef50-4254-a2f9-019e5d7467a3}",
            "in_index": 0,
            "out_id": "{5dbf7943-a2c1-470c-8dec-7aa0e0817f98}",
            "out_index": 0
        },
        {
            "in_id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "in_index": 1,
            "out_id": "{e2eb8b65-65fd-425d-93c8-944c74cd8c75}",
            "out_index": 0
        },
        {
            "in_id": "{5dbf7943-a2c1-470c-8dec-7aa0e0817f98}",
            "in_index": 2,
            "out_id": "{7028fed8-7e12-4907-aa83-5d7b810b388e}",
            "out_index": 0
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "A"
                },
                "out": {
                    "id": "vec3",
                    "name": "rgb"
                }
            },
            "in_id": "{d99a6cae-0b93-434c-81fd-e5d51e8513ea}",
            "in_index": 0,
            "out_id": "{b06a016b-ddaf-45fe-8b7f-8ffeffce3549}",
            "out_index": 1
        }
    ],
    "nodes": [
        {
            "id": "{e2eb8b65-65fd-425d-93c8-944c74cd8c75}",
            "model": {
                "Color": "0 0 0 1 ",
                "Uniform": "false",
                "Variable": "Color_627",
                "name": "Color"
            },
            "position": {
                "x": -64,
                "y": 566
            }
        },
        {
            "id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "model": {
                "Variable": "ShaderTemplate_624",
                "name": "ShaderTemplateLighting"
            },
            "position": {
                "x": 113,
                "y": 477
            }
        },
        {
            "id": "{1b18812f-5203-4858-9e11-64f3a3df9a09}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferNormal.rt",
                "Type": "General",
                "Variable": "GBuffer_Normal",
                "name": "Texture"
            },
            "position": {
                "x": -732,
                "y": 84
            }
        },
        {
            "id": "{be8c8bd3-7694-4adc-8762-9f2645122d0f}",
            "model": {
                "Attribute": "direction",
                "Variable": "DirectionLight_570",
                "name": "DirectionLight"
            },
            "position": {
                "x": -781,
                "y": 228
            }
        },
        {
            "id": "{5dbf7943-a2c1-470c-8dec-7aa0e0817f98}",
            "model": {
                "Code": "vec3 Diffuse(vec3 InNormal, vec3 InLightDir, vec4 InLightColor)\n{\n\tvec3 Normal = InNormal * 2.0 - 1.0;\n\treturn InLightColor.rgb * dot(InNormal, -InLightDir);\n}",
                "FunctionName": "Diffuse",
                "Parameters": "vec3 InNormal, vec3 InLightDir, vec4 InLightColor",
                "ReturnType": "vec3",
                "Variable": "GLSL_569",
                "name": "GLSL"
            },
            "position": {
                "x": -342,
                "y": 214
            }
        },
        {
            "id": "{7028fed8-7e12-4907-aa83-5d7b810b388e}",
            "model": {
                "Attribute": "color",
                "Variable": "DirectionLight_571",
                "name": "DirectionLight"
            },
            "position": {
                "x": -783,
                "y": 314
            }
        },
        {
            "id": "{b06a016b-ddaf-45fe-8b7f-8ffeffce3549}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferPosition.rt",
                "Type": "General",
                "Variable": "GBuffer_Position",
                "name": "Texture"
            },
            "position": {
                "x": -662,
                "y": 517
            }
        },
        {
            "id": "{02fd125d-ef50-4254-a2f9-019e5d7467a3}",
            "model": {
                "Variable": "Multiplication_276",
                "name": "Multiplication"
            },
            "position": {
                "x": -66,
                "y": 363
            }
        },
        {
            "id": "{d99a6cae-0b93-434c-81fd-e5d51e8513ea}",
            "model": {
                "Variable": "Length_324",
                "name": "Length"
            },
            "position": {
                "x": -375,
                "y": 551
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
