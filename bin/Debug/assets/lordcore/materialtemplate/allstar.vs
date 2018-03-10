#version 100

/**
 * shader所有函数都是内联编译，无须担心函数效率
 */

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matW;
uniform mat4 matWVP;

varying vec2 texCoord;
varying vec2 texCoord1;
varying vec3 normal;
varying vec3 position;

#ifdef RECEIVE_SHADOW
	uniform mat4 matWVPSM;
	varying vec4 shadowmapSpace;
#endif

#ifdef NORMAL_MAP
	attribute vec3 inTangent;	
	varying   vec3 tangent;
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

void main(void)
{
	gl_Position = matWVP * TransformByBone(vec4(inPosition, 1.0));
	texCoord = inTexCoord;
	texCoord1 = inTexCoord1;
	
	position = (matW * TransformByBone(vec4(inPosition, 1.0))).xyz;
	
#ifdef RECEIVE_SHADOW
	shadowmapSpace= matWVPSM * TransformByBone(vec4(inPosition, 1.0));
#endif

	normal = mat3(matW) * RotationByBone(inNormal);
	
#ifdef NORMAL_MAP
	tangent = mat3(matW) * RotationByBone(inTangent);
#endif
}