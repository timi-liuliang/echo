#ifndef __ECHO_SKINNEDMESH_H__
#define __ECHO_SKINNEDMESH_H__

#include "Mesh.h"

namespace Echo
{
	class SkinnedMesh: public Mesh
	{
	public:
		friend class MeshManager;
		SkinnedMesh(const String& name);
		virtual ~SkinnedMesh();

		static const ui32 MAX_BONE_WEIGHT_NUM = 3;

#pragma pack(push, 1)
		struct SkinnedMeshVertex_Actor
		{
			Vector3	position;
			Vector3	normal;
			Vector2	texCoord;
			Byte	blendIndices[4];
			float	blendWeights[MAX_BONE_WEIGHT_NUM];
		};

		struct SkinnedMeshVertex_Scene
		{
			Vector3	position;
			Dword   color;
			Vector2	texCoord;
			Byte	blendIndices[4];
			float	blendWeights[MAX_BONE_WEIGHT_NUM];
		};
#pragma pack(pop)

	protected:
		virtual bool			prepareImpl( DataStream* stream);
		//virtual bool			loadImpl();
	};

}

#endif