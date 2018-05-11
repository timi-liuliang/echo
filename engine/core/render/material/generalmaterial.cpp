#include "generalmaterial.h"

// Ä¬ÈÏ²ÄÖÊ
static const char* g_generalMaterial = "\
<?xml version = \"1.0\" encoding = \"GB2312\"?>\
<material> \
<vs>#version 100\n\
\n\
attribute vec3 inPosition;\n\
attribute vec2 inTexCoord;\n\
\n\
uniform mat4 matWVP;\n\
\n\
varying vec2 texCoord;\n\
\n\
void main(void)\n\
{\n\
	vec4 position = matWVP * vec4(inPosition, 1.0);\n\
	gl_Position = position;\n\
	\n\
	texCoord = inTexCoord;\n\
}\n\
</vs>\
<ps>#version 100\n\
\n\
uniform sampler2D DiffuseSampler;\n\
varying mediump vec2 texCoord;\n\
\n\
void main(void)\n\
{\n\
	mediump vec4 textureColor = texture2D(DiffuseSampler, texCoord);\n\
	gl_FragColor = textureColor;\n\
}\n\
	</ps>\
	<BlendState>\
		<BlendEnable value = \"true\" />\
		<SrcBlend value = \"BF_SRC_ALPHA\" />\
		<DstBlend value = \"BF_INV_SRC_ALPHA\" />\
	</BlendState>\
	<RasterizerState>\
		<CullMode value = \"CULL_NONE\" />\
	</RasterizerState>\
	<DepthStencilState>\
		<DepthEnable value = \"false\" />\
		<WriteDepth value = \"false\" />\
	</DepthStencilState>\
	<SamplerState>\
		<BiLinearMirror>\
			<MinFilter value = \"FO_LINEAR\" />\
			<MagFilter value = \"FO_LINEAR\" />\
			<MipFilter value = \"FO_NONE\" />\
			<AddrUMode value = \"AM_CLAMP\" />\
			<AddrVMode value = \"AM_CLAMP\" />\
		</BiLinearMirror>\
	</SamplerState>\
	<Texture>\
		<stage no = \"0\" sampler = \"BiLinearMirror\" />\
	</Texture>\
</material>";

namespace Echo
{
	// get shader
	const char* GeneralMaterial::getContent()
	{
		return g_generalMaterial;
	}
}