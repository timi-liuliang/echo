/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PxGeometry.h"
#include "PxcMaterialMethodImpl.h"

namespace physx
{
bool PxcGetMaterialShapeShape			(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShapeMesh			(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShapeHeightField		(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShape				(SINGLE_MATERIAL_METHOD_ARGS);
bool PxcGetMaterialMesh					(SINGLE_MATERIAL_METHOD_ARGS);
bool PxcGetMaterialHeightField			(SINGLE_MATERIAL_METHOD_ARGS);


PxcGetSingleMaterialMethod g_GetSingleMaterialMethodTable[7] = 
{
	PxcGetMaterialShape,			//PxGeometryType::eSPHERE
	PxcGetMaterialShape,			//PxGeometryType::ePLANE
	PxcGetMaterialShape,			//PxGeometryType::eCAPSULE
	PxcGetMaterialShape,			//PxGeometryType::eBOX
	PxcGetMaterialShape,			//PxGeometryType::eCONVEXMESH
	PxcGetMaterialMesh,				//PxGeometryType::eTRIANGLEMESH	//not used: mesh always uses swept method for midphase.
	PxcGetMaterialHeightField,		//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this
		
};

//Table of contact methods for different shape-type combinations
PxcGetMaterialMethod g_GetMaterialMethodTable[][7] = 
{
	
	//PxGeometryType::eSPHERE
	{
		PxcGetMaterialShapeShape,			//PxGeometryType::eSPHERE
		PxcGetMaterialShapeShape,			//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,			//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,			//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,			//PxGeometryType::eTRIANGLEMESH	//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this
		
	},

	//PxGeometryType::ePLANE
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,		//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,		//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,		//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eCAPSULE
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,		//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,		//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,		//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,		//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eBOX
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,				//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,				//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eCONVEXMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,			//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eTRIANGLEMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eHEIGHTFIELD
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},
		
};

}
