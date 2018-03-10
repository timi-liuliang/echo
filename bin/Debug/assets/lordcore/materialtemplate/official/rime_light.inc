uniform vec4 uRimeColor;
uniform vec2 uRimeParam;
uniform float uFadeScale;

vec3 calcRimeColor(in vec3 inNormal, in vec3 inView)
{
	vec3 result = srgb_to_linear(uRimeColor.xyz);
	result = result * pow( (1.0 - clamp(dot(inNormal, inView), -1.0, 1.0)), uRimeParam.y ) * uRimeParam.x * uFadeScale;

	return result;
}