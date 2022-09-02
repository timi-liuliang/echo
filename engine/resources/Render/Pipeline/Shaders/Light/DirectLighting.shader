<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Lighting" CullMode="CULL_BACK" BlendMode="Opaque">
	<property name="VertexShader"><![CDATA[#version 450

layout(binding = 0, std140) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

layout(location = 0) in vec3 a_Position;

void main()
{
    vec4 worldPosition = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    vec4 clipPosition = vs_ubo.u_ViewProjMatrix * worldPosition;
    gl_Position = clipPosition;
}

]]></property>
	<property name="FragmentShader"><![CDATA[#version 450

layout(location = 0) out vec4 o_FragDiffuse;
layout(location = 1) out vec4 o_FragSpecular;

void main()
{
    vec4 Color_627_Value = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 Color_626_Value = vec4(1.0);
    vec3 _Diffuse = Color_626_Value.xyz;
    vec3 _Specular = Color_627_Value.xyz;
    o_FragDiffuse = vec4(_Diffuse, 1.0);
    o_FragSpecular = vec4(_Specular, 1.0);
}

]]></property>
	<property name="Graph"><![CDATA[{
    "connections": [
        {
            "in_id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "in_index": 1,
            "out_id": "{e2eb8b65-65fd-425d-93c8-944c74cd8c75}",
            "out_index": 0
        },
        {
            "in_id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "in_index": 0,
            "out_id": "{c731144d-4f4c-495c-b87a-80b2798926cd}",
            "out_index": 0
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
                "x": -263,
                "y": 402
            }
        },
        {
            "id": "{920e9e49-c656-4d39-91aa-3ded63350483}",
            "model": {
                "Variable": "ShaderTemplate_624",
                "name": "ShaderTemplate"
            },
            "position": {
                "x": 0,
                "y": 288
            }
        },
        {
            "id": "{c731144d-4f4c-495c-b87a-80b2798926cd}",
            "model": {
                "Color": "1 1 1 1 ",
                "Uniform": "false",
                "Variable": "Color_626",
                "name": "Color"
            },
            "position": {
                "x": -264,
                "y": 302
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
