#version 100
#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWorld;
uniform mat4 matWVP;
uniform mediump vec4 naturalColor;
//uniform vec2 UVScale;
//uniform vec3 litDir;
//uniform vec4 litColor;
//uniform vec4 ambtColor;
//uniform vec3 fogParam;
//uniform vec4 ifogColor;
//uniform vec3 cameraPos;
uniform vec4 litParam[3];
uniform vec4 fogParam[3];


varying vec4 ofogColor;
varying vec4 color;
varying vec2 texCoord;

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	vec4 vPos = vec4(inPosition, 1.0);
	gl_Position = matWVP * vPos;
	texCoord = inTexCoord;//+UVScale;

	vec3 vNormal = mat3(matWorld) * inNormal;
	color = litParam[1] * max(dot(normalize(vNormal), litParam[0].xyz), 0.0) + litParam[2] + naturalColor;
	
	vec3 worldPos = (matWorld*vPos).xyz;
	
	ofogColor = vec4(fogParam[1].rgb, ComputeFog(worldPos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogParam[1].rgb, worldPos.y);
}