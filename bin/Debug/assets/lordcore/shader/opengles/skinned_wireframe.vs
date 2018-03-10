#version 100

#define MAX_BONE_NUM 38

attribute vec4 inPosition;
attribute vec4 inColor;
attribute vec4 inBlendIndices;
attribute vec3 inBlendWeights;

uniform mat4 matWVP;
uniform vec4 boneMatRows[3*MAX_BONE_NUM];

varying vec4 color;

vec3 MulBone4( vec4 vInputPos, int nMatrix, float fBlendWeight )
{
    vec3 vResult;
    vResult.x = dot( vInputPos, boneMatRows[(3*nMatrix)+0].xyzw );
    vResult.y = dot( vInputPos, boneMatRows[(3*nMatrix)+1].xyzw );
    vResult.z = dot( vInputPos, boneMatRows[(3*nMatrix)+2].xyzw );
    return vResult * fBlendWeight;
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
	
	// blend vertex position & normal
	vec4 posV4 = vec4(vPos, 1.0);
	
	gl_Position = matWVP * posV4;
	color = inColor;
}
