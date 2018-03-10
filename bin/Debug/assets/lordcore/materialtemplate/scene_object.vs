#version 100

#include <fog_vs.inc>

attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVP;
uniform mat4 matWV;
uniform mat4 matW;
uniform mat4 matV;

varying vec2 texcoord0;
varying vec3 viewNormal;

#ifdef LIGHT_FROM_LIGHTARRAY
    uniform mediump vec4 LightArrayData[16];
    
    varying vec3 lightDir0;
    varying vec3 lightDir1;
#endif

#ifdef LIGHT_FROM_SCENE
    uniform mediump vec4 SceneObjectLightParams[3];
    varying vec3 lightDir2;
#endif

#ifdef RECEIVE_FOG
	uniform vec4 fogParam[3];
	varying vec4 ofogColor;

	float ComputeFog(vec3 camToWorldPos, vec3 param)
	{
		float fdist = max(length(camToWorldPos) - param.x, 0.0);
		float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
		return 1.0 - density;
	}
#endif 

#ifdef SKIN_MESH
	attribute vec4 inBlendIndices;
	attribute vec3 inBlendWeights;
	
	#define MAX_BONE_NUM 38
	uniform vec4 matBoneRows[3*MAX_BONE_NUM];

	vec3 MulBone3(vec3 inPos, int boneIdx, float weight)
	{
		vec3 result;
		result.x = dot(inPos, matBoneRows[3*boneIdx+0].xyz);
		result.y = dot(inPos, matBoneRows[3*boneIdx+1].xyz);
		result.z = dot(inPos, matBoneRows[3*boneIdx+2].xyz);
		return result * weight;
	}

	vec3 MulBone4(vec4 inPos, int boneIdx, float weight)
	{
		vec3 result;
		result.x = dot(inPos, matBoneRows[(3*boneIdx)+0].xyzw);
		result.y = dot(inPos, matBoneRows[(3*boneIdx)+1].xyzw);
		result.z = dot(inPos, matBoneRows[(3*boneIdx)+2].xyzw);
		return result * weight;
	}
	
	vec3 RotationByBone(vec3 inPos)
	{
		vec3  result;
		result  = MulBone3(inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone3(inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone3(inPos, int(inBlendIndices.z), inBlendWeights.z);
		return result;
	}
	
	vec4 TransformByBone(vec4 inPos)
	{
		vec3  result;
		result  = MulBone4(inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone4(inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone4(inPos, int(inBlendIndices.z), inBlendWeights.z);
		return vec4(result, 1.0);
	}
#else
	vec3 RotationByBone(vec3 inPos)
	{
		return inPos;
	}
	
	vec4 TransformByBone(vec4 inPos)
	{
		return inPos;
	}	
#endif

#ifdef ENV_MAP
	varying vec3 position1;
	varying vec3 normal;
#endif

void main ()
{
	texcoord0 = inTexCoord;

	vec4 P = TransformByBone(vec4(inPosition.xyz, 1.0));

#ifdef LIGHT_FROM_LIGHTARRAY
    lightDir0 = mat3(matV) * -LightArrayData[1].xyz;
    lightDir1 = mat3(matV) * -LightArrayData[3].xyz;
#endif

#ifdef LIGHT_FROM_SCENE
    lightDir2 = mat3(matV) * SceneObjectLightParams[0].xyz;
#endif
    
	viewNormal = mat3(matWV) * RotationByBone(inNormal);
    
#ifdef RECEIVE_FOG
    vec3 position = (matW * P).xyz;
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
    ofogColor = vec4(fogcolorLinear, ComputeFog(position-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, position.y);
#endif
	
#ifdef ENV_MAP
	position1 = (matW*P).xyz;
	normal = (mat3(matW)*RotationByBone(inNormal)).xyz;
#endif
	
	gl_Position = matWVP * P;
}



