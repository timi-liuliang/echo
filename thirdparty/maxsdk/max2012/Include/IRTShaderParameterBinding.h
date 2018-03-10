/**********************************************************************
*<
FILE:			IRTShaderParameterBinding.h

DESCRIPTION:	Interface to allow Render Nodes to set parameter values to the underlying graphics
				subsystem

CREATED BY:		Neil Hazzard

HISTORY:		created March 8th, 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/
#pragma once

#include "baseinterface.h"
#include "strbasic.h"

#define IRTSHADERPARAMETERBINDING_INTERFACE_ID Interface_ID(0x543218fa, 0x7a353bd1)

//!\brief Interface for setting parameters in a compiled effect file.
/*! A IRTShaderNode would implement this new interface to allow setting of parameters to the underlying
graphics subsystem.  When a shade tree is created a Microsoft Effect file is generated that contains all 
the parameters exposed during compiling of the tree.  At runtime the subsystem needs to set values to these
parameters.  At this level it knows nothing about the underlying material structure, and simply has a bunch
parameter names and data types.  To aid this, at render time the hardware renderer will query the host for the 
IRTShaderParameterBinding interface, and will call BindParameter to have the data assigned to the effect frame
work.  The "value" passed to the method is the correct size for the underlying data type, so the BindParameter
method must understand the data request.  In general this would either be a float, or a D3DXVECTOR (or D3DCOLORVALUE)
type for colors.  This value is assigned directly to the effect framework.
An example of setting a value can be seen below - taken from the Architectural and Design Material

\code
if(_tcscmp(paramName,_M("mia_material_1_diffuse"))==0)
{
	AColor val;
	val = m_mainPB->GetAColor(kPar_Diffuse, t);
	*(D3DCOLORVALUE*)value = GetD3DColor(val);
}
\endcode
This checks the name of the parameter, accesses the correct data item from the paramblock, and sets the
value correctly.
*/
class IRTShaderParameterBinding: public BaseInterface
{
public:
	virtual Interface_ID GetID() { return IRTSHADERPARAMETERBINDING_INTERFACE_ID; }
	/*! Assign the correct value based on the name parsed from the effect file.  This method
	will be called for every parameter parsed from the effect file that the renderer doesn't understand
	\param[in] paramName The name of the parameter as parsed from the effect file
	\param[out]value The pointer to a data structure that will receive the value to set
	*/
	virtual void BindParameter(const MCHAR * paramName, LPVOID value) =0 ;

	/*! Assign the correct value based on the name parsed from the effect file.  This method
	will be called for every parameter parsed from the effect file that the renderer doesn't understand
	\param[in] t Current time value
	\param[in] paramName The name of the parameter as parsed from the effect file
	\param[out]value The pointer to a data structure that will receive the value to set
	*/
	virtual void BindParameter(const TimeValue t, const MCHAR * paramName, LPVOID value) =0 ;

	/*! Called once before the parameter list is parsed.  Allows for one time setup of data.  This 
	could be used to make sure all parameters are valid before being queried
	*/
	virtual void InitializeBinding() = 0 ;
};
