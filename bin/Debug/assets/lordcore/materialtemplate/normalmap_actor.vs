#version 100
#include <fog_vs.inc>

attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVP;
uniform mat4 matW;
uniform mat4 matWV;

varying vec2 texcoord0;
varying vec3 viewNormal;
varying vec3 viewDir;

uniform vec4 fogParam[3];
varying vec4 ofogColor;

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

#ifdef NORMAL_MAP
	attribute vec3 inTangent;	
	attribute vec3 inBinormal;
	varying   vec3 tangent;
	varying   vec3 binormal;
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
	
	// 骨节旋转
	vec3 RotationByBone(vec3 inPos)
	{
		vec3  result;
		result  = MulBone3(inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone3(inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone3(inPos, int(inBlendIndices.z), inBlendWeights.z);
		return result;
	}
	
	// 骨节转换(旋转平移)
	vec4 TransformByBone(vec4 inPos)
	{
		vec3  result;
		result  = MulBone4(inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone4(inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone4(inPos, int(inBlendIndices.z), inBlendWeights.z);
		return vec4(result, 1.0);
	}
#else
	// 骨节旋转
	vec3 RotationByBone(vec3 inPos)
	{
		return inPos;
	}
	
	// 骨节转换(旋转平移)
	vec4 TransformByBone(vec4 inPos)
	{
		return inPos;
	}	
#endif

void main ()
{
	texcoord0 = inTexCoord;

	vec4 P = TransformByBone(vec4(inPosition.xyz, 1.0));

#ifdef NORMAL_MAP
	tangent = mat3(matWV) * RotationByBone(inTangent);
	binormal = mat3(matWV) * RotationByBone(inBinormal);
#endif
	
	viewNormal = mat3(matWV) * RotationByBone(inNormal);
	viewDir = vec3(0.0, 0.0, 0.0) - (matWV * P).xyz;
	gl_Position = matWVP * P;

	vec3 camPos = (matW * vec4(inPosition.xyz, 1.0)).xyz;
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
    ofogColor = vec4(fogcolorLinear, ComputeFog(camPos-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, camPos.y);
}



