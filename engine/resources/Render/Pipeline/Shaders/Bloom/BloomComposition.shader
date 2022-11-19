<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.Bloom="Engine://Render/Pipeline/Framebuffer/Bloom/Color.rt" Uniforms.Transparent="Engine://Render/Pipeline/Framebuffer/Translucency/Color.rt">
	<property name="VertexShader"><![CDATA[#version 450

layout(binding = 0, std140) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

layout(location = 0) in vec3 a_Position;
layout(location = 7) out vec2 v_UV;
layout(location = 4) in vec2 a_UV;

void main()
{
    vec4 worldPosition = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    vec4 clipPosition = vs_ubo.u_ViewProjMatrix * worldPosition;
    gl_Position = clipPosition;
    v_UV = a_UV;
}

]]></property>
	<property name="FragmentShader"><![CDATA[#version 450

layout(binding = 2) uniform sampler2D Transparent;
layout(binding = 3) uniform sampler2D Bloom;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 Transparent_Color = texture(Transparent, v_UV);
    vec4 Bloom_Color = texture(Bloom, v_UV);
    vec3 Addition_391 = Bloom_Color.xyz + Transparent_Color.xyz;
    vec3 _Emissive = Addition_391;
    vec3 _Diffuse = vec3(0.0);
    float _Opacity = 1.0;
    float _Metalic = 0.20000000298023223876953125;
    float _PerceptualRoughness = 0.5;
    vec3 FinalColor = _Diffuse;
    FinalColor += _Emissive;
    o_FragColor = vec4(FinalColor, _Opacity);
}

]]></property>
	<property name="Graph"><![CDATA[{
    "connections": [
        {
            "in_id": "{83673bf2-0793-426f-8675-e93032befbc9}",
            "in_index": 1,
            "out_id": "{317e86bb-a099-43a8-be23-6d35cc6f1747}",
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
            "in_id": "{317e86bb-a099-43a8-be23-6d35cc6f1747}",
            "in_index": 0,
            "out_id": "{dd93c273-bca6-4f7b-8ec9-b984a80b3057}",
            "out_index": 1
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "B"
                },
                "out": {
                    "id": "vec3",
                    "name": "rgb"
                }
            },
            "in_id": "{317e86bb-a099-43a8-be23-6d35cc6f1747}",
            "in_index": 1,
            "out_id": "{e9fcd17b-7500-46d6-a4a5-6b1f83c9e64c}",
            "out_index": 1
        }
    ],
    "nodes": [
        {
            "id": "{317e86bb-a099-43a8-be23-6d35cc6f1747}",
            "model": {
                "Variable": "Addition_391",
                "name": "Addition"
            },
            "position": {
                "x": -185,
                "y": 365
            }
        },
        {
            "id": "{e9fcd17b-7500-46d6-a4a5-6b1f83c9e64c}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/Translucency/Color.rt",
                "Type": "General",
                "Variable": "Transparent",
                "name": "Texture"
            },
            "position": {
                "x": -395,
                "y": 429
            }
        },
        {
            "id": "{83673bf2-0793-426f-8675-e93032befbc9}",
            "model": {
                "Variable": "ShaderTemplateTransparent_319",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 6,
                "y": 335
            }
        },
        {
            "id": "{dd93c273-bca6-4f7b-8ec9-b984a80b3057}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/Bloom/Color.rt",
                "Type": "General",
                "Variable": "Bloom",
                "name": "Texture"
            },
            "position": {
                "x": -392,
                "y": 270
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
