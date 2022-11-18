<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.BrightTexture="Engine://Render/Pipeline/Framebuffer/Bloom/Color.rt">
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

layout(binding = 2) uniform sampler2D BrightTexture;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

vec3 GaussianBlur(sampler2D tex, vec2 uv, float radius, float dirs, float samples, float weight, float strength)
{
    float _step = (1.0 / samples) * radius;
    vec4 origin = texture(tex, uv);
    vec4 color = origin;
    float count = 1.0;
    for (float pi = 6.283185482025146484375, directions = dirs, d = 0.0; d < pi; d += (pi / directions))
    {
        for (float i = 1.0; i <= samples; i += 1.0)
        {
            float weightPow = pow(clamp(1.0 - (i / samples), 0.0, 1.0), weight);
            color += (texture(tex, uv + ((vec2(cos(d), sin(d)) * _step) * i)) * weightPow);
            count += weightPow;
        }
    }
    color /= vec4(count);
    return mix(origin.xyz, color.xyz, vec3(strength));
}

void main()
{
    float Float_317_Value = 8.0;
    float Float_318_Value = 1.0;
    float Float_316_Value = 16.0;
    float Float_315_Value = 0.02999000065028667449951171875;
    float Float_319_Value = 1.0;
    vec2 param = v_UV;
    float param_1 = Float_315_Value;
    float param_2 = Float_316_Value;
    float param_3 = Float_317_Value;
    float param_4 = Float_318_Value;
    float param_5 = Float_319_Value;
    vec3 GaussianBlur_311 = GaussianBlur(BrightTexture, param, param_1, param_2, param_3, param_4, param_5);
    vec3 _Emissive = GaussianBlur_311;
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
            "in_id": "{43d4e1da-ff31-4d37-b2ea-c57c20bade65}",
            "in_index": 1,
            "out_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 0,
            "out_id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 1,
            "out_id": "{102f3f6e-9505-4bc6-b6f3-6b494a4ae905}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 3,
            "out_id": "{014cbe53-0dc0-408e-941c-6b1b14619efb}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 2,
            "out_id": "{63a9f41d-5f44-46aa-b2af-7fd3b72e5571}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 6,
            "out_id": "{9e1679d4-1b3a-4a99-b7c2-61a168b420d9}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 4,
            "out_id": "{ddceded9-109e-4e23-a150-7059e1dcaa99}",
            "out_index": 0
        },
        {
            "in_id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "in_index": 5,
            "out_id": "{96ee33b4-aec3-4e34-a3e5-7233af4a9703}",
            "out_index": 0
        }
    ],
    "nodes": [
        {
            "id": "{43d4e1da-ff31-4d37-b2ea-c57c20bade65}",
            "model": {
                "Variable": "ShaderTemplateTransparent_309",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": 0,
                "y": 246
            }
        },
        {
            "id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/Bloom/Color.rt",
                "Type": "General",
                "Variable": "BrightTexture",
                "name": "Texture"
            },
            "position": {
                "x": -649,
                "y": 275
            }
        },
        {
            "id": "{f5b8c3aa-b999-4d11-897f-58cd09a66dfb}",
            "model": {
                "Code": "vec3 GaussianBlur(sampler2D tex,vec2 uv, float radius, float dirs, float samples, float weight, float strength)\n{\n\t// https://www.shadertoy.com/view/Xltfzj\n\tfloat pi = 6.28318530718;\n\tfloat directions = dirs;\t// blur directions (default 16.0 - more is better but slower)\n\tfloat step = 1.0 / samples * radius;\n\tvec4 origin = texture(tex, uv);\n\tvec4 color = origin;\n\tfloat count = 1.0;\n\t\n\tfor (float d = 0.0; d < pi; d += pi / directions)\n\t{\n\t\tfor (float i = 1.0; i <= samples; i += 1.0)\n\t\t{\n\t\t\tfloat weightPow = pow(clamp(1.0 - i / samples, 0.0, 1.0), weight);\n\t\t\tcolor += texture(tex, uv + vec2(cos(d), sin(d)) * step * i) * weightPow;\n\t\t\tcount += weightPow;\n\t\t}\n\t}\n\n\tcolor /= count;\n\n\treturn mix(origin.xyz, color.xyz, strength);\n}",
                "FunctionName": "GaussianBlur",
                "Parameters": "sampler2D tex,vec2 uv, float radius, float dirs, float samples, float weight, float strength",
                "ReturnType": "vec3",
                "Variable": "GaussianBlur_311",
                "name": "GaussianBlur"
            },
            "position": {
                "x": -288,
                "y": 275
            }
        },
        {
            "id": "{ddceded9-109e-4e23-a150-7059e1dcaa99}",
            "model": {
                "Uniform": "false",
                "Value": "8.0",
                "Variable": "Float_317",
                "name": "Float"
            },
            "position": {
                "x": -625,
                "y": 632
            }
        },
        {
            "id": "{96ee33b4-aec3-4e34-a3e5-7233af4a9703}",
            "model": {
                "Uniform": "false",
                "Value": "1.0",
                "Variable": "Float_318",
                "name": "Float"
            },
            "position": {
                "x": -624,
                "y": 694
            }
        },
        {
            "id": "{102f3f6e-9505-4bc6-b6f3-6b494a4ae905}",
            "model": {
                "Attribute": "uv0",
                "Variable": "VertexAttribute_312",
                "name": "VertexAttribute"
            },
            "position": {
                "x": -705,
                "y": 417
            }
        },
        {
            "id": "{014cbe53-0dc0-408e-941c-6b1b14619efb}",
            "model": {
                "Uniform": "false",
                "Value": "16.0",
                "Variable": "Float_316",
                "name": "Float"
            },
            "position": {
                "x": -628,
                "y": 566
            }
        },
        {
            "id": "{63a9f41d-5f44-46aa-b2af-7fd3b72e5571}",
            "model": {
                "Uniform": "false",
                "Value": "0.02999",
                "Variable": "Float_315",
                "name": "Float"
            },
            "position": {
                "x": -626,
                "y": 498
            }
        },
        {
            "id": "{9e1679d4-1b3a-4a99-b7c2-61a168b420d9}",
            "model": {
                "Uniform": "false",
                "Value": "1.0",
                "Variable": "Float_319",
                "name": "Float"
            },
            "position": {
                "x": -621,
                "y": 756
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
