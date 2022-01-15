<?xml version="1.0" encoding="utf-8"?>
<res class="ShaderProgram" path="Engine://Render/Pipeline/Shaders/Ui/UiImage.shader" Type="glsl" VertexShader="#version 450&#10;&#10;layout(binding = 0, std140) uniform UBO&#10;{&#10;    mat4 u_WorldMatrix;&#10;    mat4 u_ViewProjMatrix;&#10;} vs_ubo;&#10;&#10;layout(location = 0) in vec3 a_Position;&#10;layout(location = 7) out vec2 v_UV;&#10;layout(location = 4) in vec2 a_UV;&#10;&#10;void main()&#10;{&#10;    vec4 worldPosition = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);&#10;    vec4 clipPosition = vs_ubo.u_ViewProjMatrix * worldPosition;&#10;    gl_Position = clipPosition;&#10;    v_UV = a_UV;&#10;}&#10;&#10;" FragmentShader="#version 450&#10;&#10;layout(binding = 1) uniform sampler2D SrcTexture;&#10;&#10;layout(location = 7) in vec2 v_UV;&#10;layout(location = 0) out vec4 o_FragColor;&#10;&#10;void main()&#10;{&#10;    vec4 SrcTexture_Color = texture(SrcTexture, v_UV);&#10;    vec4 Tint_Value = vec4(1.0);&#10;    vec3 Multiplication_477 = SrcTexture_Color.xyz * Tint_Value.xyz;&#10;    vec3 _BaseColor = Multiplication_477;&#10;    float _Opacity = 1.0;&#10;    float _Metalic = 0.20000000298023223876953125;&#10;    float _PerceptualRoughness = 0.5;&#10;    o_FragColor = vec4(_BaseColor, _Opacity);&#10;}&#10;&#10;" Graph="{&#10;    &quot;connections&quot;: [&#10;        {&#10;            &quot;converter&quot;: {&#10;                &quot;in&quot;: {&#10;                    &quot;id&quot;: &quot;any&quot;,&#10;                    &quot;name&quot;: &quot;A&quot;&#10;                },&#10;                &quot;out&quot;: {&#10;                    &quot;id&quot;: &quot;vec3&quot;,&#10;                    &quot;name&quot;: &quot;rgb&quot;&#10;                }&#10;            },&#10;            &quot;in_id&quot;: &quot;{873b5701-4716-45f0-8512-345c1e7c366e}&quot;,&#10;            &quot;in_index&quot;: 0,&#10;            &quot;out_id&quot;: &quot;{ba33ad30-cfba-4d00-ad00-717683cf6330}&quot;,&#10;            &quot;out_index&quot;: 1&#10;        },&#10;        {&#10;            &quot;in_id&quot;: &quot;{1c1a7864-4b70-4be8-912e-c6ea515df52b}&quot;,&#10;            &quot;in_index&quot;: 0,&#10;            &quot;out_id&quot;: &quot;{873b5701-4716-45f0-8512-345c1e7c366e}&quot;,&#10;            &quot;out_index&quot;: 0&#10;        },&#10;        {&#10;            &quot;converter&quot;: {&#10;                &quot;in&quot;: {&#10;                    &quot;id&quot;: &quot;any&quot;,&#10;                    &quot;name&quot;: &quot;B&quot;&#10;                },&#10;                &quot;out&quot;: {&#10;                    &quot;id&quot;: &quot;vec3&quot;,&#10;                    &quot;name&quot;: &quot;rgb&quot;&#10;                }&#10;            },&#10;            &quot;in_id&quot;: &quot;{873b5701-4716-45f0-8512-345c1e7c366e}&quot;,&#10;            &quot;in_index&quot;: 1,&#10;            &quot;out_id&quot;: &quot;{2bec65dc-37ce-4976-a9a9-9e401b19debb}&quot;,&#10;            &quot;out_index&quot;: 0&#10;        }&#10;    ],&#10;    &quot;nodes&quot;: [&#10;        {&#10;            &quot;id&quot;: &quot;{1c1a7864-4b70-4be8-912e-c6ea515df52b}&quot;,&#10;            &quot;model&quot;: {&#10;                &quot;Variable&quot;: &quot;ShaderTemplate_474&quot;,&#10;                &quot;name&quot;: &quot;ShaderTemplate&quot;&#10;            },&#10;            &quot;position&quot;: {&#10;                &quot;x&quot;: 404,&#10;                &quot;y&quot;: 303&#10;            }&#10;        },&#10;        {&#10;            &quot;id&quot;: &quot;{873b5701-4716-45f0-8512-345c1e7c366e}&quot;,&#10;            &quot;model&quot;: {&#10;                &quot;Variable&quot;: &quot;Multiplication_477&quot;,&#10;                &quot;name&quot;: &quot;Multiplication&quot;&#10;            },&#10;            &quot;position&quot;: {&#10;                &quot;x&quot;: 126,&#10;                &quot;y&quot;: 304&#10;            }&#10;        },&#10;        {&#10;            &quot;id&quot;: &quot;{ba33ad30-cfba-4d00-ad00-717683cf6330}&quot;,&#10;            &quot;model&quot;: {&#10;                &quot;Atla&quot;: &quot;false&quot;,&#10;                &quot;Texture&quot;: &quot;Engine://Render/Pipeline/Shaders/Ui/White.png&quot;,&#10;                &quot;Type&quot;: &quot;General&quot;,&#10;                &quot;Variable&quot;: &quot;SrcTexture&quot;,&#10;                &quot;name&quot;: &quot;Texture&quot;&#10;            },&#10;            &quot;position&quot;: {&#10;                &quot;x&quot;: -189,&#10;                &quot;y&quot;: 208&#10;            }&#10;        },&#10;        {&#10;            &quot;id&quot;: &quot;{2bec65dc-37ce-4976-a9a9-9e401b19debb}&quot;,&#10;            &quot;model&quot;: {&#10;                &quot;Color&quot;: &quot;1 1 1 1 &quot;,&#10;                &quot;Variable&quot;: &quot;Tint&quot;,&#10;                &quot;name&quot;: &quot;Color&quot;&#10;            },&#10;            &quot;position&quot;: {&#10;                &quot;x&quot;: -105,&#10;                &quot;y&quot;: 360&#10;            }&#10;        }&#10;    ]&#10;}&#10;" CullMode="CULL_BACK" BlendMode="Transparent" Uniforms.SrcTexture="Engine://Render/Pipeline/Shaders/Ui/White.png">
	<property name="DepthStencilState">
		<obj class="DepthStencilState" DepthEnable="true" WriteDepth="true" />
	</property>
</res>
