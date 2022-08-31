<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Opaque" Uniforms.Input="">
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

layout(binding = 2) uniform sampler2D Input;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

void main()
{
    vec4 Input_Color = texture(Input, v_UV);
    float _Opacity = Input_Color.w;
    vec3 _Emissive = Input_Color.xyz;
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
            "in_id": "{9825764a-31d2-4948-8352-daa5cfe2bb6e}",
            "in_index": 7,
            "out_id": "{e05840ec-0410-4ba0-9f27-5e5d85d144f4}",
            "out_index": 1
        },
        {
            "in_id": "{9825764a-31d2-4948-8352-daa5cfe2bb6e}",
            "in_index": 2,
            "out_id": "{e05840ec-0410-4ba0-9f27-5e5d85d144f4}",
            "out_index": 2
        }
    ],
    "nodes": [
        {
            "id": "{9825764a-31d2-4948-8352-daa5cfe2bb6e}",
            "model": {
                "Variable": "ShaderTemplate_628",
                "name": "ShaderTemplate"
            },
            "position": {
                "x": 0,
                "y": 288
            }
        },
        {
            "id": "{e05840ec-0410-4ba0-9f27-5e5d85d144f4}",
            "model": {
                "Atla": "false",
                "Texture": "",
                "Type": "General",
                "Variable": "Input",
                "name": "Texture"
            },
            "position": {
                "x": -311,
                "y": 477
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
