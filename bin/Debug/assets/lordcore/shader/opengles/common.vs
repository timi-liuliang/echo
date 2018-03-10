#version 100

#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 color;
varying vec2 texCoord;


// for fog
uniform vec4 fogParam[3];
varying vec4 ofogColor;
float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	color = inColor;
	texCoord = inTexCoord;
    
    vec3 pos = inPosition.xyz;
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
    ofogColor = vec4(fogcolorLinear, ComputeFog(pos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, pos.y);
}