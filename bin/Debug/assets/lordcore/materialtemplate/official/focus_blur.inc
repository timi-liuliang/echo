
uniform mediump float blurWidth;
uniform mediump float blurStrength;

vec4 calcFocusBlur(in vec4 baseColor, in sampler2D baseTex, in vec2 texCoord)
{
	mediump float height = 1.0;
	mediump float width = 1.0;

	mediump vec2 focus = ((0.5 * vec2(width, height)) - texCoord);
	mediump float tmp = sqrt((focus.x * focus.x) + (focus.y * focus.y));
	mediump vec2 deltaUV = focus / tmp;

	mediump vec2 offsetUV1 = texCoord + ((deltaUV * -0.02) * blurWidth) * height;
	lowp vec4 offsetColor1 = texture2D(baseTex, offsetUV1);

	mediump vec2 offsetUV2  = texCoord + ((deltaUV * -0.01) * blurWidth) * height;
	lowp vec4 offsetColor2 = texture2D(baseTex, offsetUV2);

	mediump vec2 offsetUV3  = texCoord + ((deltaUV * 0.01) * blurWidth) * height;
	lowp vec4 offsetColor3 = texture2D (baseTex, offsetUV3);

	mediump vec2 offsetUV4  = texCoord + ((deltaUV * 0.02) * blurWidth) * height;
	lowp vec4 offsetColor4 = texture2D(baseTex, offsetUV4);

	return mix( baseColor, (baseColor + offsetColor1 + offsetColor2 + offsetColor3 + offsetColor4) * 0.2, 
	vec4(clamp((tmp * blurStrength) / height, 0.0, 1.0)) );
}
