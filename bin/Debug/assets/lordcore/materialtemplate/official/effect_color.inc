uniform vec4 uEffectColor;

vec4 ComputeEffectColor(vec4 color)
{
	return color * uEffectColor;
}
