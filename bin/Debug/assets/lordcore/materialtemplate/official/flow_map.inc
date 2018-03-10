uniform mediump vec2 uFlowDir;
uniform sampler2D 	 tFlowSampler;
uniform mediump float CurrentTime;
uniform mediump float uFlowSpeed;

vec4 calcFlowMapColor(in vec2 texCoord)
{
	vec2 newTexCoord = texCoord + uFlowDir * CurrentTime * uFlowSpeed;
	vec4 outColor = texture2d_linear(tFlowSampler, newTexCoord);

	return outColor;
}

vec4 mixFlowColor(in vec4 inColor, in vec4 flowColor, in float weight)
{
	//vec4 finalColor = mix(inColor, mix(inColor, flowColor, flowColor.a), 1.0);
	vec4 finalColor = vec4( ( vec3(1.0) - (vec3(1.0) - inColor.rgb) * (vec3(1.0) - flowColor.rgb * flowColor.a * weight) ) , 1.0);

	return finalColor;
}
