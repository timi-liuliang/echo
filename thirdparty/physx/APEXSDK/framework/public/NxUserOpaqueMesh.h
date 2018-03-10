/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_OPAQUE_MESH_H
#define NX_USER_OPAQUE_MESH_H

/*!
\file
\brief class NxUserOpaqueMesh
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

//! \brief Name of NxOpaqueMesh authoring type namespace
#define APEX_OPAQUE_MESH_NAME_SPACE "ApexOpaqueMesh"

/**
\brief Opaque mesh description
*/
class NxUserOpaqueMeshDesc
{
public:
	///the name of the opaque mesh
	const char* mMeshName;
};

/**
\brief An abstract interface to an opaque mesh
*
* An 'opaque' mesh is a binding between the 'name' of a mesh and some internal mesh representation used by the
* application.  This allows the application to refer to meshes by name without involving duplciation of index buffer and
* vertex buffer data declarations.
*/
class NxUserOpaqueMesh
{
public:
	virtual ~NxUserOpaqueMesh() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
