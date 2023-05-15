<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.BaseColor="1 1 1 1 " Uniforms.BaseTexture="">
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

layout(binding = 1, std140) uniform UBO
{
    vec2 u_BaseTextureSize;
    vec4 BaseColor;
} fs_ubo;

layout(binding = 2) uniform sampler2D BaseTexture;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 BaseTexture_Color = texture(BaseTexture, v_UV);
    vec4 BaseColor_Value = vec4(fs_ubo.BaseColor);
    float Multiplication_395 = BaseTexture_Color.w * BaseColor_Value.w;
    vec3 Multiplication_394 = BaseTexture_Color.xyz * BaseColor_Value.xyz;
    vec3 _Diffuse = Multiplication_394;
    float _Opacity = Multiplication_395;
    float _Metalic = 0.20000000298023223876953125;
    float _PerceptualRoughness = 0.5;
    vec3 FinalColor = _Diffuse;
    o_FragColor = vec4(FinalColor, _Opacity);
}

]]></property>
	<property name="Graph"><![CDATA[{
    "connections": [
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
            "in_id": "{b0e86aa7-ffb4-4355-9cde-3313072c36da}",
            "in_index": 0,
            "out_id": "{a774dcdb-984a-4098-a86a-e1d45c4e728b}",
            "out_index": 2
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
            "in_id": "{b0e86aa7-ffb4-4355-9cde-3313072c36da}",
            "in_index": 1,
            "out_id": "{99e60fe4-8498-44b1-b7bd-fa8178951ab2}",
            "out_index": 0
        },
        {
            "in_id": "{059b5a1c-1963-482c-9be4-ac635375e7f8}",
            "in_index": 0,
            "out_id": "{b0e86aa7-ffb4-4355-9cde-3313072c36da}",
            "out_index": 0
        },
        {
            "in_id": "{059b5a1c-1963-482c-9be4-ac635375e7f8}",
            "in_index": 2,
            "out_id": "{8302a621-2de2-4086-8db6-2d001e1646f4}",
            "out_index": 0
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "A"
                },
                "out": {
                    "id": "float",
                    "name": "a"
                }
            },
            "in_id": "{8302a621-2de2-4086-8db6-2d001e1646f4}",
            "in_index": 0,
            "out_id": "{a774dcdb-984a-4098-a86a-e1d45c4e728b}",
            "out_index": 3
        },
        {
            "converter": {
                "in": {
                    "id": "any",
                    "name": "B"
                },
                "out": {
                    "id": "float",
                    "name": "a"
                }
            },
            "in_id": "{8302a621-2de2-4086-8db6-2d001e1646f4}",
            "in_index": 1,
            "out_id": "{99e60fe4-8498-44b1-b7bd-fa8178951ab2}",
            "out_index": 1
        }
    ],
    "nodes": [
        {
            "id": "{059b5a1c-1963-482c-9be4-ac635375e7f8}",
            "model": {
                "Variable": "ShaderTemplateTransparent_390",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 44,
                "y": 286
            }
        },
        {
            "id": "{a774dcdb-984a-4098-a86a-e1d45c4e728b}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "BaseTexture",
                "name": "Texture"
            },
            "position": {
                "x": -436,
                "y": 157
            }
        },
        {
            "id": "{99e60fe4-8498-44b1-b7bd-fa8178951ab2}",
            "model": {
                "Color": "1 1 1 1 ",
                "Uniform": "true",
                "Variable": "BaseColor",
                "name": "Color"
            },
            "position": {
                "x": -343,
                "y": 371
            }
        },
        {
            "id": "{8302a621-2de2-4086-8db6-2d001e1646f4}",
            "model": {
                "Variable": "Multiplication_395",
                "name": "Multiplication"
            },
            "position": {
                "x": -181,
                "y": 373
            }
        },
        {
            "id": "{b0e86aa7-ffb4-4355-9cde-3313072c36da}",
            "model": {
                "Variable": "Multiplication_394",
                "name": "Multiplication"
            },
            "position": {
                "x": -182,
                "y": 241
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
