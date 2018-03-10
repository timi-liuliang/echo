#version 100
#include <fog_vs.inc>

#define MAX_BONE_NUM 38

attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec4 inBlendIndices;
attribute vec3 inBlendWeights;

uniform mat4 matWVP;
uniform mat4 matWorld;

uniform mediump vec4 naturalColor;
// uniform vec3 litDir;
// uniform vec4 litColor;
// uniform vec4 ambtColor;
// uniform vec3 fogParam;
// uniform vec4 ifogColor;
// uniform vec3 camPos;

uniform vec4 litParam[3];
uniform vec4 fogParam[3];

uniform vec4 boneMatRows[3*MAX_BONE_NUM];

varying vec4 ofogColor;
varying vec4 color;
varying vec2 texCoord;

vec3 MulBone3( vec3 vInputPos, int nMatrix, float fBlendWeight )
{
    vec3 vResult;
    vResult.x = dot( vInputPos, boneMatRows[3*nMatrix+0].xyz );
    vResult.y = dot( vInputPos, boneMatRows[3*nMatrix+1].xyz );
    vResult.z = dot( vInputPos, boneMatRows[3*nMatrix+2].xyz );
    return vResult * fBlendWeight;
}

vec3 MulBone4( vec4 vInputPos, int nMatrix, float fBlendWeight )
{
    vec3 vResult;
    vResult.x = dot( vInputPos, boneMatRows[(3*nMatrix)+0].xyzw );
    vResult.y = dot( vInputPos, boneMatRows[(3*nMatrix)+1].xyzw );
    vResult.z = dot( vInputPos, boneMatRows[(3*nMatrix)+2].xyzw );
    return vResult * fBlendWeight;
}

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	vec3 vPos;
	vec3 vNorm;
	
	 ivec3 BoneIndices;
	 BoneIndices.x = int(inBlendIndices.x);
	 BoneIndices.y = int(inBlendIndices.y);
	 BoneIndices.z = int(inBlendIndices.z);
	
  	vPos = MulBone4(inPosition, BoneIndices.x, inBlendWeights.x);
  	vPos += MulBone4(inPosition, BoneIndices.y, inBlendWeights.y);
  	vPos += MulBone4(inPosition, BoneIndices.z, inBlendWeights.z);
	
	vNorm = MulBone3(inNormal, BoneIndices.x, inBlendWeights.x);
	vNorm += MulBone3(inNormal, BoneIndices.y, inBlendWeights.y);
	vNorm += MulBone3(inNormal, BoneIndices.z, inBlendWeights.z);
	// blend vertex position & normal
	
	vec4 posV4 = vec4(vPos, 1.0);
	
	gl_Position = matWVP * posV4;
	texCoord = inTexCoord;
	
	vec3 vNormal = mat3(matWorld) * vNorm;
	color = (litParam[1] * max(0.0, dot(normalize(vNormal), litParam[0].xyz)) + litParam[2]) + naturalColor;
    
	vec3 worldPos = (matWorld*posV4).xyz;
	
	ofogColor = vec4(fogParam[1].rgb, ComputeFog(worldPos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogParam[1].rgb, worldPos.y);
}