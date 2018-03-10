#version 100

attribute vec3 inPosition;
attribute vec2 inTexCoord;

#ifdef ALPHA_TEST
varying mediump vec2 texCoord;
#endif

uniform mat4 matWVPSM;

#ifdef SKIN_MESH
	attribute vec4 inBlendIndices;
	attribute vec3 inBlendWeights;
	
	#define MAX_BONE_NUM 38
	uniform vec4 matBoneRows[3*MAX_BONE_NUM];

	vec3 MulBone3( vec3 inPos, int boneIdx, float weight)
	{
		vec3 result;
		result.x = dot( inPos, matBoneRows[3*boneIdx+0].xyz);
		result.y = dot( inPos, matBoneRows[3*boneIdx+1].xyz);
		result.z = dot( inPos, matBoneRows[3*boneIdx+2].xyz);
		return result * weight;
	}

	vec3 MulBone4( vec4 inPos, int boneIdx, float weight)
	{
		vec3 result;
		result.x = dot( inPos, matBoneRows[(3*boneIdx)+0].xyzw);
		result.y = dot( inPos, matBoneRows[(3*boneIdx)+1].xyzw);
		result.z = dot( inPos, matBoneRows[(3*boneIdx)+2].xyzw);
		return result * weight;
	}
	
	// 骨节旋转
	vec3 RotationByBone( vec3 inPos)
	{
		vec3  result;
		result  = MulBone3( inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone3( inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone3( inPos, int(inBlendIndices.z), inBlendWeights.z);
		return result;
	}
	
	// 骨节转换(旋转平移)
	vec4 TransformByBone( vec4 inPos)
	{
		vec3  result;
		result  = MulBone4( inPos, int(inBlendIndices.x), inBlendWeights.x);
		result += MulBone4( inPos, int(inBlendIndices.y), inBlendWeights.y);
		result += MulBone4( inPos, int(inBlendIndices.z), inBlendWeights.z);
		return vec4( result, 1.0);
	}
#else
	// 骨节旋转
	vec3 RotationByBone( vec3 inPos)
	{
		return inPos;
	}
	
	// 骨节转换(旋转平移)
	vec4 TransformByBone( vec4 inPos)
	{
		return inPos;
	}	
#endif

varying vec4 vScreenPos;

void main(void)
{
	gl_Position = matWVPSM * TransformByBone( vec4(inPosition, 1.0));
	vScreenPos = gl_Position;
	
	#ifdef ALPHA_TEST
		texCoord = inTexCoord;
	#else
		vScreenPos.x += inTexCoord.x * 0.000001;
	#endif
}