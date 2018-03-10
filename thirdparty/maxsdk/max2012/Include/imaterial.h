/**********************************************************************
 *<
	FILE: IMaterial.h

	DESCRIPTION: Material extension Interface class

	CREATED BY: Nikolai Sander

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "baseinterface.h"
// forward declarations
class MtlBase;
class INode;

#define IMATERIAL_INTERFACE_ID Interface_ID(0x578773a5, 0x44cc0d7d)

/*! \sa  Class MtlBase, Class INode , Class BaseInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the material extension interface with the interface ID
<b>IMATERIAL_INTERFACE_ID</b>.  */
class IMaterial : public BaseInterface
{
public:
	// We'll do this as soon as Animatable derives from GenericInterfaceServer<BaseInterface>
	/*! \remarks This method allows you to set the material base.
	\par Parameters:
	<b>MtlBase *mtl</b>\n\n
	A pointer to the material base. */
	virtual void SetMtl(MtlBase *mtl)=0;
	/*! \remarks This method returns a pointer to the material base. */
	virtual MtlBase *GetMtl()=0;

	/*! \remarks This method allows you to set the node to associate with the
	material interface.
	\par Parameters:
	<b>INode *node</b>\n\n
	A pointer to the node you wish to set. */
	virtual void SetINode(INode *node)=0;
	/*! \remarks This method returns a pointer to the node associated with the
	material interface. */
	virtual INode *GetINode()=0;
};

