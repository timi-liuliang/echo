/**********************************************************************
 *<
	FILE: IEmissionColor.h

	DESCRIPTION: Emission Color Extension Interface

	CREATED BY: Norbert Jeske

	HISTORY: Created 08/22/01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "baseinterface.h"
#include "point3.h"

#define EMISSION_COLOR_INTERFACE_ID Interface_ID(0x4f803aa8, 0x71611798)

/*! \par Description:
<b>This class is only available in release 5 or later.</b>\n\n
Function-published class: Material emission color, for self-illuminates meshes
in viewport  */
class IEmissionColor : public BaseInterface
{
public:
	/*! \remarks Returns the Function Published interface ID for this
	interface.\n\n
	It is #<b>define EMISSION_COLOR_INTERFACE_ID Interface_ID(0x4f803aa8,
	0x71611798)</b> */
	virtual Interface_ID	GetID() { return EMISSION_COLOR_INTERFACE_ID; }

	// Interface Lifetime
	/*! \remarks none */
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	/*! \remarks Sets the emission color as Point3. */
	virtual void SetColor(Point3 color)=0;
	/*! \remarks K Prototype\n\n
	Returns the emission color as Point3. */
	virtual Point3 GetColor()=0;

	/*! \remarks K Prototype\n\n
	Sets the alpha color value for the emission in the range of 0.f to 1.f. */
	virtual void SetAlpha(float alpha)=0;
	/*! \remarks K Prototype\n\n
	Gets the alpha color value for the emission in the range of 0.f to 1.f. */
	virtual float GetAlpha()=0;
};

