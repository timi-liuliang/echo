<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_NONE" BlendMode="Transparent" Uniforms.BrightTexture="Engine://Render/Pipeline/Framebuffer/ShadowDepth/ShadowDepthColorA.rt">
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

vec3 BoxBlur(sampler2D texSampler, vec2 texSize, vec2 uv, float radius)
{
    vec2 invSize = vec2(1.0) / texSize;
    vec4 color = vec4(0.0);
    float weights = 0.0;
    for (float x = -radius; x <= radius; x += 1.0)
    {
        for (float y = -radius; y <= radius; y += 1.0)
        {
            vec2 offset = vec2(x, y) * invSize;
            float weight = 1.0 - (sqrt((x * x) + (y * y)) / (radius * 1.41429996490478515625));
            color += (texture(texSampler, uv + offset) * weight);
            weights += weight;
        }
    }
    return color.xyz / vec3(weights);
}

void main()
{
    float Float_316_Value = 4.0;
    vec2 Vector2_356_Value = vec2(2048.0);
    vec2 param = Vector2_356_Value;
    vec2 param_1 = v_UV;
    float param_2 = Float_316_Value;
    vec3 GLSL_351 = BoxBlur(BrightTexture, param, param_1, param_2);
    vec3 _Emissive = GLSL_351;
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
            "in_id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "in_index": 0,
            "out_id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "out_index": 0
        },
        {
            "in_id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "in_index": 1,
            "out_id": "{3934e5f9-18a5-4330-87ff-90eeb936a407}",
            "out_index": 0
        },
        {
            "in_id": "{43d4e1da-ff31-4d37-b2ea-c57c20bade65}",
            "in_index": 1,
            "out_id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "out_index": 0
        },
        {
            "in_id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "in_index": 3,
            "out_id": "{014cbe53-0dc0-408e-941c-6b1b14619efb}",
            "out_index": 0
        },
        {
            "in_id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "in_index": 2,
            "out_id": "{102f3f6e-9505-4bc6-b6f3-6b494a4ae905}",
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
                "x": -17,
                "y": 248
            }
        },
        {
            "id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/ShadowDepth/ShadowDepthColorA.rt",
                "Type": "General",
                "Variable": "BrightTexture",
                "name": "Texture"
            },
            "position": {
                "x": -649,
                "y": 274
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
                "x": -706,
                "y": 485
            }
        },
        {
            "id": "{b158b18b-022f-43fd-af41-cf4245c938c5}",
            "model": {
                "Code": "vec3 BoxBlur(sampler2D texSampler, vec2 texSize, vec2 uv, float radius)\n{\n\tvec2 invSize = 1.0 / texSize;\n\t\n\tvec4  color = vec4(0.0);\n\tfloat weights = 0.0;\n\tfor(float x=-radius; x<=radius; x++)\n\t{\n\t\tfor(float y=-radius; y<=radius; y++)\n\t\t{\n\t\t\tvec2 offset = vec2(x, y) * invSize;\n\t\t\tfloat weight = 1.0 - sqrt(x*x + y*y) / (radius * 1.4143);\n\t\t\t\n\t\t\tcolor += texture(texSampler, uv+offset) * weight;\n\t\t\tweights += weight;\n\t\t}\n\t}\n\n\treturn color.rgb / weights;\n}",
                "FunctionName": "BoxBlur",
                "Parameters": "sampler2D texSampler, vec2 texSize, vec2 uv, float radius",
                "ReturnType": "vec3",
                "Variable": "GLSL_351",
                "name": "GLSL"
            },
            "position": {
                "x": -318,
                "y": 276
            }
        },
        {
            "id": "{014cbe53-0dc0-408e-941c-6b1b14619efb}",
            "model": {
                "Uniform": "false",
                "Value": "4.0",
                "Variable": "Float_316",
                "name": "Float"
            },
            "position": {
                "x": -624,
                "y": 573
            }
        },
        {
            "id": "{3934e5f9-18a5-4330-87ff-90eeb936a407}",
            "model": {
                "Uniform": "false",
                "Value": "2048 2048",
                "Variable": "Vector2_356",
                "name": "Vector2"
            },
            "position": {
                "x": -572,
                "y": 413
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="false" WriteDepth="false" />
	</property>
</res>
