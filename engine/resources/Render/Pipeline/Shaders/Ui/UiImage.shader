<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.BaseTexture="Engine://Render/Pipeline/Shaders/Ui/White.png">
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
} fs_ubo;

layout(binding = 2) uniform sampler2D BaseTexture;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 BaseTexture_Color = texture(BaseTexture, v_UV);
    vec4 BaseColor_Value = vec4(1.0);
    vec3 Multiplication_386 = BaseTexture_Color.xyz * BaseColor_Value.xyz;
    vec3 _Diffuse = Multiplication_386;
    float _Opacity = BaseTexture_Color.w;
    float _Metalic = 0.20000000298023223876953125;
    float _PerceptualRoughness = 0.5;
    vec3 FinalColor = _Diffuse;
    o_FragColor = vec4(FinalColor, _Opacity);
}

]]></property>
	<property name="Graph"><![CDATA[{
    "connections": [
        {
            "in_id": "{84cf5024-a536-4aaf-bf6d-3a2738663ac5}",
            "in_index": 2,
            "out_id": "{a334abbd-e79f-49a6-a763-e7ba2c5a708a}",
            "out_index": 3
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
            "in_id": "{fc779634-fd38-462f-8bbb-ab67d475852b}",
            "in_index": 0,
            "out_id": "{a334abbd-e79f-49a6-a763-e7ba2c5a708a}",
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
            "in_id": "{fc779634-fd38-462f-8bbb-ab67d475852b}",
            "in_index": 1,
            "out_id": "{8f6ff6fc-2080-4da7-8bbb-ae7043369df5}",
            "out_index": 0
        },
        {
            "in_id": "{84cf5024-a536-4aaf-bf6d-3a2738663ac5}",
            "in_index": 0,
            "out_id": "{fc779634-fd38-462f-8bbb-ab67d475852b}",
            "out_index": 0
        }
    ],
    "nodes": [
        {
            "id": "{fc779634-fd38-462f-8bbb-ab67d475852b}",
            "model": {
                "Variable": "Multiplication_386",
                "name": "Multiplication"
            },
            "position": {
                "x": -10,
                "y": 235
            }
        },
        {
            "id": "{84cf5024-a536-4aaf-bf6d-3a2738663ac5}",
            "model": {
                "Variable": "ShaderTemplateTransparent_376",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 202,
                "y": 237
            }
        },
        {
            "id": "{a334abbd-e79f-49a6-a763-e7ba2c5a708a}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Shaders/Ui/White.png",
                "Type": "General",
                "Variable": "BaseTexture",
                "name": "Texture"
            },
            "position": {
                "x": -325,
                "y": 170
            }
        },
        {
            "id": "{8f6ff6fc-2080-4da7-8bbb-ae7043369df5}",
            "model": {
                "Color": "1 1 1 1 ",
                "Uniform": "false",
                "Variable": "BaseColor",
                "name": "Color"
            },
            "position": {
                "x": -234,
                "y": 358
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
