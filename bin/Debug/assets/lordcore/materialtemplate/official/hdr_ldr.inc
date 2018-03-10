vec4 hdr2ldr(in vec4 color)
{
#ifdef IS_FOR_UI
	vec4 result = color;
	result.xyz = hv(toneMapping(color.xyz));
	return result;
#else
	return color;
#endif
}

vec4 ldr2hdr(in vec4 color)
{
	return color;
}

vec4 texture2d_ldr2hdr( sampler2D sampler, vec2 uv)
{
	vec4 color = texture2D( sampler, uv);
	return color;
} 