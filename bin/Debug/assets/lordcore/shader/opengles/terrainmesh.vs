#version 100
#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec4 inColor;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matWorld;
uniform mat4 matWVP;
// uniform vec3 fogParam;
// uniform vec4 ifogColor;
// uniform vec3 cameraPos;
uniform vec4 fogParam[3];

varying vec4 blendines;
varying vec4 ofogColor;
varying vec2 texCoord;
varying vec2 texCoord1;

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	gl_Position = matWVP * vec4(inPosition + inNormal*vec3(0.000001,0.000001,0.000001), 1.0);
	blendines = inColor;
	texCoord = inTexCoord;
	texCoord1 = inTexCoord1;
	
	vec3 worldPos = (matWorld*vec4(inPosition, 1.0)).xyz;
	
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
	ofogColor = vec4(fogcolorLinear, ComputeFog(worldPos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, worldPos.y);
}