<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.BaseColor="" Uniforms.LightDiffuse="" Uniforms.LightSpecular="" Uniforms.MetalicRoughnessShadingModelID="">
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

layout(binding = 2) uniform sampler2D MetalicRoughnessShadingModelID;
layout(binding = 3) uniform sampler2D BaseColor;
layout(binding = 4) uniform sampler2D LightDiffuse;
layout(binding = 5) uniform sampler2D LightSpecular;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 MetalicRoughnessShadingModelID_Color = texture(MetalicRoughnessShadingModelID, v_UV);
    vec4 BaseColor_Color = texture(BaseColor, v_UV);
    vec4 LightDiffuse_Color = texture(LightDiffuse, v_UV);
    float Float_648_Value = 1.0;
    vec4 LightSpecular_Color = texture(LightSpecular, v_UV);
    vec3 Multiplication_646 = BaseColor_Color.xyz * LightDiffuse_Color.xyz;
    vec3 Addition_647 = Multiplication_646 + LightSpecular_Color.xyz;
    vec3 Mix_274 = mix(BaseColor_Color.xyz, Addition_647, vec3(MetalicRoughnessShadingModelID_Color.z));
    vec3 _Emissive = Mix_274;
    float _Opacity = Float_648_Value;
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
            "in_id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "in_index": 2,
            "out_id": "{f3c612ed-4f62-4378-a269-2d0579349d95}",
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
                    "name": "rgb"
                }
            },
            "in_id": "{cafdafc3-47e3-447e-a504-aac71978e729}",
            "in_index": 0,
            "out_id": "{754a2c4d-ca26-4c3a-8524-d90490afd820}",
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
                    "name": "vec3"
                }
            },
            "in_id": "{cafdafc3-47e3-447e-a504-aac71978e729}",
            "in_index": 1,
            "out_id": "{bd79137d-46e2-4756-8666-05e244a06edf}",
            "out_index": 0
        },
        {
            "in_id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "in_index": 1,
            "out_id": "{cafdafc3-47e3-447e-a504-aac71978e729}",
            "out_index": 0
        },
        {
            "in_id": "{cafdafc3-47e3-447e-a504-aac71978e729}",
            "in_index": 2,
            "out_id": "{feab35af-9275-4404-9e75-1056ca600b79}",
            "out_index": 2
        },
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
            "in_id": "{feab35af-9275-4404-9e75-1056ca600b79}",
            "in_index": 0,
            "out_id": "{46627100-748c-44b5-aa49-d4b226a71c5f}",
            "out_index": 1
        }
    ],
    "nodes": [
        {
            "id": "{2ec4509f-f789-4e40-813e-d0fc579b2193}",
            "model": {
                "Variable": "ShaderTemplate_642",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 339,
                "y": 265
            }
        },
        {
            "id": "{46627100-748c-44b5-aa49-d4b226a71c5f}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "MetalicRoughnessShadingModelID",
                "name": "Texture"
            },
            "position": {
                "x": -528,
                "y": 630
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
                "x": -658,
                "y": 208
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
            "id": "{f3c612ed-4f62-4378-a269-2d0579349d95}",
            "model": {
                "Uniform": "false",
                "Value": "1.0",
                "Variable": "Float_648",
                "name": "Float"
            },
            "position": {
                "x": 96,
                "y": 415
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
        },
        {
            "id": "{feab35af-9275-4404-9e75-1056ca600b79}",
            "model": {
                "Variable": "Split_273",
                "name": "Split"
            },
            "position": {
                "x": -235,
                "y": 634
            }
        },
        {
            "id": "{cafdafc3-47e3-447e-a504-aac71978e729}",
            "model": {
                "Variable": "Mix_274",
                "name": "Mix"
            },
            "position": {
                "x": 116,
                "y": 250
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="false" WriteDepth="false" />
	</property>
</res>
