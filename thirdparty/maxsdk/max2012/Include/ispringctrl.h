/**********************************************************************
 *<
	FILE: jiggleAPI.h

	DESCRIPTION:	Public header file for Spring controller

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "springsys.h"
#include "ifnpub.h"


#define JIGGLEPOS 0x79697d2a
#define JIGGLEP3 0x13892172

#define JIGGLE_POS_CLASS_ID	Class_ID(JIGGLEPOS, 0xf2b8a1c8)
#define JIGGLE_P3_CLASS_ID	Class_ID(JIGGLEP3, 0x68976279)

#define JIGGLE_CONTROL_REF  0
#define JIGGLE_PBLOCK_REF1	1
#define JIGGLE_PBLOCK_REF2  2

//parameter defaults
#define JIGGLE_DEFAULT_TENSION 2.0f
#define JIGGLE_DEFAULT_DAMPENING 0.5f
#define JIGGLE_DEFAULT_MASS 300.0f
#define JIGGLE_DEFAULT_DRAG 1.0f

#define SET_PARAMS_RELATIVE  0
#define SET_PARAMS_ABSOLUTE  1

#define SPRING_CONTROLLER_INTERFACE Interface_ID(0x4c212b2e, 0x680828ab)

/*! \sa  Class FPMixinInterface,  Class SpringSys , Class INode\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the Spring Controller and comes in two
different flavors, the <b>JIGGLE_POS_CLASS_ID</b> for the position spring
controller and <b>JIGGLE_P3_CLASS_ID</b> for the Point3 spring controller.\n\n
All methods of this class are Implemented by the System.\n\n

\par Data Members:
<b>SpringSys* partsys;</b>\n\n
A pointer to the spring system class.  */
class IJiggle : public FPMixinInterface
{
	public:
		enum { get_mass, set_mass, get_drag, set_drag, get_tension, set_tension, get_dampening, set_dampening,
			   add_spring, get_spring_count, remove_spring_by_index, remove_spring, get_spring_system, };

		BEGIN_FUNCTION_MAP
			FN_0	(get_mass, TYPE_FLOAT, GetMass);
			VFN_1	(set_mass, SetMass, TYPE_FLOAT);
			FN_0	(get_drag, TYPE_FLOAT, GetDrag);
			VFN_1	(set_drag, SetDrag, TYPE_FLOAT);
			FN_1	(get_tension, TYPE_FLOAT, GetTension, TYPE_INDEX);
			VFN_2	(set_tension, SetTension, TYPE_INDEX, TYPE_FLOAT);
			FN_1	(get_dampening, TYPE_FLOAT, GetDampening, TYPE_INDEX);
			VFN_2	(set_dampening, SetDampening, TYPE_INDEX, TYPE_FLOAT);

			FN_1	(add_spring, TYPE_BOOL, AddSpring, TYPE_INODE);
			FN_0	(get_spring_count, TYPE_INT, GetSpringCount);
			VFN_1	(remove_spring_by_index, RemoveSpring, TYPE_INDEX);
			VFN_1	(remove_spring, RemoveSpring, TYPE_INODE);
			//FN_0	(get_spring_system, TYPE_INTERFACE, GetSpringSystem);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		
		SpringSys* partsys;
		/*! \remarks This method returns the pointer to the associated spring
		system object. */
		virtual SpringSys* GetSpringSystem()=0;

		/*! \remarks This method returns the mass value. */
		virtual float	GetMass()=0;
		/*! \remarks This method allows you to set the mass value.
		\par Parameters:
		<b>float mass</b>\n\n
		The mass you wish to set.\n\n
		<b>bool update=true</b>\n\n
		This flag initiates an update if set to TRUE. */
		virtual void	SetMass(float mass, bool update=true)=0;
		/*! \remarks This method returns the drag value. */
		virtual float	GetDrag()=0;
		/*! \remarks This method allows you to set the drag value.
		\par Parameters:
		<b>float drag</b>\n\n
		The drag you wish to set.\n\n
		<b>bool update=true</b>\n\n
		This flag initiates an update if set to TRUE. */
		virtual void	SetDrag(float drag, bool update=true)=0;
		/*! \remarks This method returns the tension for the specified spring
		in the system.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the spring in the system. */
		virtual float	GetTension(int index)=0;
		/*! \remarks This method allows you to set the tension for the
		specified spring in the system.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the spring in the system.\n\n
		<b>float tension</b>\n\n
		The tension you wish to set.\n\n
		<b>int absolute=1</b>\n\n
		The flag defining whether the tension is absolute or relative.\n\n
		<b>bool update=true</b>\n\n
		This flag initiates an update if set to TRUE. */
		virtual void	SetTension(int index, float tension, int absolute=1, bool update=true)=0;
		/*! \remarks This method returns the dampening value for the specified
		spring in the system.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the spring in the system. */
		virtual float	GetDampening(int index)=0;
		/*! \remarks This method allows you to set the dampening value for the
		specified spring in the system.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the spring in the system.\n\n
		<b>float dampening</b>\n\n
		The tension you wish to set.\n\n
		<b>int absolute=1</b>\n\n
		The flag defining whether the tension is absolute or relative.\n\n
		<b>bool update=true</b>\n\n
		This flag initiates an update if set to TRUE. */
		virtual void	SetDampening(int index, float dampening, int absolute=1, bool update=true)=0;

		/*! \remarks This method allows you to add a spring.
		\par Parameters:
		<b>INode *node</b>\n\n
		A pointer to the node.
		\return  TRUE if the spring was added, otherwise FALSE. */
		virtual BOOL	AddSpring(INode *node)=0;
		/*! \remarks This method returns the current number of springs in the
		system. */
		virtual INT		GetSpringCount()=0;
		/*! \remarks This method allows you to remove a spring from the
		system.
		\par Parameters:
		<b>int which</b>\n\n
		The index of the spring in the system to remove. */
		virtual void	RemoveSpring(int which)=0;
		/*! \remarks This method allows you to remove a spring from the
		system.
		\par Parameters:
		<b>INode *node</b>\n\n
		A pointer to the node you wish to remove. */
		virtual void	RemoveSpring(INode *node)=0;
};

