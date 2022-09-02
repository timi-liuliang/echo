<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.BaseColor="" Uniforms.LightDiffuse="" Uniforms.LightSpecular="">
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

layout(binding = 2) uniform sampler2D LightDiffuse;
layout(binding = 3) uniform sampler2D BaseColor;
layout(binding = 4) uniform sampler2D LightSpecular;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    float Float_648_Value = 1.0;
    vec4 LightDiffuse_Color = texture(LightDiffuse, v_UV);
    vec4 BaseColor_Color = texture(BaseColor, v_UV);
    vec4 LightSpecular_Color = texture(LightSpecular, v_UV);
    vec3 Multiplication_646 = BaseColor_Color.xyz * LightDiffuse_Color.xyz;
    vec3 Addition_647 = Multiplication_646 + LightSpecular_Color.xyz;
    float _Opacity = Float_648_Value;
    vec3 _Emissive = Addition_647;
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
                    "name": "B"
                },
                "out": {
                    "id": "vec3",
                    "name": "rgb"
                }
            },
            "in_id": "{ae675b2c-9570-41ad-928c-9c615ad12fc3}",
            "in_index": 1,
            "out_id": "{34e0cd93-293a-415b-9466-ba7c5ad27e3a}",
            "out_index": 1
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
            "in_id": "{ae675b2c-9570-41ad-928c-9c615ad12fc3}",
            "in_index": 0,
            "out_id": "{754a2c4d-ca26-4c3a-8524-d90490afd820}",
            "out_index": 1
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
            "in_id": "{bd79137d-46e2-4756-8666-05e244a06edf}",
            "in_index": 0,
            "out_id": "{ae675b2c-9570-41ad-928c-9c615ad12fc3}",
            "out_index": 0
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
            "in_id": "{bd79137d-46e2-4756-8666-05e244a06edf}",
            "in_index": 1,
            "out_id": "{848af082-e2a1-40f3-80c2-a4874c6971ef}",
            "out_index": 1
        },
        {
            "in_id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "in_index": 2,
            "out_id": "{f3c612ed-4f62-4378-a269-2d0579349d95}",
            "out_index": 0
        },
        {
            "in_id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "in_index": 7,
            "out_id": "{bd79137d-46e2-4756-8666-05e244a06edf}",
            "out_index": 0
        }
    ],
    "nodes": [
        {
            "id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "model": {
                "Variable": "ShaderTemplate_642",
                "name": "ShaderTemplate"
            },
            "position": {
                "x": 41,
                "y": 161
            }
        },
        {
            "id": "{f3c612ed-4f62-4378-a269-2d0579349d95}",
            "model": {
                "Uniform": "false",
                "Value": "1.0",
                "Variable": "Float_648",
                "name": "Float"
            },
            "position": {
                "x": -261,
                "y": 237
            }
        },
        {
            "id": "{34e0cd93-293a-415b-9466-ba7c5ad27e3a}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "LightDiffuse",
                "name": "Texture"
            },
            "position": {
                "x": -655,
                "y": 395
            }
        },
        {
            "id": "{754a2c4d-ca26-4c3a-8524-d90490afd820}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "BaseColor",
                "name": "Texture"
            },
            "position": {
                "x": -654,
                "y": 243
            }
        },
        {
            "id": "{848af082-e2a1-40f3-80c2-a4874c6971ef}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "LightSpecular",
                "name": "Texture"
            },
            "position": {
                "x": -429,
                "y": 467
            }
        },
        {
            "id": "{ae675b2c-9570-41ad-928c-9c615ad12fc3}",
            "model": {
                "Variable": "Multiplication_646",
                "name": "Multiplication"
            },
            "position": {
                "x": -385,
                "y": 322
            }
        },
        {
            "id": "{bd79137d-46e2-4756-8666-05e244a06edf}",
            "model": {
                "Variable": "Addition_647",
                "name": "Addition"
            },
            "position": {
                "x": -207,
                "y": 383
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="false" WriteDepth="false" />
	</property>
</res>
