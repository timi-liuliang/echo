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

layout(binding = 1, std140) uniform UBO
{
    vec2 u_BrightTextureSize;
} fs_ubo;

layout(binding = 2) uniform sampler2D BrightTexture;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

vec3 GaussianBlur(sampler2D tex, vec2 size, vec2 uv)
{
    float radius = 3.0;
    float sigma = 0.840896427631378173828125;
    float twoSigmaSigma = (2.0 * sigma) * sigma;
    float twoPiSigmaSigma = 1.0 / (twoSigmaSigma * 3.141592502593994140625);
    vec2 texSizeInv = vec2(1.0) / size;
    vec4 color = vec4(0.0);
    float weights = 0.0;
    for (float w = -radius; w <= radius; w += 1.0)
    {
        for (float h = -radius; h <= radius; h += 1.0)
        {
            vec2 offset = vec2(w, h) * texSizeInv;
            float weight = (1.0 / twoPiSigmaSigma) * exp((-((w * w) + (h * h))) / twoSigmaSigma);
            color += (texture(tex, uv + offset) * weight);
            weights += weight;
        }
    }
    return color.xyz / vec3(weights);
}

void main()
{
    vec4 BrightTexture_Color = texture(BrightTexture, v_UV);
    vec2 param = fs_ubo.u_BrightTextureSize;
    vec2 param_1 = v_UV;
    vec3 GaussianBlur_345 = GaussianBlur(BrightTexture, param, param_1);
    vec3 _Emissive = GaussianBlur_345;
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
            "in_id": "{62988fea-922b-4721-8f29-5a5dc00babb5}",
            "in_index": 0,
            "out_id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "out_index": 0
        },
        {
            "in_id": "{62988fea-922b-4721-8f29-5a5dc00babb5}",
            "in_index": 2,
            "out_id": "{102f3f6e-9505-4bc6-b6f3-6b494a4ae905}",
            "out_index": 0
        },
        {
            "in_id": "{43d4e1da-ff31-4d37-b2ea-c57c20bade65}",
            "in_index": 1,
            "out_id": "{62988fea-922b-4721-8f29-5a5dc00babb5}",
            "out_index": 0
        },
        {
            "in_id": "{62988fea-922b-4721-8f29-5a5dc00babb5}",
            "in_index": 1,
            "out_id": "{394656fc-a8e2-447a-b1fa-d4127abcafe5}",
            "out_index": 1
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
                "x": -12,
                "y": 239
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
            "id": "{62988fea-922b-4721-8f29-5a5dc00babb5}",
            "model": {
                "Code": "vec3 GaussianBlur(sampler2D tex, vec2 size, vec2 uv)\n{\n\tfloat radius = 3.0;\n\tfloat sigma = 0.84089642;\n\tfloat twoSigmaSigma = 2 * sigma * sigma;\n\tfloat twoPiSigmaSigma = 1.0 / (twoSigmaSigma * 3.1415926);\n\tvec2  texSizeInv = 1.0 / size;\n\t\n\tvec4  color = vec4(0.0);\n\tfloat weights = 0.0;\n\tfor (float w = -radius; w <= radius; w++)\n\t{\n\t\tfor (float h = -radius; h <= radius; h++)\n\t\t{\n\t\t\tvec2 offset = vec2(w, h) * texSizeInv;\n\t\t\tfloat weight = 1.0 / twoPiSigmaSigma * exp(-(w*w+h*h) / twoSigmaSigma);\n\n\t\t\tcolor += texture(tex, uv + offset) * weight;\n\t\t\tweights += weight;\n\t\t}\n\t}\n\n\treturn color.xyz / weights;\n}",
                "FunctionName": "GaussianBlur",
                "Parameters": "sampler2D tex, vec2 size, vec2 uv",
                "ReturnType": "vec3",
                "Variable": "GaussianBlur_345",
                "name": "GaussianBlur"
            },
            "position": {
                "x": -288,
                "y": 272
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
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="false" WriteDepth="false" />
	</property>
</res>
