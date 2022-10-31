<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.Depth="Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferDepth.rt">
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

layout(binding = 2) uniform sampler2D Depth;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 Depth_Color = texture(Depth, v_UV);
    float Float_284_Value = 1.0;
    vec3 _Emissive = vec3(Depth_Color.x);
    float _Opacity = Float_284_Value;
    vec3 _Diffuse = vec3(0.0);
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
            "converter": {
                "in": {
                    "id": "any",
                    "name": "any"
                },
                "out": {
                    "id": "vec3",
                    "name": "rgb"
                }
            },
            "in_id": "{07de2d2b-8bdb-48b3-87ae-0668a0344a5a}",
            "in_index": 0,
            "out_id": "{d5f47fb8-7c0e-4627-9384-e6f20bf68692}",
            "out_index": 1
        },
        {
            "in_id": "{7029286d-3a95-4bf8-8869-c5a65488fe85}",
            "in_index": 2,
            "out_id": "{9b812b6b-03f8-4b59-bfcb-58fc821cf4f7}",
            "out_index": 0
        },
        {
            "converter": {
                "in": {
                    "id": "vec3",
                    "name": "Emissive"
                },
                "out": {
                    "id": "float",
                    "name": "r"
                }
            },
            "in_id": "{7029286d-3a95-4bf8-8869-c5a65488fe85}",
            "in_index": 1,
            "out_id": "{07de2d2b-8bdb-48b3-87ae-0668a0344a5a}",
            "out_index": 0
        }
    ],
    "nodes": [
        {
            "id": "{d5f47fb8-7c0e-4627-9384-e6f20bf68692}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/GBuffer/GBufferDepth.rt",
                "Type": "General",
                "Variable": "Depth",
                "name": "Texture"
            },
            "position": {
                "x": -577,
                "y": 292
            }
        },
        {
            "id": "{07de2d2b-8bdb-48b3-87ae-0668a0344a5a}",
            "model": {
                "Variable": "Split_282",
                "name": "Split"
            },
            "position": {
                "x": -305,
                "y": 323
            }
        },
        {
            "id": "{7029286d-3a95-4bf8-8869-c5a65488fe85}",
            "model": {
                "Variable": "ShaderTemplateTransparent_283",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 0,
                "y": 289
            }
        },
        {
            "id": "{9b812b6b-03f8-4b59-bfcb-58fc821cf4f7}",
            "model": {
                "Uniform": "false",
                "Value": "1.0",
                "Variable": "Float_284",
                "name": "Float"
            },
            "position": {
                "x": -323,
                "y": 515
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
