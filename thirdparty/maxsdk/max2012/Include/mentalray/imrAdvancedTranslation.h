/*==============================================================================

  file:     imrAdvancedTranslation.h

  author:   Daniel Levesque

  created:  28 April 2004

  description:

    Defition of the mental ray advanced shader translation interface.

  modified:	


(c) 2003 Autodesk
==============================================================================*/
#pragma once

#include "imrTranslation.h"
#include "..\maxheap.h"

//==============================================================================
// class imrAdvancedTranslation
//
// A reference to this class is passed to imrShaderTranslation::TranslateParameters_Advanced().
//
// All methods in the class are an _exact_ match with a function in the mental ray API.
// They need to be used in _exactly_ the same way as the mental ray API equivalent.
// (For example: all the strings passed to the mental ray API need to be allocated using mi_mem_strdup()).
//
// For documentation on each method, see the MENTAL RAY MANUAL.
//==============================================================================
class imrAdvancedTranslation : public imrTranslation {

public:
	enum miParam_type
	{
		miTYPE_BOOLEAN = 0,			/* simple types: used for */
		miTYPE_INTEGER, 			/* returns and parameters */
		miTYPE_SCALAR,
		miTYPE_STRING,
		miTYPE_COLOR,
		miTYPE_VECTOR,
		miTYPE_TRANSFORM,
		miTYPE_SHADER,				/* complex types: used for */
		miTYPE_SCALAR_TEX,			/* parameters only */
		miTYPE_COLOR_TEX,
		miTYPE_VECTOR_TEX,
		miTYPE_LIGHT,
		miTYPE_STRUCT,
		miTYPE_ARRAY,
		miTYPE_TEX,
		miTYPE_MATERIAL,			/* phenomenon types */
		miTYPE_GEOMETRY,
		miTYPE_LIGHTPROFILE,			/* light profiles as args */
		miTYPE_DATA,				/* free-form user data */
		miNTYPES
	};

	enum miBoolean
	{
		miFALSE = 0,
		miTRUE  = 1
	};

	virtual miBoolean mi_api_parameter_name		(char *) = 0;
	virtual miBoolean mi_api_parameter_value	(miParam_type, void *, int *, int *) = 0;
	virtual miBoolean mi_api_parameter_push		(miBoolean) = 0;
	virtual miBoolean mi_api_parameter_pop		(void) = 0;
	virtual miBoolean mi_api_new_array_element	(void) = 0;

	virtual char* _mi_mem_strdup(const char*) = 0;
	virtual void _mi_mem_release(void*) = 0;
	virtual void* _mi_mem_allocate(int) = 0;
};
