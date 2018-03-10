#version 100
#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec3 inTangent;	
attribute vec3 inBinormal;
attribute vec4 inColor;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matWorld;
uniform mat4 matWVP;
uniform mat4 matWVPSM;
uniform vec4 fogParam[3];

varying vec3 p_normal;
varying vec3 p_tangent;
varying vec3 p_binormal;
varying vec4 blendines;
varying vec4 ofogColor;
varying vec2 texCoord;
varying vec2 texCoord1;
varying vec4 lightmapSpace;
varying vec3 eye;

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	gl_Position  = matWVP * vec4(inPosition, 1.0);
	lightmapSpace= matWVPSM * vec4( inPosition, 1.0);
	blendines    = inColor;
	texCoord     = inTexCoord;
	texCoord1    = inTexCoord1;
	
	vec3 worldPos = (matWorld*vec4(inPosition, 1.0)).xyz;
	eye = fogParam[2].xyz - worldPos;
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
	ofogColor = vec4(fogcolorLinear, ComputeFog(worldPos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, worldPos.y);
	p_normal = mat3(matWorld) * inNormal;
	p_tangent = mat3(matWorld) * inTangent;
	p_binormal = mat3(matWorld) * inBinormal;
}