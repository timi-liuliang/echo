<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" Type="glsl" Domain="Surface" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.TransparentResult="Engine://Render/Pipeline/Framebuffer/Translucency/Color.rt">
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

layout(binding = 2) uniform sampler2D TransparentResult;

layout(location = 7) in vec2 v_UV;
layout(location = 0) out vec4 o_FragColor;

vec3 ExtractingBrightColor(vec3 color, float threshold)
{
    float brightness = dot(color, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
    float t = smoothstep(threshold - 0.0500000007450580596923828125, threshold + 0.0500000007450580596923828125, brightness);
    return mix(vec3(0.0), color, vec3(t));
}

void main()
{
    vec4 TransparentResult_Color = texture(TransparentResult, v_UV);
    float Float_294_Value = 0.7799999713897705078125;
    vec3 param = TransparentResult_Color.xyz;
    float param_1 = Float_294_Value;
    vec3 GLSL_293 = ExtractingBrightColor(param, param_1);
    vec3 _Emissive = GLSL_293;
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
            "in_id": "{61e56723-7b94-4f88-8160-79adffe762be}",
            "in_index": 1,
            "out_id": "{85aa3e36-349e-416a-9fb8-445af41bb6b2}",
            "out_index": 0
        },
        {
            "in_id": "{85aa3e36-349e-416a-9fb8-445af41bb6b2}",
            "in_index": 0,
            "out_id": "{78e85592-fc0c-4037-96a4-2b04d2259a6b}",
            "out_index": 1
        },
        {
            "in_id": "{85aa3e36-349e-416a-9fb8-445af41bb6b2}",
            "in_index": 1,
            "out_id": "{380b116d-a58d-4e47-abd4-cfb1eea30282}",
            "out_index": 0
        }
    ],
    "nodes": [
        {
            "id": "{61e56723-7b94-4f88-8160-79adffe762be}",
            "model": {
                "Variable": "ShaderTemplateTransparent_291",
                "name": "ShaderTemplateTransparent"
            },
            "position": {
                "x": -39,
                "y": 329
            }
        },
        {
            "id": "{78e85592-fc0c-4037-96a4-2b04d2259a6b}",
            "model": {
                "Atla": "false",
                "Texture": "Engine://Render/Pipeline/Framebuffer/Translucency/Color.rt",
                "Type": "General",
                "Variable": "TransparentResult",
                "name": "Texture"
            },
            "position": {
                "x": -585,
                "y": 285
            }
        },
        {
            "id": "{85aa3e36-349e-416a-9fb8-445af41bb6b2}",
            "model": {
                "Code": "vec3 ExtractingBrightColor(vec3 color, float threshold)\n{\n\tfloat brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));\n\tfloat t = smoothstep(threshold-0.05, threshold+0.05, brightness);\n\n\treturn mix(vec3(0.0,0.0,0.0), color, t);\n}",
                "FunctionName": "ExtractingBrightColor",
                "Parameters": "vec3 color, float threshold",
                "ReturnType": "vec3",
                "Variable": "GLSL_293",
                "name": "GLSL"
            },
            "position": {
                "x": -296,
                "y": 361
            }
        },
        {
            "id": "{380b116d-a58d-4e47-abd4-cfb1eea30282}",
            "model": {
                "Uniform": "false",
                "Value": "0.77999",
                "Variable": "Float_294",
                "name": "Float"
            },
            "position": {
                "x": -558,
                "y": 446
            }
        }
    ]
}
]]></property>
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="false" WriteDepth="false" />
	</property>
</res>
