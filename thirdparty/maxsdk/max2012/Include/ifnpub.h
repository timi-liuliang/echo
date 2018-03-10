//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        iFnPub.h
// DESCRIPTION: Interface to Plugin Function Publishing system
// AUTHOR:      John Wainwright
// HISTORY:     created 2/15/00 
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "baseInterface.h"
#include "strbasic.h"
#include "iparamb2Typedefs.h"
#include "coreexp.h" // for corexport
#include "strclass.h"

// forward declarations
class ISave;
class ILoad;
class AColor;
class Point4;

// classes defined in this header
class FPInterface;
	class FPInterfaceDesc;
		class FPStaticInterface;
	class FPMixinInterface;

class FPFunctionDef;
class FPActionDef;
class FPParamDef;
class FPPropDef;
class FPParams;
class FPParamOptions;
class FPValidator;
class FPEnum;
class FPValue;
class IObject;
class MAXException;
class FPMacroEmitter;

typedef int   FPStatus;
//! \brief ID for individual functions within an interface (local)
typedef short FunctionID;
//! \brief Special ID indicating no function bound
#define FP_NO_FUNCTION ((FunctionID)-1)
typedef short EnumID;
#define FP_NO_ENUM ((EnumID)-1)

// built-in interface IDs
#define NULLINTERFACE_ID	Interface_ID(0, 0)  // null interface
// BASEINTERFACE_ID (1,1) in baseInterface.h
#define FPINTERFACE_ID		Interface_ID(0, 2)  // root FPInterface
#define FPMIXININTERFACE_ID Interface_ID(0, 3)  // root FPMixinInterface
#define FPINTERFACEDESC_ID	Interface_ID(0, 4)  // root FPInterfaceDesc

// standard interface accessors
inline FPInterface*		GetFPInterface(BaseInterface* bi)     { return reinterpret_cast<FPInterface*>(     bi->GetInterface(FPINTERFACE_ID));      }
inline FPInterfaceDesc*  GetFPInterfaceDesc(BaseInterface* bi) { return reinterpret_cast<FPInterfaceDesc*>( bi->GetInterface(FPINTERFACEDESC_ID));  }
inline FPMixinInterface* GetFPMixinInterface(BaseInterface* bi){ return reinterpret_cast<FPMixinInterface*>(bi->GetInterface(FPMIXININTERFACE_ID)); }

// external classes
class ClassDesc;
class MSPluginClass;
class Rollout;
class MaxIcon;
class ActionTable;
class Point3;
class PBBitmap;
class Mtl;
class Texmap;
class INode;
class ReferenceTarget;
class Matrix3;
class AngAxis;
class Quat;
class Ray;
class Point2;
class BitArray;
class ClassDesc;
class Mesh;
class Object;
class Control;
class Interval;
class Color;
class Value;

/*! \defgroup fpStatusValues FPStatus values
One of the following values are returned from the various FPInterface methods.
\sa Class FPInterface, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a> */
//@{
#define FPS_FAIL				0 //!< Indicates a failure.
#define FPS_NO_SUCH_FUNCTION	-1 //!< The function called does not exist.
#define FPS_ACTION_DISABLED		-2 //!< The action is disabled.
#define FPS_OK					1 //!< Indicates a success.
//@}

// FPInterface class, the base class for FnPub interfaces.  
//   contains the basic dispatching code & virtual base methods
//	 The prime subclass is FPInterfaceDesc, which contains the 
//	 interface metadata in a singleton instance.  There are two
//   typedefs, FPStaticInterface used as the base class for static and core 
//   interfaces, and FPMixinInterface used as the
//   base class for mixin (object-based) interfaces

/*! \sa  Class BaseInterface, Class ClassDesc, Class FPInterfaceDesc, Class Interface_ID, Class ActionTable,
\ref fpStatusValues, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The Function Publishing system makes use of this class. Functions are published
in one or more Interfaces by a plug-in. Each interface is represented by an
instance of a class derived from this base class.\n\n
Note that the Function Publishing class hierarchy is as follows:\n\n
<b>FPInterface</b>: This is the base class for all interfaces, the prime client
type for using interfaces.\n\n
<b>FPInterfaceDesc</b>: This is the class which contains interface
metadata.\n\n
<b>FPStaticInterface</b>: This is the class to use as the base class for
defining static or core virtual interface classes.\n\n
<b>FPMixinInterface</b>: This is for use as the base class for defining
object-based mixin interface classes, in this case you also use FPInterfaceDesc
for mixin interface descriptors.  */
class FPInterface : public BaseInterface
{
protected:
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual FPStatus _dispatch_fn(FunctionID fid, TimeValue t, FPValue& result, FPParams* p) { return FPS_NO_SUCH_FUNCTION; }
	#pragma warning(pop)
public:
	static CoreExport FPInterfaceDesc nullInterface;			//  null interface descriptor, can be returned by interfaces not publishing metadata

	// from BaseInterface
	virtual BaseInterface*			GetInterface(Interface_ID id) 
									{ 
										if (id == FPINTERFACE_ID) return this; 
										else if (id == FPINTERFACEDESC_ID) return (BaseInterface*)GetDesc();
										else return BaseInterface::GetInterface(id); 
									}
	
	// metadata access
	/*! \remarks Returns a pointer to the class which contains the interface
	metadata. */
	virtual FPInterfaceDesc*		GetDesc() = 0;

	// Fn calling
	/*! \remarks 
	\par Parameters:
	<b>FunctionID fid</b>\n\n
	The function ID of the function to invoke.\n\n
	<b>TimeValue t=0</b>\n\n
	The timevalue at which to invoke the function.\n\n
	<b>FPParams* params=NULL</b>\n\n
	The FPParams to pass.
	\return  The FPStatus. See \ref fpStatusValues for details */
	CoreExport virtual FPStatus		Invoke(FunctionID fid, TimeValue t=0, FPParams* params=NULL);
	/*! \remarks This method will invoke the specified function.
	\par Parameters:
	<b>FunctionID fid</b>\n\n
	The function ID of the function to invoke.\n\n
	<b>FPParams* params</b>\n\n
	The FPParams to pass.
	\return  The FPStatus. See \ref fpStatusValues for details */
	virtual inline FPStatus			Invoke(FunctionID fid, FPParams* params) { return Invoke(fid, 0, params); }
	/*! \remarks This method will invoke the specified function.
	\par Parameters:
	<b>FunctionID fid</b>\n\n
	The function ID of the function to invoke.\n\n
	<b>TimeValue t</b>\n\n
	The timevalue at which to invoke the function.\n\n
	<b>FPValue\& result</b>\n\n
	A reference to the resulting FPValue.\n\n
	<b>FPParams* params=NULL</b>\n\n
	The FPParams to pass.
	\return  The FPStatus. See \ref fpStatusValues for details */
	CoreExport virtual FPStatus		Invoke(FunctionID fid, TimeValue t, FPValue& result, FPParams* params=NULL);
	/*! \remarks This method will invoke the specified function.
	\par Parameters:
	<b>FunctionID fid</b>\n\n
	The function ID of the function to invoke.\n\n
	<b>FPValue\& result</b>\n\n
	A reference to the resulting FPValue.\n\n
	<b>FPParams* params=NULL</b>\n\n
	The FPParams to pass.
	\return  The FPStatus. See \ref fpStatusValues for details */
	virtual inline FPStatus			Invoke(FunctionID fid, FPValue& result, FPParams* params=NULL) { return Invoke(fid, 0, result, params); }
	/*! \remarks This method returns a function ID based on the name of the
	function specified.
	\par Parameters:
	<b>MCHAR* name</b>\n\n
	The name of the function to retrieve the Function ID for. */
	CoreExport virtual FunctionID	FindFn(MCHAR* name);

	// predicate access
	/*! \remarks This method allows you to check whether a specific action
	function is enabled, in which case the method will return TRUE. If the
	action function is not enabled FALSE will be returned.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to check the enabled state for. */
	CoreExport virtual BOOL			IsEnabled(FunctionID actionID);
	/*! \remarks This method allows you to check whether a specific action
	function is checked, in which case the method will return TRUE. If the
	action function is not checked FALSE will be returned.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to check the checked state for. */
	CoreExport virtual BOOL			IsChecked(FunctionID actionID);
	/*! \remarks This method allows you to check whether a specific action
	function is visible, in which case the method will return TRUE. If the
	action function is not visible FALSE will be returned.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to check the visibility state for.
	*/
	CoreExport virtual BOOL			IsVisible(FunctionID actionID);
	/*! \remarks This method will return the isEnabled ID for the specified
	action function.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to get the isEnabled ID for. */
	CoreExport virtual FunctionID	GetIsEnabled(FunctionID actionID);
	/*! \remarks This method will return the isChecked ID for the specified
	action function.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to get the isChecked ID for. */
	CoreExport virtual FunctionID	GetIsChecked(FunctionID actionID);
	/*! \remarks This method will return the isVisible ID for the specified
	action function.
	\par Parameters:
	<b>FunctionID actionID</b>\n\n
	The function ID of the action you wish to get the isVisible ID for. */
	CoreExport virtual FunctionID	GetIsVisible(FunctionID actionID);

	// Action interface access & control
	/*! \remarks This method returns a pointer to the ActionTable.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual ActionTable*			GetActionTable() { return NULL; }
	// global Actions enable/disable
	/*! \remarks This method allows you to enable or disable the entire set of
	actions in the interface You might want to use this method if the actions
	are only to be active during certain periods in the running of 3ds Max.
	Usually, this control is achieved via ActionTable contexts.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE to enable actions, FALSE to disable them.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void					EnableActions(BOOL onOff) { UNUSED_PARAM(onOff); }
};

#include "paramtype.h"

// FPInterfaceDesc class, an FPInterface that contains the metadata for an interface, a distinhished
//						  singleton instance per interface kept in a Tab<> in ClassDesc
//						  This class is subclassed typically by static and core inerfaces and instantiated
//						  by mixins to provide their metadata
/*! \sa  Class FPInterface,  Class ClassDesc,  Class Interface_ID, Class FPEnum, Class FPFunctionDef, Class FPPropDef, Class ActionTable, Template Class Tab, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the Function Publishing interface descriptor. This is usually a static
instance of the implementation interface. The constructor for this class uses
the same var-args technique used by the ParamBlockDesc2 constructor, enabling
descriptive information for all the functions in the interface to be supplied
in one constructor call.\n\n
The FPInterfaceDesc class, an FPInterface that contains the metadata for an
interface, is a distinguished  singleton instance per interface kept in a
Tab\<\> in ClassDesc. This class is subclassed typically by static and core
interfaces and instantiated by mixins to provide their metadata.\n\n
Note the following typedef: <b>typedef FPInterfaceDesc FPStaticInterface</b>
\par Data Members:
<b>Interface_ID ID;</b>\n\n
The unique ID of the interface.\n\n
<b>MSTR internal_name;</b>\n\n
The fixed internal name for the interface.\n\n
<b>StringResID description;</b>\n\n
The description string resource ID.\n\n
<b>ClassDesc* cd;</b>\n\n
Points to the publishing plug-in's ClassDesc.\n\n
<b>USHORT flags;</b>\n\n
The flag bits. One or more of the following values:\n\n
<b>FP_ACTIONS</b>\n\n
Marks this as an Action Interface, holding only UI modal, zero parameter action
functions.\n\n
<b>FP_MIXIN</b>\n\n
Marks this as a Mixin Interface, it is implemented directly by the plug-in
class, so the methods in it are virtual on the plugin's objects.\n\n
<b>FP_CORE</b>\n\n
Marks this as a 3ds Max Core Interface, available through
<b>GetCOREInterface(Interface_ID)</b>.\n\n
<b>FP_STATIC_METHODS</b>\n\n
This interface is used as a static method interface in MAXScript, properties
are not directly callable\n\n
<b>FP_SCRIPTED_CLASS</b>\n\n
Internal use only: Belongs to a scripted plug-in class.\n\n
<b>FP_TEMPORARY</b>\n\n
Internal use only: Temporary descriptor built during scene load.\n\n
<b>Tab\<FPFunctionDef*\> functions;</b>\n\n
A table of descriptors for individual functions in this interface.\n\n
<b>Tab\<FPPropDef*\> props;</b>\n\n
A Table of descriptors for individual properties in this interface.\n\n
<b>Tab\<FPEnum*\> enumerations;</b>\n\n
A table of any symbolic enums for the interface. This is a Tab\<\> of pointers
to <b>FPEnum</b> class instances which themselves contain a Tab\<\> of name,
code pairs.\n\n
The following data members are for scripted plug-ins if this interface belongs
to a scripted plug-in class.\n\n
<b>MSPluginClass* pc;</b>\n\n
The scripted class if non-NULL (gc-protected by the scripted plugin class).\n\n
<b>Rollout* rollout;</b>\n\n
The rollout if specified (gc-protected by the scripted plugin class).\n\n
<b>ActionTable* action_table;</b>\n\n
The table published for this action interface.\n\n
For more information, see Class ActionTable.  */
class FPInterfaceDesc : public FPInterface 
{
protected:
	CoreExport void	load_descriptor(Interface_ID id, MCHAR* int_name, StringResID descr, ClassDesc* cd, USHORT flag, va_list ap);

public:
	// interface metadata 
	Interface_ID	ID;				// my unique ID
	MSTR			internal_name;	// fixed, internal name
	StringResID		description;	// description string resource
	ClassDesc*		cd;				// publishing plugin's ClassDesc
	USHORT			flags;			// flag bits
	Tab<FPFunctionDef*> functions;	// descriptors for individual functions in this interface
	Tab<FPPropDef*> props;			//    "         "      "      properties in this interface
	Tab<FPEnum*>	enumerations;	// any symbolic enums for the interface
	// scripted plug-in stuff if this belongs to a scripted plug-in class
	MSPluginClass*	pc;				// the scripted class if non-NULL (gc-protected by the scripted plugin class)
	Rollout*		rollout;		// rollout if specified (gc-protected by the scripted plugin class)
	// Action-interface stuff
	ActionTable*	action_table;	// the table published for this action interface

	// constructors
	/*! \remarks Constructor. No initialization is performed. */
	CoreExport FPInterfaceDesc() { }
	/*! \remarks Constructor.
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The unique ID of the interface.\n\n
	<b>MCHAR* int_name</b>\n\n
	The fixed internal name for the interface.\n\n
	<b>StringResID descr</b>\n\n
	A string resource ID containing the description for this interface
	class.\n\n
	<b>ClassDesc* cd</b>\n\n
	A pointer to a ClassDesc class descriptor of the publishing plug-in.\n\n
	<b>ULONG flag</b>\n\n
	The flag bits. One or more of the following values: <b>FP_ACTIONS,
	FP_MIXIN, FP_CORE, FP_STATIC_METHODS, FP_SCRIPTED_CLASS, FP_TEMPORARY</b>.
	For a description see the data members descriptions.\n\n
	<b>...</b>\n\n
	This constructor takes a variable number of arguments representing the
	'properties' sections and function definitions. For more information see
	the advanced topics on
	<a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function
	Publishing</a>. */
	CoreExport FPInterfaceDesc(Interface_ID id, MCHAR* int_name, StringResID descr, ClassDesc* cd, ULONG flag, ...);

	CoreExport ~FPInterfaceDesc();
	/*! \remarks This is a virtual method called by the varargs-based
	constructors for interface descriptors and static interfaces, so that they
	have an opportunity to do runtime initialization of any extra state data
	you add to these interfaces (usually to static interfaces). Since such
	interfaces are usually constructed with the built-in varargs constructor,
	there is no way to do custom initialization without a hook like the Init()
	call. Your static interface would provide an implementation of Init() to do
	any special initialization.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void Init() { }

	// from GenericInterface, default lifetime is noRelease
	LifetimeType			LifetimeControl() { return noRelease; }

	// from BaseInterface
	virtual BaseInterface*	GetInterface(Interface_ID id) { if (id == FPINTERFACEDESC_ID) return this; else return FPInterface::GetInterface(id); }
	
	// construction utilities
	/*! \remarks This method relates to <b>Init()</b>. In some cases, you
	really do need to provide your own constructor or set of constructors for a
	static interface or descriptor, but you still want to be able to load it
	with all the interface metadata that the built-in varargs constructor does.
	You can do this by calling the <b>LoadDescriptor()</b> method at any point
	in your own constructors and it takes the same arguments as the built-in
	varargs constructor.
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The unique ID of the interface.\n\n
	<b>MCHAR* int_name</b>\n\n
	The fixed internal name for the interface.\n\n
	<b>StringResID descr</b>\n\n
	A string resource ID containing the description for this interface
	class.\n\n
	<b>ClassDesc* cd</b>\n\n
	A pointer to a ClassDesc class descriptor of the publishing plug-in.\n\n
	<b>ULONG flag</b>\n\n
	The flag bits. One or more of the following values: <b>FP_ACTIONS,
	FP_MIXIN, FP_CORE, FP_STATIC_METHODS, FP_SCRIPTED_CLASS, FP_TEMPORARY</b>.
	For a description see the data members descriptions.\n\n
	<b>...</b>\n\n
	This method takes a variable number of arguments representing the
	'properties' sections and function definitions. For more information see
	the advanced topics on
	<a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function
	Publishing</a>. */
	CoreExport void			LoadDescriptor(Interface_ID id, MCHAR* int_name, StringResID descr, ClassDesc* cd, ULONG flag, ...);

	/** \brief Add a new function to this interface descriptor.
	 *
	 * Uses the same parameter syntax as LoadDescriptor and the Constructor.
	 * Note that it is mandatory to mark the end of the variable arguments list
	 * with <code>end</code>.  Failure to do so will result in undefined 
	 * behaviour.
	 */
	CoreExport void AppendFunction(int id, ...);


	/** \brief Add a new property to this interface descriptor.
	 *
	 * Uses the same parameter syntax as LoadDescriptor and the Constructor.
	 * Note that it is mandatory to mark the end of the variable arguments list
	 * with <code>end</code>.  Failure to do so will result in undefined 
	 * behaviour.
	 */
	CoreExport void AppendProperty(int id, ...);
	
	/** \brief Add a new enum to this interface descriptor.
	 *
	 * Uses the same parameter syntax as LoadDescriptor and the Constructor.
	 * Note that it is mandatory to mark the end of the variable arguments list
	 * with <code>end</code>.  Failure to do so will result in undefined 
	 * behaviour.
	 */
	CoreExport void AppendEnum(int id, ...);


	/*! \remarks This method sets the ClassDesc pointer associated
	FPInterfaceDesc class. You can only call this method once on a descriptor
	and then only if it has been constructed initially with a NULL cd. See the
	notes in the constructor.\n\n

	\par Parameters:
	<b>ClassDesc* i_cd</b>\n\n
	This points to the ClassDesc class descriptor to set. */
	CoreExport void			SetClassDesc(ClassDesc* i_cd);
	/*! \remarks This is used internally. */
	CoreExport va_list		check_fn(va_list ap, int id);
	/*! \remarks This is used internally. */
	CoreExport va_list		scan_fn(va_list ap, int id, int index);
	/*! \remarks This is used internally. */
	CoreExport va_list		check_prop(va_list ap, int id);
	/*! \remarks This is used internally. */
	CoreExport va_list		scan_prop(va_list ap, int id, int index);
	/*! \remarks This is used internally. */
	CoreExport va_list		check_enum(va_list ap, EnumID id);
	/*! \remarks This is used internally. */
	CoreExport va_list		scan_enum(va_list ap, EnumID id, int index);

	// metadata access
	/*! \remarks This method returns a pointer to the descriptor for this
	Function Publishing interface descriptor.
	\par Default Implementation:
	<b>{ return this; }</b> */
	FPInterfaceDesc*		GetDesc() { return this; }
	Interface_ID			GetID() { return ID; }
	/*! \remarks This method returns a pointer to the function definition of a
	specific function identified by its ID. Calls to this method, given an
	<b>FPInterface*</b>, can be made indirectly through
	<b>FPInterface::GetDesc()</b>. For example; <b>FPFunctionDef* fd =
	fpi-\>GetDesc()-\>GetFnDef(foo_move);</b>
	\par Parameters:
	<b>FunctionID fid</b>\n\n
	The unique function ID used to identify the function. */
	CoreExport FPFunctionDef* GetFnDef(FunctionID fid);
	/*! \remarks This method returns a pointer to the action table.
	\par Default Implementation:
	<b>{ return action_table; }</b> */
	ActionTable*			GetActionTable() { return action_table; }

	// global Actions enable/disable
	/*! \remarks This method allows you to enable or disable the entire set of
	actions in the interface You might want to use this method if the actions
	are only to be active during certain periods in the running of 3ds Max.
	Usually, this control is achieved via ActionTable contexts.
	\par Parameters:
	<b>BOOL onOff;</b>\n\n
	TRUE to enable actions, FALSE to disable actions. */
	CoreExport void			EnableActions(BOOL onOff);

	// overridable HInstance and resource access from owning module
	/*! \remarks This method will return a handle to the owning instance. */
	CoreExport virtual HINSTANCE HInstance();
	/*! \remarks This method returns the string associated with a specified
	String Resource ID
	\par Parameters:
	<b>StringResD id</b>\n\n
	The string resource ID for which you want to obtain the string. */
	CoreExport virtual MCHAR* GetRsrcString(StringResID id);

private:
	va_list AppendFunction(va_list ap, int id);
	va_list AppendProperty(va_list ap, int id);
	va_list AppendEnum(va_list ap, int id);
};

// FPInterfaceDesc flag bits
// per interface flags
#define FP_ACTIONS			0x0001	// marks this as an Action Interface, holding only UI modal, zero param action fns
#define FP_MIXIN			0x0002	// marks this as a Mixin Interface, it is implemented directly by the plugin class, so the 
									// methods in it are virtual on the plugin's objects
#define FP_CORE				0x0004	// marks this as a MAX Core Interface, available through GetCOREInterface(Interface_ID) 
#define FP_STATIC_METHODS	0x0008	// this interface is used as a static method interface in MAXScript, properties are not directly callable
#define FP_TEST_INTERFACE	0x0010	// a non-public test interface, Discreet internal use only 
	
// per descriptor internal flags
#define	FP_SCRIPTED_CLASS	0x1000	// belongs to a scripted plug-in class
#define	FP_TEMPORARY		0x2000	// temporary descripter built during scene load to support schema migration

// static interface
/*! \sa  Class FPInterfaceDesc.\n\n
\par Description:
This class is currently the same as <b>FPInterfaceDesc</b> as per the following
typedef:\n\n
<b>typedef FPInterfaceDesc FPStaticInterface</b>\n\n
See Class FPInterfaceDesc for
details. */
class FPStaticInterface : public FPInterfaceDesc
{
};

// The MAXScript FPMixinInterface wrapper class
class FPMixinInterfaceValue;

// mixin interface
/*! \sa  Class FPInterface, Class FPInterfaceDesc, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function publishing</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
A "Mixin" interface provides a way for a plug-in to expose some of its
functionality for use by other plug-ins or MAXScript. The notion of "Mixin"
refers to the idea that the interface is a sub-class of the plug-in class and
thus "mixed in" with it. Many classes in the SDK now inherit from
FPMixinInterface in order to expose some of their functionality.\n\n
Developers should see the documentation for
Class FPInterface for reference on this
class as well.\n\n
For an overview of the function publishing system as a whole, see
<a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function
publishing</a>.  */
class FPMixinInterface : public FPInterface
{
	friend class FPMixinInterfaceValue;
private:
	FPMixinInterfaceValue* MXS_fpi;
protected:
	Tab<InterfaceNotifyCallback*> *interfaceNotifyCBs;

	// copy constructor and copy assignment operator are defined to prevent MXS_fpi and 
	// interfaceNotifyCBs pointers from being copied from right-hand side. 
	// These pointers are instance specific.
	FPMixinInterface(const FPMixinInterface& rhs) : MXS_fpi(NULL), interfaceNotifyCBs(NULL) { UNUSED_PARAM(rhs); }
	FPMixinInterface& operator=(const FPMixinInterface& rhs) { UNUSED_PARAM(rhs); return *this;}

public:

	FPMixinInterface() : MXS_fpi(NULL), interfaceNotifyCBs(NULL) {}


	// from GenericInterface, default lifetime is serverControlled
	virtual LifetimeType	LifetimeControl() { return serverControlled; }
	
 	CoreExport virtual bool RegisterNotifyCallback(InterfaceNotifyCallback* incb);      
	
	virtual void UnRegisterNotifyCallback(InterfaceNotifyCallback* incb) 
	{ 
		if (interfaceNotifyCBs)
			for (int i=0; i < interfaceNotifyCBs->Count(); i++)
			{	if (incb == (*interfaceNotifyCBs)[i])
					interfaceNotifyCBs->Delete(i,1);
			}
	}
	
	CoreExport ~FPMixinInterface();

	// from BaseInterface
	virtual BaseInterface*	GetInterface(Interface_ID id) { if (id == FPMIXININTERFACE_ID) return this; else return FPInterface::GetInterface(id); }
	
	// utility metadata accessors...
	// use GetDescByID() to directly implement FPInterface::GetDesc() in your public virtual base mixin class, 
	//   like this:     FPInterfacedesc* GetDesc() { return GetDescByID(THIS_INTERFACE_ID); }
	//   then implement a GetDescByID() in the implementing class to avoid link export issues
	/*! \remarks This method is used to directly implement
	<b>FPInterface::GetDesc()</b> in your public virtual base mixin class, like
	this;\n\n
	<b>FPInterfaceDesc* GetDesc() { return GetDescByID(THIS_INTERFACE_ID);
	}</b>\n\n
	Then implement a GetDescByID() in the implementing class to avoid link
	export issues.
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The unique interface ID by which to get the FPInterfaceDesc.
	\par Default Implementation:
	<b>{ return \&nullInterface; }</b> */
	virtual FPInterfaceDesc* GetDescByID(Interface_ID id) { UNUSED_PARAM(id); return &nullInterface; }

	// override GetID() in those interfaces that don't publish metadata but have a unique Interface_ID for quick internal identification 
	//   in implementation code that might be shared by a bunch of mixin interfaces
	/*! \remarks This method overrides GetID() in those interfaces that do not
	publish metadata but instead have a unique Interface_ID for quick internal
	identification in implementation code that might be shared by a bunch of
	mixin interfaces.
	\return  The Interface_ID.
	\par Default Implementation:
	<b>{ return GetDesc()-\>ID; }</b> */
	virtual Interface_ID	 GetID() { return GetDesc()->ID; }
};

// FPFunctionDef, contains descriptor for each published function
//				  live in Tab<> in FPInterface
/*! \sa  Class FPInterface, Class FPInterfaceDesc, Class FPActionDef, Class FPParamDef, Template Class Tab, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>, 
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of Param Type Choices</a>, 
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of ParamType2 Choices</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class stores data about a single function of an FPInterface. A table of
pointers to these objects is a data member of Class <b>FPInterfaceDesc</b>.
\par Data Members:
<b>FunctionID ID;</b>\n\n
The interface-local ID (unique to the interface only) used to identify the
function in calls.\n\n
<b>MSTR internal_name;</b>\n\n
The fixed internal name for the function.\n\n
<b>StringResID description;</b>\n\n
The description string resource.\n\n
<b>USHORT flags;</b>\n\n
The internal flag bits. One of more of the following values:\n\n
<b>FP_ACTION</b>\n\n
Indicates this is an action function.\n\n
<b>FP_HAS_UI</b>\n\n
Indicates the action has UI specified.\n\n
<b>FP_ICONRES</b>\n\n
Indicates icon via resource ID.\n\n
<b>FP_ICONFILE</b>\n\n
Indicates icon via bmp file + index.\n\n
<b>FP_HAS_SHORTCUT</b>\n\n
Indicates has default keyboard shortcut.\n\n
<b>FP_HAS_KEYARGS</b>\n\n
Indicates function has some optional keyword args defined.\n\n
<b>FP_VAR_ARGS</b>\n\n
Indicates a variable number of args, pass args directly in a FPParams
instance.\n\n
<b>FP_NO_REDRAW</b>\n\n
Do not flag need for viewport redraw when function is invoked, MAXScript
defaults to flag redraw.\n\n
<b>ParamType2 result_type;</b>\n\n
The type of value returned by the function.\n\n
<b>EnumID enumID;</b>\n\n
The ID of symbolic enumeration in owning interface if any.\n\n
<b>FPActionDef* action_def;</b>\n\n
The extra metadata if function in an Action interface.\n\n
<b>Tab\<FPParamDef*\> params;</b>\n\n
The descriptors for parameters to this function.  */
class FPFunctionDef: public MaxHeapOperators
{
public:

	DWORD			cbStruct;			// size of the struct
	FunctionID		ID;				// interface-local ID, used to identify fn in calls
	MSTR			internal_name;	// fixed, internal name
	StringResID		description;	// description string resource
	USHORT			flags;			// flag bits
	ParamType2		result_type;	// type of value returned by fn
	EnumID			enumID;			// ID of symbolic enumeration in owning interface if any
	FPActionDef*	action_def;		// extra metadata if function in an Action interface
	Tab<FPParamDef*> params;		// descriptors for parameters to this fn 
	SHORT			keyparam_count; // count of keyword-parameters (with f_keyArgDefault's)

	/*! \remarks Constructor.  The data members are initialized as
	follows:\n\n
	<b>flags = 0; description = 0; action_def = NULL; enumID = FP_NO_ENUM;</b>
	*/
	FPFunctionDef() {cbStruct = sizeof(FPFunctionDef); flags = 0; description = 0; action_def = NULL; enumID = FP_NO_ENUM; keyparam_count = 0; }
	CoreExport ~FPFunctionDef();
};

/*! \sa  Class FPParamOptions, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of Param Type Choices</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The 'properties' section follows the function definitions. Each propery has a
single entry defining the function IDs for the getter and setter functions, a
fixed internal property name, a descriptor string resource ID and the property
type. If the property is read-only and there is no setter function, specify
<b>FP_NO_FUNCTION</b> for the setter ID.
\par Data Members:
<b>FunctionID getter_ID;</b>\n\n
The interface-local ID for getter method.\n\n
<b>FunctionID setter_ID;</b>\n\n
The interface-local ID for setter method.\n\n
<b>MSTR internal_name;</b>\n\n
The fixed, internal name.\n\n
<b>StringResID description;</b>\n\n
The description string resource ID.\n\n
<b>USHORT flags;</b>\n\n
The flag bits.\n\n
<b>ParamType2 prop_type;</b>\n\n
The property type.\n\n
<b>EnumID enumID;</b>\n\n
The ID of symbolic enumeration in owning interface if any.\n\n
<b>FPParamOptions* options;</b>\n\n
Present if non-NULL, used for setter param.  */
class FPPropDef: public MaxHeapOperators
{
public:
	DWORD			cbStruct;		// size of the struct
	FunctionID		getter_ID;		// interface-local ID for getter method
	FunctionID		setter_ID;		// interface-local ID for setter method
	MSTR			internal_name;	// fixed, internal name
	StringResID		description;	// description string resource
	USHORT			flags;			// flag bits
	ParamType2		prop_type;		// property type
	EnumID			enumID;			// ID of symbolic enumeration in owning interface if any
	FPParamDef*		setter_param;	// present if non-NULL, used for setter param options

	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b>flags = 0; description = 0; getter_ID = setter_ID =
	FPS_NO_SUCH_FUNCTION; enumID = FP_NO_ENUM; options = NULL;</b> */
	FPPropDef() { cbStruct = sizeof(FPPropDef); flags = 0; description = 0; getter_ID = setter_ID = FPS_NO_SUCH_FUNCTION; enumID = FP_NO_ENUM; setter_param = NULL; }
	CoreExport ~FPPropDef();
};

// function def flag bits	
#define FP_NO_REDRAW		  0x0001	// do not flag need for viewport redraw when function is invoked, MAXScript defaults to flag redraw
#define FP_VAR_ARGS			  0x0002	// variable number of args, pass args directly in a FPParams instance
#define FP_CLIENT_OWNS_RESULT 0x0004	// client owns lifetime of pointer-based results, should delete when finished using
// internal function bits
#define FP_ACTION			  0x0100	// indicates an action function
#define FP_HAS_UI			  0x0200	// action has UI specifiec
#define FP_ICONRES			  0x0400	// icon via res ID
#define FP_ICONFILE			  0x0800	// icon via bmp file + index
#define FP_HAS_SHORTCUT		  0x1000	// has default KB shortct
#define FP_HAS_KEYARGS		  0x2000	// fn has some optional keyword args defined

// FPActionDef,  contains extra descriptor info for function if fn is in an Action interface 

/*! \sa  Class FPInterface, Class FPInterfaceDesc, Class FPActionDef, Class FPParamDef, Template Class Tab, Class MaxIcon, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>, 
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_controltype2_choices.html">List of ControlType2 Choices</a>, 
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of ParamType2 Choices</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
FPActionDef, contains extra descriptor info for Action interface functions.
\par Data Members:
<b>MSTR internal_cat;</b>\n\n
The fixed, internal category name.\n\n
<b>StringResID category;</b>\n\n
The localizable category string resource ID.\n\n
<b>FunctionID isEnabled_id;</b>\n\n
The interface function IDs for the isEnabled predicate for this action.\n\n
<b>FunctionID isChecked_id;</b>\n\n
The interface function IDs for the isChecked predicate for this action.\n\n
<b>FunctionID isVisible_id;</b>\n\n
The interface function IDs for the isVisible predicate for this action.\n\n
<b>ResID icon_resID;</b>\n\n
The icon as resource ID.\n\n
<b>MSTR icon_file;</b>\n\n
The icon as UI .bmp filename, index pair, as per CUI icon specifications.\n\n
<b>short icon_index;</b>\n\n
The index of the icon associated with the action.\n\n
<b>MaxIcon* icon;</b>\n\n
The MaxIcon class data associated with this action.\n\n
<b>StringResID button_text;</b>\n\n
The button text string resource ID, defaults to function description.\n\n
<b>StringResID tool_tip;</b>\n\n
The tooltip string resource ID, defaults to function description.\n\n
<b>StringResID menu_text;</b>\n\n
The menu item text string resource ID, defaults to buttonText or function
description.\n\n
<b>ControlType2 ctrl_type;</b>\n\n
The type of UI control, if f_ui specified.\n\n
<b>ResID ctrl_pbID;</b>\n\n
The control's host parammap pblock ID.\n\n
<b>MapID ctrl_mapID;</b>\n\n
The control's host parammap map ID within the block.\n\n
<b>int ctrl_id;</b>\n\n
The control dialog item ID.\n\n
<b>COLORREF ctrl_hiCol;</b>\n\n
The highlight color if check button.\n\n
<b>ACCEL shortcut;</b>\n\n
The default keyboard shortcut.  */
class FPActionDef: public MaxHeapOperators
{
public:
	DWORD		cbStruct;			// size of the struct
	MSTR		internal_cat;		// fixed, internal category name
	StringResID category;			// localizable category resID
	FunctionID	isEnabled_id;		// interface function IDs for the isEnabled predicate for this action
	FunctionID	isChecked_id;		//   "  " for isChecked predicate
	FunctionID	isVisible_id;		//   "  " for isVisible predicate
	ResID		icon_resID;			// icon as resource ID
	MSTR		icon_file;			// icon as UI .bmp filename, index pair, as per CUI icon specifications...
	short		icon_index;	
	MaxIcon*	icon;
	StringResID	button_text;		// button text string resID, defaults to function description
	StringResID	tool_tip;			// tooltip string resID, defaults to function description
	StringResID	menu_text;			// menu item text string resID, defaults to buttonText or function description
	ControlType2 ctrl_type;			// type of UI control, if f_ui specified
	ResID		ctrl_pbID;			// control's host parammap pblock ID
	MapID		ctrl_mapID;			// control's host parammap map ID within the block
	int			ctrl_id;			// control dialog item ID
	COLORREF	ctrl_hiCol;			// highlight colorif check button
	ACCEL		shortcut;			// default keyboard shortcut
	FPMacroEmitter* macro_emitter;	// if non-NULL, callback object to emit macros to macroRecorder

	/*! \remarks Constructor.\n\n
	This will initialize the members to their empty default values. */
	CoreExport FPActionDef();
	/*! \remarks Destructor. */
	CoreExport ~FPActionDef();
};

// fn def option tags for Action functions & parameters
enum {
	// Action options
	f_category = -(1<<30),			// category name, as internal MCHAR* and localizable string resID, defaults to interface name
	f_predicates,					// supply 3 functionIDs for isEnabled, isChecked, isVisible predicates 
	f_isEnabled,					// isEnabled predicate functionID
	f_isChecked,					// isChecked predicate functionID
	f_isVisible,					// isVisible predicate functionID
	f_iconRes,						// icon as resource ID
	f_icon,							// icon as UI .bmp filename, index pair, as per CUI icon specifications
	f_buttonText,					// button text string resID, defaults to function description
	f_toolTip,						// tooltip string resID, defaults to function description
	f_menuText,						// menu item text string resID, defaults to buttonText or function description
	f_ui,							// UI spec if paramMap2-implemented UI (pmap blockID, mapID, control type, button or checkbutton resID, hilight col if chkbtn)
	f_shortCut,						// default keyboard short cut, as pair: virt ACCEL flags word, keycode  (first two items in Win32 ACCEL struct)
	f_macroEmitter,					// provide callback object to handle macro emmission
	// param options
	f_range,						// valid range, two type-specific vals
	f_validator,					// validator object, FPValidator*
	f_inOut,						// in, out flags FPP_IN_PARM, FPP_OUT_PARAM or both, defaults to both
	f_keyArgDefault,				// marks this as an optional keyArg param and gives default value which must me of type to match param type
	f_index,						// no args, of present indicates values used as indexes, client can map own origin to always 0-origin internally
};

// FPParamDef, contains descriptor for each published function
//			   live in Tab<> in FPInterface

// per-param flags
#define FPP_HAS_RANGE		0x0001
#define FPP_HAS_VALIDATOR	0x0002
#define FPP_IN_PARAM		0x0004  // in-out flags used by _BR ref types to decide when to pass in source values or hand back returns
#define FPP_OUT_PARAM		0x0008  //   " "
#define FPP_IN_OUT_PARAM	0x000C  //   "  "  both
#define FPP_KEYARG			0x0010  // if p_keyArgDefault supplied, client canuse keyword args if supported for this param
#define FPP_INDEX			0x0020  // parameter values used as indexes, always 0-origin internally, allows client to map to other origins

/*! \sa  <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>, 
Class FPParamOptions, Template Class Tab.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class contains a descriptor for each published function, found in Tab\<\>
in <b>FPInterface</b>.
\par Data Members:
<b>MSTR internal_name;</b>\n\n
The internal name.\n\n
<b>StringResID description;</b>\n\n
The string resource ID of the description.\n\n
<b>ParamType2 type;</b>\n\n
The parameter type. See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List
of ParamType2 Choices</a> for details.\n\n
<b>EnumID enumID;</b>\n\n
ID of symbolic enumeration in owning interface if any.\n\n
<b>USHORT flags;</b>\n\n
The parameter definition flags;\n\n
<b>FPP_HAS_RANGE</b>\n\n
Indicates that the parameter definition contains a range.\n\n
<b>FPP_HAS_VALIDATOR</b>\n\n
Indicates that the parameter has a validator.\n\n
<b>FPP_IN_PARAM</b>\n\n
In flag used by _BR ref types to decide when to pass in source values or hand
back returns.\n\n
<b>FPP_OUT_PARAM</b>\n\n
Out flag used by _BR ref types to decide when to pass in source values or hand
back returns.\n\n
<b>FPP_IN_OUT_PARAM</b>\n\n
In-Out flag used by _BR ref types to decide when to pass in source values or
hand back returns.\n\n
<b>FPP_KEYARG</b>\n\n
If p_keyArgDefault supplied, the client can use keyword args if supported for
this param.\n\n
<b>FPP_INDEX</b>\n\n
Parameter values used as indexes, always 0-origin internally, allows client to
map to other origins.\n\n
<b>FPParamOptions* options;</b>\n\n
Present if non-NULL, a pointer to the parameter options object.  */
class FPParamDef: public MaxHeapOperators
{
public:
	DWORD			cbStruct;			// size of the struct
	MSTR			internal_name;
	StringResID		description;
	ParamType2		type;
	EnumID			enumID;			// ID of symbolic enumeration in owning interface if any
	USHORT			flags;
	FPParamOptions*	options;		// present if non-NULL

	/*! \remarks Constructor.
	\par Default Implementation:
	<b>{ description = 0; options = NULL; flags = FPP_IN_OUT_PARAM; enumID =
	FP_NO_ENUM; }</b> */
	FPParamDef() : cbStruct(sizeof(FPParamDef)), description(0), options(NULL), enumID(FP_NO_ENUM), flags(FPP_IN_OUT_PARAM) { }
	CoreExport ~FPParamDef() ;
};

// FPParams,  contains a Tab<> of FPValue's being the actual parameters for an FP Fn call
//			  at present, FP Fn arguments are positional. We could expand this to allow 
//			  optional, order-independent keyword params

/*! \sa  Class FPValue\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This parameters class contains a <b>Tab\<\></b> of <b>FPValue's</b>, being the
actual parameters for an FP Function call.
\par Data Members:
<b>Tab\<FPValue\> params;</b>\n\n
The table of FPValue's contained in this class.  */
class FPParams: public MaxHeapOperators
{
public:
	Tab<FPValue>	params;

			  /*! \remarks Constructor.
			  \par Default Implementation:
			  <b>{ }</b> */
			  FPParams() { }
	/*! \remarks Constructor.
	\par Parameters:
	<b>int count</b>\n\n
	The number of parameter values to add.\n\n
	<b>...</b>\n\n
	This method takes a variable number of arguments representing the parameter
	values that will be stored in the <b>params</b> table. */
	CoreExport FPParams(int count, ...);
	/*! \remarks Destructor. */
	CoreExport ~FPParams();

	/*! \remarks This method loads a number of parameter values.
	\par Parameters:
	<b>int count</b>\n\n
	The number of parameter values to add.\n\n
	<b>...</b>\n\n
	This method takes a variable number of arguments representing the parameter
	values that will be stored in the <b>params</b> table. */
	CoreExport void Load(int count, ...);
};

// symbolic enums for an interface, 
//   used by metadata clients to support symbolic value for TYPE_ENUM types (ints)

/*! \sa  Class FPInterface, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class contains an ID for the enumeration and a table of structures which
contains a name and an integer code for each item. This is used by metadata
clients to support symbolic values for <b>TYPE_ENUM</b> types (ints).\n\n
One or more symbolic enums, similar to C++ enums, can now be added to an
FPInterface's metadata, and individual int parameters and/or results for
functions in that interface can be defined as <b>TYPE_ENUM</b> and associated
with one of the enum lists. Working in a similar manner as MAXScript, this
allows metadata clients to support symbolic encodings for these parameters and
results.\n\n
Enums are defined in the FPInterface descriptor following the function and
property definitions as sets of string/code pairs. Each enum list is identified
by a unique integer, similar to function IDs, which is used to associated a
<b>TYPE_ENUM</b> parameter or result with its enum. IDs for these would
normally be defined somewhere near the function IDs for an interface. For
example:
\code
// function IDs
enum
{ 
	bmm_getWidth,
	bmm_getHeight, 
	bmm_getType, 
	bmm_copyImage,
	...
};
// enum IDs
enum
{
	bmm_type, 
	bmm_copy_quality,
	...
};
\endcode
might be some of the IDs for a possible bitmap manager interface. The two enums
provide symbolic codes for the bitmap type and copyImage quality defines in the
"bitmap.h" SDK header, such as <b>BMM_PALETTED, BMM_TRUE_32,
COPY_IMAGE_RESIZE_LO_QUALITY</b>, etc. In the descriptor for the interface, any
enum lists follow the function and property definitions. They are introduced by
the special tag, 'enums', as in the following example:
\code
	static FPInterfaceDesc bmmfpi
	(
		BMM_INTERFACE, _M("bmm"), IDS_BMMI, NULL, FP_CORE,
			...
		bmm_copyImage, _M("copyImage"), ...
			_M("copyType"), IDS_COPYTYPE, TYPE_ENUM, bmm_copy_quality,
			...
		properties,
			geo_getType, geo_setType, _M("type"), 0, TYPE_ENUM, bmm_type,
		enums,
			bmm_type, 7,
				"lineArt"  , BMM_LINE_ART,
				"paletted" , BMM_PALETTED,
				"gray8"    , BMM_GRAY_8,
				"gray16"   , BMM_GRAY_16 ,
				"true16"   , BMM_TRUE_16,
				"true32"   , BMM_TRUE_32,
				"true24"   , BMM_TRUE_64,
			bmm_copy_quality, 4,
				"crop"      , COPY_IMAGE_CROP,
				"resizeLo"  , COPY_IMAGE_RESIZE_LO_QUALITY,
				"resizeHi"  , COPY_IMAGE_RESIZE_HI_QUALITY,
				"useCustom" , COPY_IMAGE_USE_CUSTOM,
		end
	);
\endcode
In the above example, the enums are listed following the function and property
definitions. They are introduced by the 'enums' tag and consist of an enum ID
followed by a count of items, followed by that many string and code pairs. By
attaching them to the interface like this, any number of functions and
properties in the interface can use them.\n\n
The above example also has function and property definitions showing the use of
<b>TYPE_ENUM</b>. The <b>copyImage</b> function takes a <b>copyType</b>
parameter which uses the <b>bmm_copy_quality enum</b> and the type property
uses the <b>bmm_type enum</b>. In all situations where TYPE_xxx types can be
supplied in a descriptor, including the new property definitions,
<b>TYPE_ENUM</b> can be used to indicate an int by-value type.
<b>TYPE_ENUM's</b> must always be followed by an enum ID. This is the only case
in which the type is specified as a pair of values. <b>TYPE_ENUM</b> parameters
and results show up in MAXScript as # names. For example, if a bmm interface
was in the variable 'bm1' and the bitmap type was <b>BMM_GRAY_16</b>:
\code
bm1.type
--> #gray16
bm1.type = #true32 -- set it to #true24 (code is BMM_TRUE_24)
bm2 = bm1.copyImage #resizeHi
\endcode
the integer <b>TYPE_ENUM</b> codes are translated back-and-forth to symbolic #
names by MAXScript using the definitions in the FPInterface descriptor's enums.
If you need to access the enum metadata in an <b>FPInterfaceDesc</b>, it is
available in the 'enumerations' data member. This is a Tab\<\> of
pointers to <b>FPEnum</b> class instances which themselves contain a Tab\<\> of
name, code pairs. See class FPEnum in /MAXSDK/INCLUDE/FnPub.h for
details. */
class FPEnum : public MaxHeapOperators
{
public:
	//! ID for this enumeration
	EnumID	ID;
	struct enum_code: public MaxHeapOperators
	{
		//! Symbolic name for the enum.
		MCHAR*	name;
		//! Equivalent integer code
		int		code;
	};
	//! The table of enumeration codes.
	Tab<enum_code> enumeration; 
};

// FPValue, a variant structure containing a single value, passable as a FP Fn parameter or result
class PBBitmap;
class Texmap;
class Value;

/*! \sa  <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>, 
Template Class Tab.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class contains a single value used as part of the Function Publishing
system. It's capable of holding any of the FnPub supported types. This value is
used as a parameter and as a return value.
\par Data Members:
<b>ParamType2 type;</b>\n\n
This data member identifies which type of value is stored in the union
below.\n\n
The following union contains a single value, pointer to a single value, or a
pointer to a single table (Tab\<\>) of values.\n\n
\code
union
{
	int i;
	float f;
	int* iptr;
	float* fptr;
	Point3* p;
	TimeValue t;
	MCHAR* s;
	MSTR* tstr;
	PBBitmap* bm;
	Mtl* mtl;
	Texmap* tex;
	INode* n;
	ReferenceTarget* r;
	Matrix3* m;
	AngAxis* aa;
	Quat* q;
	Ray* ray;
	Point2* p2;
	BitArray* bits;
	ClassDesc* cd;
	Mesh* msh;
	Object* obj;
	Control* ctrl;
	Interval* intvl;
	POINT* pt;
	HWND hwnd;
	IObject* iobj;
	FPInterface* fpi;
	void* ptr;
	Color* clr;
	FPValue* fpv;
	Value* v;
// Tab<>s of above
	Tab<int>* i_tab;
	Tab<float>* f_tab;
	Tab<Point3*>* p_tab;
	Tab<TimeValue>* t_tab;
	Tab<MCHAR*>* s_tab;
	Tab<MSTR*>* tstr_tab;
	Tab<PBBitmap*>* bm_tab;
	Tab<Mtl*>* mtl_tab;
	Tab<Texmap*>* tex_tab;
	Tab<INode*>* n_tab;
	Tab<ReferenceTarget*>* r_tab;
	Tab<Matrix3*>* m3_tab;
	Tab<AngAxis*>* aa_tab;
	Tab<Quat*>* q_tab;
	Tab<Ray*>* ray_tab;
	Tab<Point2*>* p2_tab;
	Tab<BitArray*>* bits_tab;
	Tab<ClassDesc*>* cd_tab;
	Tab<Mesh*>* msh_tab;
	Tab<Object*>* obj_tab;
	Tab<Control*>* ctrl_tab;
	Tab<Interval*>* intvl_tab;
	Tab<POINT*>* pt_tab;
	Tab<HWND>* hwnd_tab;
	Tab<IObject*>* iobj_tab;
	Tab<FPInterface*>* fpi_tab;
	Tab<void*>* ptr_tab;
	Tab<Color*>* clr_tab;
	Tab<FPValue*>* fpv_tab;
	Tab<Value*>* v_tab;
};
\endcode  */
class FPValue: public MaxHeapOperators
{
public:
	ParamType2	type;
	union 
	{
		int					i;
		float				f;
		DWORD				d;
		bool				b;

		INT_PTR				intptr;

		// SR NOTE64: Bumps union to 64 bits in Win32, this assumes that FPValue are
		// temporaries so this should not cause too much memory bloat.
		INT64				i64;

		double				dbl;

		int*				iptr;
		float*				fptr;
		Point3*				p;
		Point4*				p4;
		TimeValue			t;
		MCHAR*				s;
		MSTR*				tstr;
		PBBitmap*			bm;
		Mtl*				mtl;
		Texmap*				tex;
		INode*				n;
		ReferenceTarget*	r;
		Matrix3*			m;
		AngAxis*			aa;
		Quat*				q;
		Ray*				ray;
		Point2*				p2;
		BitArray*			bits;
		ClassDesc*			cd;
		Mesh*				msh;
		Object*				obj;
		Control*			ctrl;
		Interval*			intvl;
		POINT*				pt;
		HWND				hwnd;
		IObject*			iobj;
		FPInterface*		fpi;
		void*				ptr;
		Color*				clr;
		AColor*				aclr;
		FPValue*			fpv;
		Value*				v;
		DWORD*				dptr;
		bool*				bptr;
		INT_PTR*			intptrptr;
		INT64*				i64ptr;
		double*				dblptr;

		// Tab<>s of above
		Tab<int>*			i_tab;
		Tab<float>*			f_tab;
		Tab<Point3*>*		p_tab;
		Tab<Point4*>*		p4_tab;
		Tab<TimeValue>*		t_tab;
		Tab<MCHAR*>*		s_tab;
		Tab<MSTR*>*			tstr_tab;
		Tab<PBBitmap*>*		bm_tab;
		Tab<Mtl*>*			mtl_tab;
		Tab<Texmap*>*		tex_tab;
		Tab<INode*>*		n_tab;
		Tab<ReferenceTarget*>*	r_tab;
		Tab<Matrix3*>*		m3_tab;
		Tab<AngAxis*>*		aa_tab;
		Tab<Quat*>*			q_tab;
		Tab<Ray*>*			ray_tab;
		Tab<Point2*>*		p2_tab;
		Tab<BitArray*>*		bits_tab;
		Tab<ClassDesc*>*	cd_tab;
		Tab<Mesh*>*			msh_tab;
		Tab<Object*>*		obj_tab;
		Tab<Control*>*		ctrl_tab;
		Tab<Interval*>*		intvl_tab;
		Tab<POINT*>*		pt_tab;
		Tab<HWND>*			hwnd_tab;
		Tab<IObject*>*		iobj_tab;
		Tab<FPInterface*>*	fpi_tab;
		Tab<void*>*			ptr_tab;
		Tab<Color*>*		clr_tab;
		Tab<AColor*>*		aclr_tab;
		Tab<FPValue*>*		fpv_tab;
		Tab<Value*>*		v_tab;
		Tab<DWORD>*			d_tab;
		Tab<bool>*			b_tab;
		Tab<INT_PTR>*		intptr_tab;
		Tab<INT64>*			i64_tab;
		Tab<double>*		dbl_tab;
	};

			   /*! \remarks Constructor
			   \par Default Implementation:
			   <b>{ Init(); }</b> */
			   FPValue() { Init(); }
				/*! \remarks Copy Constructor.
				\par Parameters:
				<b>FPValue\& from</b>\n\n
				A reference to a FPValue to copy from.
				\par Default Implementation:
				<b>{ Init(); *this = from; }</b> */
			   FPValue(const FPValue& from) { Init(); *this = from; }
			   /*! \remarks Constructor
			   \par Default Implementation:
			   <b>{va_list ap; va_start(ap, type); ap = Loadva(type, ap);
			   va_end(ap);}</b> */
			   FPValue(int type, ...) { va_list ap; va_start(ap, type); ap = Loadva(type, ap); va_end(ap); }
	/*! \remarks Destructor.
	\par Default Implementation:
	<b>{ Free(); }</b> */
	CoreExport ~FPValue() { Free(); }
	/*! \remarks This method will free up all memory used by the class.\n\n
	  */
	CoreExport void Free();

	/*! \remarks This method will Initialize FPValue class.
	\par Default Implementation:
	<b>{ type = (ParamType2)TYPE_INT; s = NULL; }</b> */
	CoreExport void	Init();

	//! \brief Allocates and initializes the Tab in the FPValue for TAB types.
	/*! \param [in] type - The ParamType2 type to set the FPValue to.
	\param [in] size - The size of the tab
	*/
	CoreExport void	InitTab(ParamType2 type, int size);

	/*! \remarks Assignment operator. */
	CoreExport FPValue& operator=(const FPValue& sv);
	CoreExport va_list Loadva(int type, va_list ap, bool ptr=false);
	/*! \remarks This method will load the FPValue class with the provided
	data.
	\par Parameters:
	<b>int type</b>\n\n
	The FPValue parameter type to load.\n\n
	<b>...</b>\n\n
	This method takes a variable number of arguments.
	\par Default Implementation:
	<b>{va_list ap; va_start(ap, type); ap = Loadva(type, ap); va_end(ap);}</b>
	*/
	inline    void Load(int type, ...) { va_list ap; va_start(ap, type); ap = Loadva(type, ap); va_end(ap); }
	/*! \remarks This method will load the FPValue class with the provided
	data.
	\par Parameters:
	<b>int type</b>\n\n
	The FPValue parameter type to load.\n\n
	<b>...</b>\n\n
	This method takes a variable number of arguments.
	\return  <b>{ va_list ap; va_start(ap, type);</b>\n\n
	<b>ap = Loadva(type, ap, true); va_end(ap); }</b>\n\n
	  */
	inline    void LoadPtr(int type, ...) { va_list ap; va_start(ap, type); ap = Loadva(type, ap, true); va_end(ap); }

	//! \brief Saves the FPValue.
	/*! Note that raw pointers are not saved. FPValues that contain a raw pointer (TYPE_IOBJECT, TYPE_INTERFACE, 
	TYPE_VOID, TYPE_HWND) will store a pointer value of NULL.

	For TYPE_VALUE values, only the following singleton values are saved and loaded: undefined, unsupplied, emptyVal, ok.

	All other value types are saved as undefined.
	\param [in] isave - This class provides methods to save data to disk. 
	\return IOResult - One of the following values:
	IO_OK - The result was acceptable - no errors.
	IO_ERROR - This is returned if an error occurred.
	*/
	/*! \remarks Saves the FPValue. Note that raw pointers are not saved.
	FPValues that contain a raw pointer (TYPE_IOBJECT, TYPE_INTERFACE,
	TYPE_VOID, TYPE_HWND) will store a pointer value of NULL. For TYPE_VALUE
	values, only the following singleton values are saved and loaded:
	undefined, unsupplied, emptyVal, ok. All other value types are saved as
	undefined.
	\par Parameters:
	<b>isave</b>\n\n
	This class provides methods to save data to disk. \see ISave
	\return  IOResult - One of the following values:\n\n
	IO_OK - The result was acceptable - no errors.\n\n
	IO_ERROR - This is returned if an error occurred.\n\n
	*/
	CoreExport IOResult Save(ISave* isave);

	//! \brief Loads the FPValue.
	/*! \param [in] iload - This class provides methods to load data from disk.
	\return IOResult - One of the following values:
	IO_OK - The result was acceptable - no errors.
	IO_ERROR - This is returned if an error occurred.
	*/
	CoreExport IOResult Load(ILoad* iload);

	//! \brief Returns true if the data associated with the type is represented by a pointer. If the type is a TAB type, whether the data in the Tab is represented by a pointer.
	/*! \param [in] type - The ParamType2 type to test.
	\return bool - True if the FPValue's data, or the Tab's data if a TAB type, is a pointer
	*/
	CoreExport static bool IsPointerBasedType(ParamType2 type);

};

// optional param-specific descriptor info
/*! \sa  Class FPValue, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>,
Template Class Tab.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class contains the optional parameters holding specific descriptor
information.
\par Data Members:
<b>FPValue range_low;</b>\n\n
The low range if specified.\n\n
<b>FPValue range_high;</b>\n\n
The high range if specified.\n\n
<b>FPValidator* validator;</b>\n\n
The validator if specified.\n\n
<b>FPValue keyarg_default;</b>\n\n
The default if value is optional keyword arg\n\n
<b>BYTE pos_param_count;</b>\n\n
The count of positional params in event of keyword arg presence  */
class FPParamOptions: public MaxHeapOperators
{
public:
	DWORD			cbStruct;			// size of the struct
	FPValue			range_low;		// range values if specified
	FPValue			range_high;
	FPValidator*	validator;		// validator if specified
	FPValue			keyarg_default; // default if value is optional keyword arg

	/*! \remarks Constructor. */
	FPParamOptions() : cbStruct(sizeof(FPParamOptions)), validator(NULL) { }
};

// virtual base class for parameter validation objects
/*! \sa  Class InterfaceServer, Class FPInterfaceDesc, Class FPValue, Class Interface_ID, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
An interface descriptor may contain validation information for individual
parameters, so that clients (such as MAXScript) can validate values given as
parameters to FPInterface calls, prior to making the call. The validation
information can be in the form of a range of values for int and float types, or
more generally, a validator object that is called to the validate a parameter
value.\n\n
The validation info is specified in the FPInterface descriptor in optional
tagged entries following the parameters to be validated. The two possible tags
are <b>f_range</b> and <b>f_validator</b>. An instance of this class is used
when <b>f_validator</b> is specified.\n\n
Here's an example from a possible mixin interface to Cylinder:\n\n
<b>static FPInterfaceDesc cylfpi (</b>\n\n
<b>CYL_INTERFACE, _M("cylMixin"), 0, \&cylinderDesc, FP_MIXIN,</b>\n\n
<b>...</b>\n\n
<b>cyl_setRadius, _M("setRadius"), 0, TYPE_VOID, 0, 1,</b>\n\n
<b>_M("radius"), 0, TYPE_FLOAT,</b>\n\n
<b>f_range, 0.0, 10000.0,</b>\n\n
<b>cyl_setDirection, _M("setDirection"), 0, TYPE_VOID, 0, 1,</b>\n\n
<b>_M("vector"), 0, TYPE_POINT3,</b>\n\n
<b>f_validator, \&cylValidator,</b>\n\n
<b>...</b>\n\n
<b>end</b>\n\n
<b>);</b>\n\n
The "vector" parameter in the above example has a validator object specified.
This must be a pointer to an instance of a class derived from the new class,
<b>FPValidator</b>, defined in <b>I/MAXSDK/INCLUDE/iFnPub.h</b>. This is a
virtual base class, containing a single method, <b>Validate()</b>, that is
called to validate a prospective value for a parameter. You would typically
subclass <b>FPValidator</b> in your code and provide an implementation of
<b>Validate()</b> to do the validation.  */
class FPValidator : public InterfaceServer 
{
public:
	// validate val for the given param in function in interface
	/*! \remarks This method is called to validate the value <b>val</b> passed
	for the given parameter in the function whose ID is passed in the specified
	interface passed. If there are many parameters to validate this way,
	developers can choose to provide a separate subclass for each parameter or
	a single subclass and switch on the parameter identification supplied.
	\par Parameters:
	<b>FPInterface* fpi</b>\n\n
	Points to the interface the function is a part of.\n\n
	<b>FunctionID fid</b>\n\n
	The ID of the function within the interface above.\n\n
	<b>int paramNum</b>\n\n
	Identifies which parameter within the function above to validate.\n\n
	<b>FPValue\& val</b>\n\n
	The value to validate.\n\n
	<b>MSTR\& msg</b>\n\n
	Update this string with an error message if needed. The user of the
	Validator can then display this string.
	\return  Returns true if the value was valid; false if invalid. */
	virtual bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, MSTR& msg)=0;
};

// virtual base class for action function macroRecorder emitter objects
class FPMacroEmitter: public MaxHeapOperators
{
public:
	// gen macro for a call to given action fn
	virtual void EmitMacro(FPInterface* fpi, FPFunctionDef* fd)=0;
};

// IObject class, virtual base class for random classes that want to 
//                implement GetInterface().  Similar to IUnknown in COM.
//                would be used to pass interface-based objects not
//                otherwise supported by the FPValue base types.
//				  MAXScript handles these and will use GetInterface() to 
//                publish interface & methods as properties of the IObjects
/*! \sa  Class BaseInterfaceServer,  Class FPInterface,  Class FPInterfaceDesc, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing</a>, Class InterfaceServer,\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class used to publish functions from those objects not derived
from Animatable. A developer inherits from this class and implements the
methods of this class to provide information about the interfaces published by
the class.\n\n
There is a corresponding <b>ParamType2</b> type code, <b>TYPE_IOBJECT</b>, that
allows instances of these classes to be passed and returned in FPInterface
methods. This provides a simple form of user-defined type, in the sense that
these instance collections are passed as interfaces rather than pointers.
MAXScript has wrapper value classes for IObjects and so this mechanism provides
a light-weight alternative to the MAXScript SDK facilities for adding new
wrapper value classes to the scripter.\n\n
MAXScript also calls the <b>AcquireInterface()</b> and
<b>ReleaseInterface()</b> methods on IObjects as it creates and collects these
wrappers, so that IObject objects can keep track of MAXScript's extant
references to them. <br>  \n\n
  */
class IObject : public BaseInterfaceServer
{
public:
	// inherits interface access and iteration from BaseInterfaceServer
	//
	//   virtual BaseInterface* GetInterface(Interface_ID id);
	//   virtual int NumInterfaces();			
	//   virtual BaseInterface* GetInterfaceAt(int i)

	// object/class name
	/*! \remarks Returns the object/class name.
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual MCHAR* GetIObjectName() { return _M(""); }							
	// interface enumeration...
	// IObject ref management (can be implemented by dynamically-allocated IObjects for
	//                         ref-count based lifetime control)
	/*! \remarks This method is called when MAXScript makes a reference to
	this object. This is part of the IObject reference management and can be
	implemented by dynamically allocated IObjects for ref-count based lifetime
	control.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void AcquireIObject() { }
	/*! \remarks This method is called when MAXScript deletes a reference to
	this object. This is part of the IObject reference management and can be
	implemented by dynamically allocated IObjects for ref-count based lifetime
	control.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void ReleaseIObject() { }
	// virtual destructor
	/*! \remarks This method is the virtual destructor for the IObject.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void DeleteIObject() { }
};

// base exception class for FP-based exceptions.  FnPub functions can throw this
//  instances of this class or subclasses to signal error conditions.
/*! \sa  Class FPInterfaceDesc, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
Function Publishing interface functions can now report fatal error conditions
to callers by using C++ exception-handling. This base class, MAXException, can
be thrown directly, or subclassed as needed for error grouping. The class
contains a message buffer and an optional error code. You would signal an error
using the <b>MAXException()</b> constructor and the C++ throw statement, as in
the following example:\n\n
<b>...</b>\n\n
<b>if (discrim \< 0.0) // oh-oh, not good</b>\n\n
<b>throw MAXException ("Unable to find root.", -23);</b>\n\n
<b>...</b>\n\n
This signals a fatal error with the message and code shown. If the error occurs
during a call to the function by MAXScript code, it will be trapped by
MAXScript and the error message will be displayed and the running script will
be terminated (but 3ds Max will continue running). If the error occurs during a
C++-level call, typically the outer 3ds Max error catcher will catch and report
the error and then exit 3ds Max, or clients of the interface can install their
own catch code.
\par Data Members:
<b>MSTR message;</b>\n\n
The exception message.\n\n
<b>int error_code;</b>\n\n
The exception error code.  */
class MAXException: public MaxHeapOperators
{
public:
	MSTR	message;
	int		error_code;

	/*! \remarks Constructor. The data members are initialized to the values
	passed.
	\par Parameters:
	<b>MCHAR* msg</b>\n\n
	The exception message.\n\n
	<b>int code=0</b>\n\n
	The exception error code.
	\par Default Implementation:
	<b>{}</b> */
	MAXException(MCHAR* msg, int code=0) : message(msg), error_code(code) { }
};

// publishing DESCRIPTOR & FUNCTION_MAP macros

#define DECLARE_DESCRIPTOR(_interface)					\
	public:												\
	_interface() { }									\
	_interface(Interface_ID id, MCHAR* name,			\
			   StringResID descr, ClassDesc* cd,		\
			   USHORT flags, ...)						\
	{													\
		va_list ap;										\
		va_start(ap, flags);							\
		load_descriptor(id, name, descr, cd, flags, ap); \
		va_end(ap);										\
	}								
					
#define DECLARE_DESCRIPTOR_INIT(_interface)				\
	public:												\
	_interface() { init(); }							\
	void init();										\
	_interface(Interface_ID id, MCHAR* name,			\
			   StringResID descr, ClassDesc* cd,		\
			   USHORT flags, ...)						\
	{													\
		init();											\
		va_list ap;										\
		va_start(ap, flags);							\
		load_descriptor(id, name, descr, cd, flags, ap); \
		va_end(ap);										\
	}

// NOTE: the default ctor is NOT called! If any 
// initialization is required use DECLARE_DESCRIPTOR_INIT_NDC
#define DECLARE_DESCRIPTOR_NDC(_interface)				\
	public:												\
	_interface(Interface_ID id, MCHAR* name,			\
			   StringResID descr, ClassDesc* cd,		\
			   USHORT flags, ...)						\
	{													\
		va_list ap;										\
		va_start(ap, flags);							\
		load_descriptor(id, name, descr, cd, flags, ap); \
		va_end(ap);										\
	}								
					
#define DECLARE_DESCRIPTOR_INIT_NDC(_interface)			\
	public:												\
	void init();										\
	_interface(Interface_ID id, MCHAR* name,			\
				StringResID descr, ClassDesc* cd,		\
				USHORT flags, ...)						\
	{													\
		init();											\
		va_list ap;										\
		va_start(ap, flags);							\
		load_descriptor(id, name, descr, cd, flags, ap); \
		va_end(ap);										\
	}								
					
#define BEGIN_FUNCTION_MAP								\
	public:												\
	FPStatus _dispatch_fn(FunctionID fid, TimeValue t,	\
					FPValue& result, FPParams* p)		\
	{													\
		UNUSED_PARAM(t);								\
		UNUSED_PARAM(result);							\
		UNUSED_PARAM(p);								\
		FPStatus status = FPS_OK;						\
		switch (fid)									\
		{
	
#define BEGIN_FUNCTION_MAP_PARENT(Parent)				\
	public:												\
	FPStatus _dispatch_fn(FunctionID fid, TimeValue t,	\
					FPValue& result, FPParams* p)		\
	{													\
		FPStatus status									\
			= Parent::_dispatch_fn(fid, t, result, p);	\
		if (status == FPS_OK) return status;			\
		status = FPS_OK;								\
		switch (fid)									\
		{

#define END_FUNCTION_MAP								\
			default: status = FPS_NO_SUCH_FUNCTION;		\
		}												\
		return status;									\
	}

#define NO_FUNCTION_MAP									\
	public:												\
	FPStatus _dispatch_fn(FunctionID fid, TimeValue t,	\
					FPValue& result, FPParams* p)		\
	{													\
		return FPS_NO_SUCH_FUNCTION;					\
	}

// ----------- indivudal MAP entry macros ----------

#define FP_FIELD(_type, _v)			(_type##_FIELD(_v))

// Action function

#define FN_ACTION(_fid, _fn)							\
	case _fid:											\
		status = _fn();									\
		break;	

// predicates

#define FN_PRED(_fid, _fn)								\
	case _fid:											\
		result.Load(TYPE_BOOL, _fn());					\
		break;	
#define FN_PREDS(_fid1, _fn1, _fid2, _fn2, _fid3, _fn3) \
	case _fid1:											\
		result.Load(TYPE_BOOL, _fn1());					\
		break;											\
	case _fid2:											\
		result.Load(TYPE_BOOL, _fn2());					\
		break;											\
	case _fid3:											\
		result.Load(TYPE_BOOL, _fn3());					\
		break;	

//! \name Property Macros
//! \brief Turns getters and setters into maxscript properties
//@{
//! \brief Property Macro
/*! This takes five arguments:
	\param _getID - The enum associated with the getter function.
	\param _getFn - The pointer to the getter function.
	\param _setID - The enum associated with the setter function.
	\param _setFn - The pointer to the setter function.
	\param _ptype - An enum (ParamType or ParamType2) specifying the type of property. */
#define PROP_FNS(_getID, _getFn, _setID, _setFn, _ptype) \
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(			\
			_getFn()));									\
		break;											\
	case _setID:										\
		_setFn(FP_FIELD(_ptype, p->params[0]));			\
		break;	

//! \brief Read only Property Macro
/*! Same as PROP_FNS, except specifies a property that is read only, and has
	no associated setter function. */
#define RO_PROP_FN(_getID, _getFn, _ptype)				\
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(			\
			_getFn()));									\
		break;

//! \brief Property Macro with Time parameter
/*! Same as PROP_FNS, except specifies an addition TimeValue is passed
	to the getter and setter functions. */
#define PROP_TFNS(_getID, _getFn, _setID, _setFn, _ptype) \
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(t)));	\
		break;											\
	case _setID:										\
		_setFn(FP_FIELD(_ptype, p->params[0]), t);		\
		break;	

//! \brief Read only Property Macro with Time parameter
/*! Same as PROP_TFNS, except specifies a property that is read only, and has
	no associated setter function. */
#define RO_PROP_TFN(_getID, _getFn, _ptype)				\
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(t)));	\
		break;
//@}

//! \name Static Method Macros
//! \brief Property FN_MAP macros for the Static Method Interfaces used in MAXScript.
//@{
#define SM_PROP_FNS(_getID, _getFn, _setID, _setFn, _ptype) \
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0])))); \
		break;											\
	case _setID:										\
		_setFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0]), FP_FIELD(_ptype, p->params[1])); \
		break;	
#define SM_RO_PROP_FN(_getID, _getFn, _ptype)			\
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0]))));\
		break;
#define SM_PROP_TFNS(_getID, _getFn, _setID, _setFn, _ptype) \
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0]), t)));\
		break;											\
	case _setID:										\
		_setFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0]), FP_FIELD(_ptype, p->params[1]), t); \
		break;	
#define SM_RO_PROP_TFN(_getID, _getFn, _ptype)			\
	case _getID:										\
		result.LoadPtr(_ptype,	_ptype##_RSLT(		\
			_getFn(FP_FIELD(TYPE_FPVALUE_BR, p->params[0]), t)));	\
		break;
//@}

//! \name Function Macros with No Time.
//! \brief Functions with return value, no time
//@{
#define FN_VA(_fid, _rtype, _f)							\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f(p)));									\
		break;

#define FN_0(_fid, _rtype, _f)							\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f()));										\
		break;

#define FN_1(_fid, _rtype, _f, _p1)						\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f(FP_FIELD(_p1, p->params[0]))));			\
		break;	

#define FN_2(_fid, _rtype, _f, _p1, _p2)				\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]))));	\
		break;	
#define FN_3(_fid, _rtype, _f, _p1, _p2, _p3)			\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]))));	\
		break;	
#define FN_4(_fid, _rtype, _f, _p1, _p2, _p3, _p4)		\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]))));	\
		break;	
#define FN_5(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]))));	\
		break;	

#define FN_6(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]))));	\
		break;	
#define FN_7(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]))));	\
		break;	
#define FN_8(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]),		\
					   FP_FIELD(_p8, p->params[7]))));	\
		break;	
#define FN_9(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]),		\
					   FP_FIELD(_p8, p->params[7]),		\
					   FP_FIELD(_p9, p->params[8]))));	\
		break;	
#define FN_10(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9, _p10)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]),		\
					   FP_FIELD(_p8, p->params[7]),		\
					   FP_FIELD(_p9, p->params[8]),		\
					   FP_FIELD(_p10, p->params[9]))));	\
		break;	
#define FN_11(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9, _p10,_p11)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]),		\
					   FP_FIELD(_p8, p->params[7]),		\
					   FP_FIELD(_p9, p->params[8]),		\
					   FP_FIELD(_p10, p->params[9]),		\
					   FP_FIELD(_p11, p->params[10]))));	\
		break;	
//@}

//! \name Void Function Macros with No Time.
//! \brief Specifies void functions that take no time parameter
//@{
#define VFN_VA(_fid, _f)								\
	case _fid:											\
			_f(p);										\
		break;
#define VFN_0(_fid, _f)									\
	case _fid:											\
			_f();										\
		break;
#define VFN_1(_fid, _f, _p1)							\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]));			\
		break;	
#define VFN_2(_fid, _f, _p1, _p2)						\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]));			\
		break;	
#define VFN_3(_fid, _f, _p1, _p2, _p3)					\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]));		\
		break;	
#define VFN_4(_fid, _f, _p1, _p2, _p3, _p4)				\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]));		\
		break;	
#define VFN_5(_fid, _f, _p1, _p2, _p3, _p4, _p5)		\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					FP_FIELD(_p5, p->params[4]));		\
		break;	
#define VFN_6(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6)		\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					FP_FIELD(_p5, p->params[4]),		\
					FP_FIELD(_p6, p->params[5]));		\
		break;	
#define VFN_7(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7)		\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					FP_FIELD(_p5, p->params[4]),		\
					FP_FIELD(_p6, p->params[5]),		\
					FP_FIELD(_p7, p->params[6]));		\
		break;	
//@}

//! \name Const Return Function Macros
//! \brief Specifies Functions with constant return values, with no time parameter.
//@{
#define CFN_VA(_fid, _rtype, _f)							\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
			_f(p))));	\
		break;

#define CFN_0(_fid, _rtype, _f)							\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
			_f())));	\
		break;

#define CFN_1(_fid, _rtype, _f, _p1)						\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
			_f(FP_FIELD(_p1, p->params[0])))));	\
		break;	

#define CFN_2(_fid, _rtype, _f, _p1, _p2)				\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1])))));	\
		break;	
#define CFN_3(_fid, _rtype, _f, _p1, _p2, _p3)			\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2])))));	\
		break;	
#define CFN_4(_fid, _rtype, _f, _p1, _p2, _p3, _p4)		\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3])))));	\
		break;	
#define CFN_5(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5)	\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(const_cast<_rtype##_TYPE>( \
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4])))));	\
		break;	
//@}

//! \name Value Returning Function Macros
//! \brief Specifies Functions that return values, with time parameter.
//@{
#define FNT_VA(_fid, _rtype, _f)						\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f(p, t)));	\
		break;
#define FNT_0(_fid, _rtype, _f)							\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f(t)));	\
		break;
#define FNT_1(_fid, _rtype, _f, _p1)					\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
			_f(FP_FIELD(_p1, p->params[0]),				\
					t)));	\
		break;	
#define FNT_2(_fid, _rtype, _f, _p1, _p2)				\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   t)));	\
		break;	
#define FNT_3(_fid, _rtype, _f, _p1, _p2, _p3)			\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(			\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   t)));	\
		break;	
#define FNT_4(_fid, _rtype, _f, _p1, _p2, _p3, _p4)		\
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   t)));	\
		break;	
#define FNT_5(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5) \
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   t)));	\
		break;	
#define FNT_6(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6) \
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   t)));	\
		break;	
#define FNT_7(_fid, _rtype, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7) \
	case _fid:											\
		result.LoadPtr(_rtype,	_rtype##_RSLT(		\
					_f(FP_FIELD(_p1, p->params[0]),		\
					   FP_FIELD(_p2, p->params[1]),		\
					   FP_FIELD(_p3, p->params[2]),		\
					   FP_FIELD(_p4, p->params[3]),		\
					   FP_FIELD(_p5, p->params[4]),		\
					   FP_FIELD(_p6, p->params[5]),		\
					   FP_FIELD(_p7, p->params[6]),		\
					   t)));	\
		break;	
//@}

//! \name Void Function (with time) Macros
//! \brief Specifies void Functions, with time parameter.
//@{
#define VFNT_VA(_fid, _f)								\
	case _fid:											\
			_f(p, t);									\
		break;
#define VFNT_0(_fid, _f)								\
	case _fid:											\
			_f(t);										\
		break;
#define VFNT_1(_fid, _f, _p1)							\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					t);									\
		break;	
#define VFNT_2(_fid, _f, _p1, _p2)						\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   t);										\
		break;	
#define VFNT_3(_fid, _f, _p1, _p2, _p3)					\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					t);									\
		break;	
#define VFNT_4(_fid, _f, _p1, _p2, _p3, _p4)			\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					t);									\
		break;	
#define VFNT_5(_fid, _f, _p1, _p2, _p3, _p4, _p5)		\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					FP_FIELD(_p5, p->params[4]),		\
					t);									\
		break;	
#define VFNT_6(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
					FP_FIELD(_p2, p->params[1]),		\
					FP_FIELD(_p3, p->params[2]),		\
					FP_FIELD(_p4, p->params[3]),		\
					FP_FIELD(_p5, p->params[4]),		\
					FP_FIELD(_p6, p->params[5]),		\
					t);									\
		break;	
#define VFNT_7(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   FP_FIELD(_p3, p->params[2]),				\
			   FP_FIELD(_p4, p->params[3]),				\
			   FP_FIELD(_p5, p->params[4]),				\
			   FP_FIELD(_p6, p->params[5]),				\
			   FP_FIELD(_p7, p->params[6]),				\
			   t);										\
		break;	
#define VFNT_8(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   FP_FIELD(_p3, p->params[2]),				\
			   FP_FIELD(_p4, p->params[3]),				\
			   FP_FIELD(_p5, p->params[4]),				\
			   FP_FIELD(_p6, p->params[5]),				\
			   FP_FIELD(_p7, p->params[6]),				\
			   FP_FIELD(_p8, p->params[7]),				\
			   t);										\
		break;	
#define VFNT_9(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   FP_FIELD(_p3, p->params[2]),				\
			   FP_FIELD(_p4, p->params[3]),				\
			   FP_FIELD(_p5, p->params[4]),				\
			   FP_FIELD(_p6, p->params[5]),				\
			   FP_FIELD(_p7, p->params[6]),				\
			   FP_FIELD(_p8, p->params[7]),				\
			   FP_FIELD(_p9, p->params[8]),				\
			   t);										\
		break;	
#define VFNT_10(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9, _p10)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   FP_FIELD(_p3, p->params[2]),				\
			   FP_FIELD(_p4, p->params[3]),				\
			   FP_FIELD(_p5, p->params[4]),				\
			   FP_FIELD(_p6, p->params[5]),				\
			   FP_FIELD(_p7, p->params[6]),				\
			   FP_FIELD(_p8, p->params[7]),				\
			   FP_FIELD(_p9, p->params[8]),				\
			   FP_FIELD(_p10, p->params[9]),			\
			   t);										\
		break;	
#define VFNT_11(_fid, _f, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9, _p10, _p11)	\
	case _fid:											\
			_f(FP_FIELD(_p1, p->params[0]),				\
			   FP_FIELD(_p2, p->params[1]),				\
			   FP_FIELD(_p3, p->params[2]),				\
			   FP_FIELD(_p4, p->params[3]),				\
			   FP_FIELD(_p5, p->params[4]),				\
			   FP_FIELD(_p6, p->params[5]),				\
			   FP_FIELD(_p7, p->params[6]),				\
			   FP_FIELD(_p8, p->params[7]),				\
			   FP_FIELD(_p9, p->params[8]),				\
			   FP_FIELD(_p10, p->params[9]),			\
			   FP_FIELD(_p11, p->params[10]),			\
			   t);										\
		break;	
//@}

// parameter type field selectors
#define TYPE_FLOAT_FP_FIELD				f
#define TYPE_INT_FP_FIELD				i
#define TYPE_RGBA_FP_FIELD				p
#define TYPE_POINT3_FP_FIELD			p
#define TYPE_FRGBA_FP_FIELD				p4
#define TYPE_POINT4_FP_FIELD			p4
#define TYPE_BOOL_FP_FIELD				i
#define TYPE_ANGLE_FP_FIELD				f
#define TYPE_PCNT_FRAC_FP_FIELD			f
#define TYPE_WORLD_FP_FIELD				f
#define TYPE_STRING_FP_FIELD			s
#define TYPE_FILENAME_FP_FIELD			s
#define TYPE_HSV_FP_FIELD				p
#define TYPE_COLOR_CHANNEL_FP_FIELD		f
#define TYPE_TIMEVALUE_FP_FIELD			i
#define TYPE_RADIOBTN_INDEX_FP_FIELD	i
#define TYPE_MTL_FP_FIELD				mtl
#define TYPE_TEXMAP_FP_FIELD			tex
#define TYPE_BITMAP_FP_FIELD			bm
#define TYPE_INODE_FP_FIELD				n
#define TYPE_REFTARG_FP_FIELD			r
#define TYPE_INDEX_FP_FIELD				i
#define TYPE_MATRIX3_FP_FIELD			m
#define TYPE_VOID_FP_FIELD				void_paramtype_bad
#define TYPE_ENUM_FP_FIELD				i
#define TYPE_INTERVAL_FP_FIELD			intvl
#define TYPE_ANGAXIS_FP_FIELD			aa
#define TYPE_QUAT_FP_FIELD				q
#define TYPE_RAY_FP_FIELD				ray
#define TYPE_POINT2_FP_FIELD			p2
#define TYPE_BITARRAY_FP_FIELD			bits
#define TYPE_CLASS_FP_FIELD				cd
#define TYPE_MESH_FP_FIELD				msh
#define TYPE_OBJECT_FP_FIELD			obj
#define TYPE_CONTROL_FP_FIELD			ctrl
#define TYPE_POINT_FP_FIELD				pt
#define TYPE_TSTR_FP_FIELD				tstr
#define TYPE_IOBJECT_FP_FIELD			iobj
#define TYPE_INTERFACE_FP_FIELD			fpi
#define TYPE_HWND_FP_FIELD				hwnd
#define TYPE_NAME_FP_FIELD				s
#define TYPE_COLOR_FP_FIELD				clr
#define TYPE_ACOLOR_FP_FIELD			aclr
#define TYPE_FPVALUE_FP_FIELD			fpv
#define TYPE_VALUE_FP_FIELD				v
#define TYPE_DWORD_FP_FIELD				d
#define TYPE_bool_FP_FIELD				b
#define TYPE_INTPTR_FP_FIELD			intptr
#define TYPE_INT64_FP_FIELD				i64
#define TYPE_DOUBLE_FP_FIELD			dbl

// Tab<>s of the above...

#define TYPE_FLOAT_TAB_FP_FIELD				f_tab
#define TYPE_INT_TAB_FP_FIELD				i_tab
#define TYPE_RGBA_TAB_FP_FIELD				p_tab
#define TYPE_POINT3_TAB_FP_FIELD			p_tab
#define TYPE_FRGBA_TAB_FP_FIELD				p4_tab
#define TYPE_POINT4_TAB_FP_FIELD			p4_tab
#define TYPE_BOOL_TAB_FP_FIELD				i_tab
#define TYPE_ANGLE_TAB_FP_FIELD				f_tab
#define TYPE_PCNT_FRAC_TAB_FP_FIELD			f_tab
#define TYPE_WORLD_TAB_FP_FIELD				f_tab
#define TYPE_STRING_TAB_FP_FIELD			s_tab
#define TYPE_FILENAME_TAB_FP_FIELD			s_tab
#define TYPE_HSV_TAB_FP_FIELD				p_tab
#define TYPE_COLOR_CHANNEL_TAB_FP_FIELD		f_tab
#define TYPE_TIMEVALUE_TAB_FP_FIELD			i_tab
#define TYPE_RADIOBTN_INDEX_TAB_FP_FIELD	i_tab
#define TYPE_MTL_TAB_FP_FIELD				mtl_tab
#define TYPE_TEXMAP_TAB_FP_FIELD			tex_tab
#define TYPE_BITMAP_TAB_FP_FIELD			bm_tab
#define TYPE_INODE_TAB_FP_FIELD				n_tab
#define TYPE_REFTARG_TAB_FP_FIELD			r_tab
#define TYPE_INDEX_TAB_FP_FIELD				i_tab
#define TYPE_MATRIX3_TAB_FP_FIELD			m3_tab
#define TYPE_ENUM_TAB_FP_FIELD				i_tab
#define TYPE_INTERVAL_TAB_FP_FIELD			intvl_tab
#define TYPE_ANGAXIS_TAB_FP_FIELD			aa_tab
#define TYPE_QUAT_TAB_FP_FIELD				q_tab
#define TYPE_RAY_TAB_FP_FIELD				ray_tab
#define TYPE_POINT2_TAB_FP_FIELD			p2_tab
#define TYPE_BITARRAY_TAB_FP_FIELD			bits_tab
#define TYPE_CLASS_TAB_FP_FIELD				cd_tab
#define TYPE_MESH_TAB_FP_FIELD				msh_tab
#define TYPE_OBJECT_TAB_FP_FIELD			obj_tab
#define TYPE_CONTROL_TAB_FP_FIELD			ctrl_tab
#define TYPE_POINT_TAB_FP_FIELD				pt_tab
#define TYPE_TSTR_TAB_FP_FIELD				tstr_tab
#define TYPE_IOBJECT_TAB_FP_FIELD			iobj_tab
#define TYPE_INTERFACE_TAB_FP_FIELD			fpi_tab
#define TYPE_HWND_TAB_FP_FIELD				hwnd_tab
#define TYPE_NAME_TAB_FP_FIELD				s_tab
#define TYPE_COLOR_TAB_FP_FIELD				clr_tab
#define TYPE_ACOLOR_TAB_FP_FIELD			aclr_tab
#define TYPE_FPVALUE_TAB_FP_FIELD			fpv_tab
#define TYPE_VALUE_TAB_FP_FIELD				v_tab
#define TYPE_DWORD_TAB_FP_FIELD				d_tab
#define TYPE_bool_TAB_FP_FIELD				b_tab
#define TYPE_INTPTR_TAB_FP_FIELD			intptr_tab
#define TYPE_INT64_TAB_FP_FIELD				i64_tab
#define TYPE_DOUBLE_TAB_FP_FIELD			dbl_tab

// by-pointer fields
#define TYPE_FLOAT_BP_FP_FIELD				fptr		
#define TYPE_INT_BP_FP_FIELD				iptr		
#define TYPE_BOOL_BP_FP_FIELD				iptr		
#define TYPE_ANGLE_BP_FP_FIELD				fptr		
#define TYPE_PCNT_FRAC_BP_FP_FIELD			fptr	
#define TYPE_WORLD_BP_FP_FIELD				fptr
#define TYPE_COLOR_CHANNEL_BP_FP_FIELD		fptr
#define TYPE_TIMEVALUE_BP_FP_FIELD			iptr
#define TYPE_RADIOBTN_INDEX_BP_FP_FIELD		iptr
#define TYPE_INDEX_BP_FP_FIELD				iptr
#define TYPE_ENUM_BP_FP_FIELD				iptr
#define TYPE_DWORD_BP_FP_FIELD				dptr
#define TYPE_bool_BP_FP_FIELD				bptr
#define TYPE_INTPTR_BP_FP_FIELD				intptrptr
#define TYPE_INT64_BP_FP_FIELD				i64ptr
#define TYPE_DOUBLE_BP_FP_FIELD				dblptr		

// by-reference fields
#define TYPE_FLOAT_BR_FP_FIELD				fptr		
#define TYPE_INT_BR_FP_FIELD				iptr		
#define TYPE_RGBA_BR_FP_FIELD				p	
#define TYPE_POINT3_BR_FP_FIELD				p	
#define TYPE_FRGBA_BR_FP_FIELD				p4	
#define TYPE_POINT4_BR_FP_FIELD				p4	
#define TYPE_BOOL_BR_FP_FIELD				iptr		
#define TYPE_ANGLE_BR_FP_FIELD				fptr		
#define TYPE_PCNT_FRAC_BR_FP_FIELD			fptr	
#define TYPE_WORLD_BR_FP_FIELD				fptr
#define TYPE_HSV_BR_FP_FIELD				p
#define TYPE_COLOR_CHANNEL_BR_FP_FIELD		f
#define TYPE_TIMEVALUE_BR_FP_FIELD			iptr
#define TYPE_RADIOBTN_INDEX_BR_FP_FIELD		iptr
#define TYPE_BITMAP_BR_FP_FIELD				bm
#define TYPE_INDEX_BR_FP_FIELD				iptr
#define TYPE_ENUM_BR_FP_FIELD				iptr
#define TYPE_REFTARG_BR_FP_FIELD			r
#define TYPE_MATRIX3_BR_FP_FIELD			m
#define TYPE_ANGAXIS_BR_FP_FIELD			aa
#define TYPE_QUAT_BR_FP_FIELD				q
#define TYPE_BITARRAY_BR_FP_FIELD			bits
#define TYPE_RAY_BR_FP_FIELD				ray
#define TYPE_POINT2_BR_FP_FIELD				p2
#define TYPE_MESH_BR_FP_FIELD				msh
#define TYPE_INTERVAL_BR_FP_FIELD			intvl
#define TYPE_POINT_BR_FP_FIELD				pt
#define TYPE_TSTR_BR_FP_FIELD				tstr
#define TYPE_COLOR_BR_FP_FIELD				clr
#define TYPE_ACOLOR_BR_FP_FIELD				aclr
#define TYPE_FPVALUE_BR_FP_FIELD			fpv
#define TYPE_DWORD_BR_FP_FIELD				dptr
#define TYPE_bool_BR_FP_FIELD				bptr
#define TYPE_INTPTR_BR_FP_FIELD				intptrptr
#define TYPE_INT64_BR_FP_FIELD				i64ptr
#define TYPE_DOUBLE_BR_FP_FIELD				dblptr		

// Tab<> by-reference fields
#define TYPE_FLOAT_TAB_BR_FP_FIELD			f_tab
#define TYPE_INT_TAB_BR_FP_FIELD			i_tab
#define TYPE_RGBA_TAB_BR_FP_FIELD			p_tab
#define TYPE_POINT3_TAB_BR_FP_FIELD			p_tab
#define TYPE_FRGBA_TAB_BR_FP_FIELD			p4_tab
#define TYPE_POINT4_TAB_BR_FP_FIELD			p4_tab
#define TYPE_BOOL_TAB_BR_FP_FIELD			i_tab
#define TYPE_ANGLE_TAB_BR_FP_FIELD			f_tab
#define TYPE_PCNT_FRAC_TAB_BR_FP_FIELD		f_tab
#define TYPE_WORLD_TAB_BR_FP_FIELD			f_tab
#define TYPE_STRING_TAB_BR_FP_FIELD			s_tab
#define TYPE_FILENAME_TAB_BR_FP_FIELD		s_tab
#define TYPE_HSV_TAB_BR_FP_FIELD			p_tab
#define TYPE_COLOR_CHANNEL_TAB_BR_FP_FIELD	f_tab
#define TYPE_TIMEVALUE_TAB_BR_FP_FIELD		i_tab
#define TYPE_RADIOBTN_INDEX_TAB_BR_FP_FIELD i_tab
#define TYPE_MTL_TAB_BR_FP_FIELD			mtl_tab
#define TYPE_TEXMAP_TAB_BR_FP_FIELD			tex_tab
#define TYPE_BITMAP_TAB_BR_FP_FIELD			bm_tab
#define TYPE_INODE_TAB_BR_FP_FIELD			n_tab
#define TYPE_REFTARG_TAB_BR_FP_FIELD		r_tab
#define TYPE_INDEX_TAB_BR_FP_FIELD			i_tab
#define TYPE_ENUM_TAB_BR_FP_FIELD			i_tab
#define TYPE_MATRIX3_TAB_BR_FP_FIELD		m3_tab
#define TYPE_ANGAXIS_TAB_BR_FP_FIELD		aa_tab
#define TYPE_QUAT_TAB_BR_FP_FIELD			q_tab
#define TYPE_BITARRAY_TAB_BR_FP_FIELD		bits_tab
#define TYPE_CLASS_TAB_BR_FP_FIELD			cd_tab
#define TYPE_RAY_TAB_BR_FP_FIELD			ray_tab
#define TYPE_POINT2_TAB_BR_FP_FIELD			p2_tab
#define TYPE_MESH_TAB_BR_FP_FIELD			msh_tab
#define TYPE_OBJECT_TAB_BR_FP_FIELD			obj_tab
#define TYPE_CONTROL_TAB_BR_FP_FIELD		ctrl_tab
#define TYPE_INTERVAL_TAB_BR_FP_FIELD		intvl_tab
#define TYPE_POINT_TAB_BR_FP_FIELD			pt_tab
#define TYPE_HWND_TAB_BR_FP_FIELD			hwnd_tab
#define TYPE_TSTR_TAB_BR_FP_FIELD			tstr_tab
#define TYPE_IOBJECT_TAB_BR_FP_FIELD		iobj_tab
#define TYPE_INTERFACE_TAB_BR_FP_FIELD		fpi_tab
#define TYPE_NAME_TAB_BR_FP_FIELD			s_tab
#define TYPE_COLOR_TAB_BR_FP_FIELD			clr_tab
#define TYPE_ACOLOR_TAB_BR_FP_FIELD			aclr_tab
#define TYPE_FPVALUE_TAB_BR_FP_FIELD		fpv_tab
#define TYPE_VALUE_TAB_BR_FP_FIELD			v_tab
#define TYPE_DWORD_TAB_BR_FP_FIELD			d_tab
#define TYPE_bool_TAB_BR_FP_FIELD			b_tab
#define TYPE_INTPTR_TAB_BR_FP_FIELD			intptr_tab
#define TYPE_INT64_TAB_BR_FP_FIELD			i64_tab
#define TYPE_DOUBLE_TAB_BR_FP_FIELD			dbl_tab

// by-value fields
#define TYPE_RGBA_BV_FP_FIELD				p	
#define TYPE_POINT3_BV_FP_FIELD				p	
#define TYPE_FRGBA_BV_FP_FIELD				p4	
#define TYPE_POINT4_BV_FP_FIELD				p4	
#define TYPE_HSV_BV_FP_FIELD				p
#define TYPE_BITMAP_BV_FP_FIELD				bm
#define TYPE_MATRIX3_BV_FP_FIELD			m
#define TYPE_ANGAXIS_BV_FP_FIELD			aa
#define TYPE_QUAT_BV_FP_FIELD				q
#define TYPE_BITARRAY_BV_FP_FIELD			bits
#define TYPE_RAY_BV_FP_FIELD				ray
#define TYPE_POINT2_BV_FP_FIELD				p2
#define TYPE_MESH_BV_FP_FIELD				msh
#define TYPE_INTERVAL_BV_FP_FIELD			intvl
#define TYPE_POINT_BV_FP_FIELD				pt
#define TYPE_TSTR_BV_FP_FIELD				tstr
#define TYPE_COLOR_BV_FP_FIELD				clr
#define TYPE_ACOLOR_BV_FP_FIELD				aclr
#define TYPE_FPVALUE_BV_FP_FIELD			fpv
#define TYPE_CLASS_BV_FP_FIELD				cd

// by-val Tab<> fields
#define TYPE_FLOAT_TAB_BV_FP_FIELD			f_tab
#define TYPE_INT_TAB_BV_FP_FIELD			i_tab
#define TYPE_RGBA_TAB_BV_FP_FIELD			p_tab
#define TYPE_POINT3_TAB_BV_FP_FIELD			p_tab
#define TYPE_FRGBA_TAB_BV_FP_FIELD			p4_tab
#define TYPE_POINT4_TAB_BV_FP_FIELD			p4_tab
#define TYPE_BOOL_TAB_BV_FP_FIELD			i_tab
#define TYPE_ANGLE_TAB_BV_FP_FIELD			f_tab
#define TYPE_PCNT_FRAC_TAB_BV_FP_FIELD		f_tab
#define TYPE_WORLD_TAB_BV_FP_FIELD			f_tab
#define TYPE_STRING_TAB_BV_FP_FIELD			s_tab
#define TYPE_FILENAME_TAB_BV_FP_FIELD		s_tab
#define TYPE_HSV_TAB_BV_FP_FIELD			p_tab
#define TYPE_COLOR_CHANNEL_TAB_BV_FP_FIELD	f_tab
#define TYPE_TIMEVALUE_TAB_BV_FP_FIELD		i_tab
#define TYPE_RADIOBTN_INDEX_TAB_BV_FP_FIELD i_tab
#define TYPE_MTL_TAB_BV_FP_FIELD			mtl_tab
#define TYPE_TEXMAP_TAB_BV_FP_FIELD			tex_tab
#define TYPE_BITMAP_TAB_BV_FP_FIELD			bm_tab
#define TYPE_INODE_TAB_BV_FP_FIELD			n_tab
#define TYPE_REFTARG_TAB_BV_FP_FIELD		r_tab
#define TYPE_INDEX_TAB_BV_FP_FIELD			i_tab
#define TYPE_ENUM_TAB_BV_FP_FIELD			i_tab
#define TYPE_MATRIX3_TAB_BV_FP_FIELD		m3_tab
#define TYPE_ANGAXIS_TAB_BV_FP_FIELD		aa_tab
#define TYPE_QUAT_TAB_BV_FP_FIELD			q_tab
#define TYPE_BITARRAY_TAB_BV_FP_FIELD		bits_tab
#define TYPE_CLASS_TAB_BV_FP_FIELD			cd_tab
#define TYPE_RAY_TAB_BV_FP_FIELD			ray_tab
#define TYPE_POINT2_TAB_BV_FP_FIELD			p2_tab
#define TYPE_MESH_TAB_BV_FP_FIELD			msh_tab
#define TYPE_OBJECT_TAB_BV_FP_FIELD			obj_tab
#define TYPE_CONTROL_TAB_BV_FP_FIELD		ctrl_tab
#define TYPE_INTERVAL_TAB_BV_FP_FIELD		intvl_tab
#define TYPE_POINT_TAB_BV_FP_FIELD			pt_tab
#define TYPE_HWND_TAB_BV_FP_FIELD			hwnd_tab
#define TYPE_TSTR_TAB_BV_FP_FIELD			tstr_tab
#define TYPE_IOBJECT_TAB_BV_FP_FIELD		iobj_tab
#define TYPE_INTERFACE_TAB_BV_FP_FIELD		fpi_tab
#define TYPE_NAME_TAB_BV_FP_FIELD			s_tab
#define TYPE_COLOR_TAB_BV_FP_FIELD			clr_tab
#define TYPE_ACOLOR_TAB_BV_FP_FIELD			aclr_tab
#define TYPE_FPVALUE_TAB_BV_FP_FIELD		fpv_tab
#define TYPE_VALUE_TAB_BV_FP_FIELD			v_tab
#define TYPE_DWORD_TAB_BV_FP_FIELD			d_tab
#define TYPE_bool_TAB_BV_FP_FIELD			b_tab
#define TYPE_INTPTR_TAB_BV_FP_FIELD			intptr_tab
#define TYPE_INT64_TAB_BV_FP_FIELD			i64_tab
#define TYPE_DOUBLE_TAB_BV_FP_FIELD			dbl_tab

// field access macros...  

// base types, yield 'conventional' type passing conventions
//   ie, ints, floats, points, colors, 3D math types are passed as values, all 
//   others passed as pointers

#define TYPE_FLOAT_FIELD(_v)			(((_v).f))		
#define TYPE_INT_FIELD(_v)				(((_v).i))		
#define TYPE_RGBA_FIELD(_v)				(*((_v).p))	
#define TYPE_POINT3_FIELD(_v)			(*((_v).p))	
#define TYPE_FRGBA_FIELD(_v)			(*((_v).p4))	
#define TYPE_POINT4_FIELD(_v)			(*((_v).p4))	
#define TYPE_BOOL_FIELD(_v)				(((_v).i))		
#define TYPE_ANGLE_FIELD(_v)			(((_v).f))		
#define TYPE_PCNT_FRAC_FIELD(_v)		(((_v).f))	
#define TYPE_WORLD_FIELD(_v)			(((_v).f))
#define TYPE_STRING_FIELD(_v)			(((_v).s))
#define TYPE_FILENAME_FIELD(_v)			(((_v).s))
#define TYPE_HSV_FIELD(_v)				(*((_v).p))
#define TYPE_COLOR_CHANNEL_FIELD(_v)	(((_v).f))
#define TYPE_TIMEVALUE_FIELD(_v)		(((_v).i))
#define TYPE_RADIOBTN_INDEX_FIELD(_v)	(((_v).i))
#define TYPE_MTL_FIELD(_v)				(((_v).mtl))
#define TYPE_TEXMAP_FIELD(_v)			(((_v).tex))
#define TYPE_BITMAP_FIELD(_v)			(((_v).bm))
#define TYPE_INODE_FIELD(_v)			(((_v).n))
#define TYPE_REFTARG_FIELD(_v)			(((_v).r))
#define TYPE_INDEX_FIELD(_v)			(((_v).i))
#define TYPE_ENUM_FIELD(_v)				(((_v).i))
#define TYPE_MATRIX3_FIELD(_v)			(*((_v).m))
#define TYPE_ANGAXIS_FIELD(_v)			(*((_v).aa))
#define TYPE_QUAT_FIELD(_v)				(*((_v).q))
#define TYPE_BITARRAY_FIELD(_v)			(((_v).bits))
#define TYPE_CLASS_FIELD(_v)			(((_v).cd))
#define TYPE_RAY_FIELD(_v)				(*((_v).ray))
#define TYPE_POINT2_FIELD(_v)			(*((_v).p2))
#define TYPE_MESH_FIELD(_v)				(((_v).msh))
#define TYPE_OBJECT_FIELD(_v)			(((_v).obj))
#define TYPE_CONTROL_FIELD(_v)			(((_v).ctrl))
#define TYPE_INTERVAL_FIELD(_v)			(*((_v).intvl))
#define TYPE_POINT_FIELD(_v)			(*((_v).pt))
#define TYPE_TSTR_FIELD(_v)				(*((_v).tstr))
#define TYPE_IOBJECT_FIELD(_v)			(((_v).iobj))
#define TYPE_INTERFACE_FIELD(_v)		(((_v).fpi))
#define TYPE_HWND_FIELD(_v)				(((_v).hwnd))
#define TYPE_NAME_FIELD(_v)				(((_v).s))
#define TYPE_COLOR_FIELD(_v)			(((_v).clr))
#define TYPE_ACOLOR_FIELD(_v)			(((_v).aclr))
#define TYPE_FPVALUE_FIELD(_v)			(((_v).fpv))
#define TYPE_VALUE_FIELD(_v)			(((_v).v))
#define TYPE_DWORD_FIELD(_v)			(((_v).d))
#define TYPE_bool_FIELD(_v)				(((_v).b))
#define TYPE_INTPTR_FIELD(_v)			(((_v).intptr))		
#define TYPE_INT64_FIELD(_v)			(((_v).i64))		
#define TYPE_DOUBLE_FIELD(_v)			(((_v).dbl))		

// all Tab<> types passed by pointer

#define TYPE_FLOAT_TAB_FIELD(_v)			(((_v).f_tab))
#define TYPE_INT_TAB_FIELD(_v)				(((_v).i_tab))
#define TYPE_RGBA_TAB_FIELD(_v)				(((_v).p_tab))
#define TYPE_POINT3_TAB_FIELD(_v)			(((_v).p_tab))
#define TYPE_FRGBA_TAB_FIELD(_v)			(((_v).p4_tab))
#define TYPE_POINT4_TAB_FIELD(_v)			(((_v).p4_tab))
#define TYPE_BOOL_TAB_FIELD(_v)				(((_v).i_tab))
#define TYPE_ANGLE_TAB_FIELD(_v)			(((_v).f_tab))
#define TYPE_PCNT_FRAC_TAB_FIELD(_v)		(((_v).f_tab))
#define TYPE_WORLD_TAB_FIELD(_v)			(((_v).f_tab))
#define TYPE_STRING_TAB_FIELD(_v)			(((_v).s_tab))
#define TYPE_FILENAME_TAB_FIELD(_v)			(((_v).s_tab))
#define TYPE_HSV_TAB_FIELD(_v)				(((_v).p_tab))
#define TYPE_COLOR_CHANNEL_TAB_FIELD(_v)	(((_v).f_tab))
#define TYPE_TIMEVALUE_TAB_FIELD(_v)		(((_v).i_tab))
#define TYPE_RADIOBTN_INDEX_TAB_FIELD(_v)	(((_v).i_tab))
#define TYPE_MTL_TAB_FIELD(_v)				(((_v).mtl_tab))
#define TYPE_TEXMAP_TAB_FIELD(_v)			(((_v).tex_tab))
#define TYPE_BITMAP_TAB_FIELD(_v)			(((_v).bm_tab))
#define TYPE_INODE_TAB_FIELD(_v)			(((_v).n_tab))
#define TYPE_REFTARG_TAB_FIELD(_v)			(((_v).r_tab))
#define TYPE_INDEX_TAB_FIELD(_v)			(((_v).i_tab))
#define TYPE_ENUM_TAB_FIELD(_v)				(((_v).i_tab))
#define TYPE_MATRIX3_TAB_FIELD(_v)			(((_v).m3_tab))
#define TYPE_ANGAXIS_TAB_FIELD(_v)			(((_v).aa_tab))
#define TYPE_QUAT_TAB_FIELD(_v)				(((_v).q_tab))
#define TYPE_BITARRAY_TAB_FIELD(_v)			(((_v).bits_tab))
#define TYPE_CLASS_TAB_FIELD(_v)			(((_v).cd_tab))
#define TYPE_RAY_TAB_FIELD(_v)				(((_v).ray_tab))
#define TYPE_POINT2_TAB_FIELD(_v)			(((_v).p2_tab))
#define TYPE_MESH_TAB_FIELD(_v)				(((_v).msh_tab))
#define TYPE_OBJECT_TAB_FIELD(_v)			(((_v).obj_tab))
#define TYPE_CONTROL_TAB_FIELD(_v)			(((_v).ctrl_tab))
#define TYPE_INTERVAL_TAB_FIELD(_v)			(((_v).intvl_tab))
#define TYPE_POINT_TAB_FIELD(_v)			(((_v).pt_tab))
#define TYPE_TSTRT_TAB_FIELD(_v)			(((_v).tstr_tab))
#define TYPE_IOBJECT_TAB_FIELD(_v)			(((_v).iobj_tab))
#define TYPE_INTERFACE_TAB_FIELD(_v)		(((_v).fpi_tab))
#define TYPE_HWND_TAB_FIELD(_v)				(((_v).hwnd_tab))
#define TYPE_NAME_TAB_FIELD(_v)				(((_v).s_tab))
#define TYPE_COLOR_TAB_FIELD(_v)			(((_v).clr_tab))
#define TYPE_ACOLOR_TAB_FIELD(_v)			(((_v).aclr_tab))
#define TYPE_FPVALUE_TAB_FIELD(_v)			(((_v).fpv_tab))
#define TYPE_VALUE_TAB_FIELD(_v)			(((_v).v_tab))
#define TYPE_DWORD_TAB_FIELD(_v)			(((_v).d_tab))
#define TYPE_bool_TAB_FIELD(_v)				(((_v).b_tab))
#define TYPE_INTPTR_TAB_FIELD(_v)			(((_v).intptr_tab))
#define TYPE_INT64_TAB_FIELD(_v)			(((_v).i64_tab))
#define TYPE_DOUBLE_TAB_FIELD(_v)			(((_v).dbl_tab))

// the following variants all assume a pointer is used as the source of the 
// param, but deliver it to the called interface function in the given mode, 
//  _BP -> a pointer, eg, int* x
//  _BR -> a reference, eg, int& x
//  _BV -> a dereferenced value, only for pointer-based types, derived by *fpvalue.ptr

// * (pointer) field access macros
// pass by-pointer types for int & float types, implies * parameters, int* & float* are passed via .ptr fields, only for FnPub use
#define TYPE_FLOAT_BP_FIELD(_v)				(((_v).fptr))		
#define TYPE_INT_BP_FIELD(_v)				(((_v).iptr))		
#define TYPE_BOOL_BP_FIELD(_v)				(((_v).iptr))		
#define TYPE_ANGLE_BP_FIELD(_v)				(((_v).fptr))		
#define TYPE_PCNT_FRAC_BP_FIELD(_v)			(((_v).fptr))	
#define TYPE_WORLD_BP_FIELD(_v)				(((_v).fptr))
#define TYPE_COLOR_CHANNEL_BP_FIELD(_v)		(((_v).fptr))
#define TYPE_TIMEVALUE_BP_FIELD(_v)			(((_v).iptr))
#define TYPE_RADIOBTN_INDEX_BP_FIELD(_v)	(((_v).iptr))
#define TYPE_INDEX_BP_FIELD(_v)				(((_v).iptr))
#define TYPE_ENUM_BP_FIELD(_v)				(((_v).iptr))
#define TYPE_DWORD_BP_FIELD(_v)				(((_v).dptr))
#define TYPE_bool_BP_FIELD(_v)				(((_v).bptr))
#define TYPE_INTPTR_BP_FIELD(_v)			(((_v).intptrptr))		
#define TYPE_INT64_BP_FIELD(_v)				(((_v).i64ptr))		
#define TYPE_DOUBLE_BP_FIELD(_v)			(((_v).dblptr))		
// there are no specific by-pointer Tab<> types, all Tab<> types are by-pointer by default

// & (reference) field access macros
// pass by-ref types, implies & parameters, int& & float& are passed via .ptr fields, only for FnPub use
#define TYPE_FLOAT_BR_FIELD(_v)				(*((_v).fptr))		
#define TYPE_INT_BR_FIELD(_v)				(*((_v).iptr))		
#define TYPE_RGBA_BR_FIELD(_v)				(*((_v).p))	
#define TYPE_POINT3_BR_FIELD(_v)			(*((_v).p))	
#define TYPE_FRGBA_BR_FIELD(_v)				(*((_v).p4))	
#define TYPE_POINT4_BR_FIELD(_v)			(*((_v).p4))	
#define TYPE_BOOL_BR_FIELD(_v)				(*((_v).iptr))		
#define TYPE_ANGLE_BR_FIELD(_v)				(*((_v).fptr))		
#define TYPE_PCNT_FRAC_BR_FIELD(_v)			(*((_v).fptr))	
#define TYPE_WORLD_BR_FIELD(_v)				(*((_v).fptr))
#define TYPE_HSV_BR_FIELD(_v)				(*((_v).p))
#define TYPE_COLOR_CHANNEL_BR_FIELD(_v)		(*((_v).f))
#define TYPE_TIMEVALUE_BR_FIELD(_v)			(*((_v).iptr))
#define TYPE_RADIOBTN_INDEX_BR_FIELD(_v)	(*((_v).iptr))
#define TYPE_BITMAP_BR_FIELD(_v)			(*((_v).bm))
#define TYPE_INDEX_BR_FIELD(_v)				(*((_v).iptr))
#define TYPE_ENUM_BR_FIELD(_v)				(*((_v).iptr))
#define TYPE_REFTARG_BR_FIELD(_v)			(*((_v).r))
#define TYPE_MATRIX3_BR_FIELD(_v)			(*((_v).m))
#define TYPE_ANGAXIS_BR_FIELD(_v)			(*((_v).aa))
#define TYPE_QUAT_BR_FIELD(_v)				(*((_v).q))
#define TYPE_BITARRAY_BR_FIELD(_v)			(*((_v).bits))
#define TYPE_RAY_BR_FIELD(_v)				(*((_v).ray))
#define TYPE_POINT2_BR_FIELD(_v)			(*((_v).p2))
#define TYPE_MESH_BR_FIELD(_v)				(*((_v).msh))
#define TYPE_INTERVAL_BR_FIELD(_v)			(*((_v).intvl))
#define TYPE_POINT_BR_FIELD(_v)				(*((_v).pt))
#define TYPE_TSTR_BR_FIELD(_v)				(*((_v).tstr))
#define TYPE_COLOR_BR_FIELD(_v)				(*((_v).clr))
#define TYPE_ACOLOR_BR_FIELD(_v)			(*((_v).aclr))
#define TYPE_FPVALUE_BR_FIELD(_v)			(*((_v).fpv))
#define TYPE_DWORD_BR_FIELD(_v)				(*((_v).d))
#define TYPE_bool_BR_FIELD(_v)				(*((_v).b))
#define TYPE_INTPTR_BR_FIELD(_v)			(*((_v).intptrptr))		
#define TYPE_INT64_BR_FIELD(_v)				(*((_v).i64ptr))		
#define TYPE_DOUBLE_BR_FIELD(_v)			(*((_v).dblptr))		

// refs to Tab<>s

#define TYPE_FLOAT_TAB_BR_FIELD(_v)			(*((_v).f_tab))
#define TYPE_INT_TAB_BR_FIELD(_v)			(*((_v).i_tab))
#define TYPE_RGBA_TAB_BR_FIELD(_v)			(*((_v).p_tab))
#define TYPE_POINT3_TAB_BR_FIELD(_v)		(*((_v).p_tab))
#define TYPE_FRGBA_TAB_BR_FIELD(_v)			(*((_v).p4_tab))
#define TYPE_POINT4_TAB_BR_FIELD(_v)		(*((_v).p4_tab))
#define TYPE_BOOL_TAB_BR_FIELD(_v)			(*((_v).i_tab))
#define TYPE_ANGLE_TAB_BR_FIELD(_v)			(*((_v).f_tab))
#define TYPE_PCNT_FRAC_TAB_BR_FIELD(_v)		(*((_v).f_tab))
#define TYPE_WORLD_TAB_BR_FIELD(_v)			(*((_v).f_tab))
#define TYPE_STRING_TAB_BR_FIELD(_v)		(*((_v).s_tab))
#define TYPE_FILENAME_TAB_BR_FIELD(_v)		(*((_v).s_tab))
#define TYPE_HSV_TAB_BR_FIELD(_v)			(*((_v).p_tab))
#define TYPE_COLOR_CHANNEL_TAB_BR_FIELD(_v)	(*((_v).f_tab))
#define TYPE_TIMEVALUE_TAB_BR_FIELD(_v)		(*((_v).i_tab))
#define TYPE_RADIOBTN_INDEX_TAB_BR_FIELD(_v) (*((_v).i_tab))
#define TYPE_MTL_TAB_BR_FIELD(_v)			(*((_v).mtl_tab))
#define TYPE_TEXMAP_TAB_BR_FIELD(_v)		(*((_v).tex_tab))
#define TYPE_BITMAP_TAB_BR_FIELD(_v)		(*((_v).bm_tab))
#define TYPE_INODE_TAB_BR_FIELD(_v)			(*((_v).n_tab))
#define TYPE_REFTARG_TAB_BR_FIELD(_v)		(*((_v).r_tab))
#define TYPE_INDEX_TAB_BR_FIELD(_v)			(*((_v).i_tab))
#define TYPE_ENUM_TAB_BR_FIELD(_v)			(*((_v).i_tab))
#define TYPE_MATRIX3_TAB_BR_FIELD(_v)		(*((_v).m3_tab))
#define TYPE_ANGAXIS_TAB_BR_FIELD(_v)		(*((_v).aa_tab))
#define TYPE_QUAT_TAB_BR_FIELD(_v)			(*((_v).q_tab))
#define TYPE_BITARRAY_TAB_BR_FIELD(_v)		(*((_v).bits_tab))
#define TYPE_CLASS_TAB_BR_FIELD(_v)			(*((_v).cd_tab))
#define TYPE_RAY_TAB_BR_FIELD(_v)			(*((_v).ray_tab))
#define TYPE_POINT2_TAB_BR_FIELD(_v)		(*((_v).p2_tab))
#define TYPE_MESH_TAB_BR_FIELD(_v)			(*((_v).msh_tab))
#define TYPE_OBJECT_TAB_BR_FIELD(_v)		(*((_v).obj_tab))
#define TYPE_CONTROL_TAB_BR_FIELD(_v)		(*((_v).ctrl_tab))
#define TYPE_INTERVAL_TAB_BR_FIELD(_v)		(*((_v).intvl_tab))
#define TYPE_POINT_TAB_BR_FIELD(_v)			(*((_v).pt_tab))
#define TYPE_HWND_TAB_BR_FIELD(_v)			(*((_v).hwnd_tab))
#define TYPE_TSTR_TAB_BR_FIELD(_v)			(*((_v).tstr_tab))
#define TYPE_IOBJECT_TAB_BR_FIELD(_v)		(*((_v).iobj_tab))
#define TYPE_INTERFACE_TAB_BR_FIELD(_v)		(*((_v).fpi_tab))
#define TYPE_NAME_TAB_BR_FIELD(_v)			(*((_v).s_tab))
#define TYPE_COLOR_TAB_BR_FIELD(_v)			(*((_v).clr_tab))
#define TYPE_ACOLOR_TAB_BR_FIELD(_v)		(*((_v).aclr_tab))
#define TYPE_FPVALUE_TAB_BR_FIELD(_v)		(*((_v).fpv_tab))
#define TYPE_VALUE_TAB_BR_FIELD(_v)			(*((_v).v_tab))
#define TYPE_DWORD_TAB_BR_FIELD(_v)			(*((_v).d_tab))
#define TYPE_bool_TAB_BR_FIELD(_v)			(*((_v).b_tab))
#define TYPE_INTPTR_TAB_BR_FIELD(_v)		(*((_v).intptr_tab))
#define TYPE_INT64_TAB_BR_FIELD(_v)			(*((_v).i64_tab))
#define TYPE_DOUBLE_TAB_BR_FIELD(_v)		(*((_v).dbl_tab))
	
// by value field access macros
// pass by-value types, implies dereferencing the (meaningful) pointer-based values, only for FnPub use
#define TYPE_RGBA_BV_FIELD(_v)				(*((_v).p))	
#define TYPE_POINT3_BV_FIELD(_v)			(*((_v).p))	
#define TYPE_HSV_BV_FIELD(_v)				(*((_v).p))
#define TYPE_FRGBA_BV_FIELD(_v)				(*((_v).p4))	
#define TYPE_POINT4_BV_FIELD(_v)			(*((_v).p4))	
#define TYPE_BITMAP_BV_FIELD(_v)			(*((_v).bm))
#define TYPE_MATRIX3_BV_FIELD(_v)			(*((_v).m))
#define TYPE_ANGAXIS_BV_FIELD(_v)			(*((_v).aa))
#define TYPE_QUAT_BV_FIELD(_v)				(*((_v).q))
#define TYPE_BITARRAY_BV_FIELD(_v)			(*((_v).bits))
#define TYPE_RAY_BV_FIELD(_v)				(*((_v).ray))
#define TYPE_POINT2_BV_FIELD(_v)			(*((_v).p2))
#define TYPE_MESH_BV_FIELD(_v)				(*((_v).msh))
#define TYPE_INTERVAL_BV_FIELD(_v)			(*((_v).intvl))
#define TYPE_POINT_BV_FIELD(_v)				(*((_v).pt))
#define TYPE_TSTR_BV_FIELD(_v)				(*((_v).tstr))
#define TYPE_COLOR_BV_FIELD(_v)				(*((_v).clr))
#define TYPE_ACOLOR_BV_FIELD(_v)			(*((_v).aclr))
#define TYPE_FPVALUE_BV_FIELD(_v)			(*((_v).fpv))
#define TYPE_CLASS_BV_FIELD(_v)				(*((_v).cd))

// pass by-val Tab<> types
#define TYPE_FLOAT_TAB_BV_FIELD(_v)			(*((_v).f_tab))
#define TYPE_INT_TAB_BV_FIELD(_v)			(*((_v).i_tab))
#define TYPE_RGBA_TAB_BV_FIELD(_v)			(*((_v).p_tab))
#define TYPE_POINT3_TAB_BV_FIELD(_v)		(*((_v).p_tab))
#define TYPE_FRGBA_TAB_BV_FIELD(_v)			(*((_v).p4_tab))
#define TYPE_POINT4_TAB_BV_FIELD(_v)		(*((_v).p4_tab))
#define TYPE_BOOL_TAB_BV_FIELD(_v)			(*((_v).i_tab))
#define TYPE_ANGLE_TAB_BV_FIELD(_v)			(*((_v).f_tab))
#define TYPE_PCNT_FRAC_TAB_BV_FIELD(_v)		(*((_v).f_tab))
#define TYPE_WORLD_TAB_BV_FIELD(_v)			(*((_v).f_tab))
#define TYPE_STRING_TAB_BV_FIELD(_v)		(*((_v).s_tab))
#define TYPE_FILENAME_TAB_BV_FIELD(_v)		(*((_v).s_tab))
#define TYPE_HSV_TAB_BV_FIELD(_v)			(*((_v).p_tab))
#define TYPE_COLOR_CHANNEL_TAB_BV_FIELD(_v)	(*((_v).f_tab))
#define TYPE_TIMEVALUE_TAB_BV_FIELD(_v)		(*((_v).i_tab))
#define TYPE_RADIOBTN_INDEX_TAB_BV_FIELD(_v) (*((_v).i_tab))
#define TYPE_MTL_TAB_BV_FIELD(_v)			(*((_v).mtl_tab))
#define TYPE_TEXMAP_TAB_BV_FIELD(_v)		(*((_v).tex_tab))
#define TYPE_BITMAP_TAB_BV_FIELD(_v)		(*((_v).bm_tab))
#define TYPE_INODE_TAB_BV_FIELD(_v)			(*((_v).n_tab))
#define TYPE_REFTARG_TAB_BV_FIELD(_v)		(*((_v).r_tab))
#define TYPE_INDEX_TAB_BV_FIELD(_v)			(*((_v).i_tab))
#define TYPE_ENUM_TAB_BV_FIELD(_v)			(*((_v).i_tab))
#define TYPE_MATRIX3_TAB_BV_FIELD(_v)		(*((_v).m3_tab))
#define TYPE_ANGAXIS_TAB_BV_FIELD(_v)		(*((_v).aa_tab))
#define TYPE_QUAT_TAB_BV_FIELD(_v)			(*((_v).q_tab))
#define TYPE_BITARRAY_TAB_BV_FIELD(_v)		(*((_v).bits_tab))
#define TYPE_CLASS_TAB_BV_FIELD(_v)			(*((_v).cd_tab))
#define TYPE_RAY_TAB_BV_FIELD(_v)			(*((_v).ray_tab))
#define TYPE_POINT2_TAB_BV_FIELD(_v)		(*((_v).p2_tab))
#define TYPE_MESH_TAB_BV_FIELD(_v)			(*((_v).msh_tab))
#define TYPE_OBJECT_TAB_BV_FIELD(_v)		(*((_v).obj_tab))
#define TYPE_CONTROL_TAB_BV_FIELD(_v)		(*((_v).ctrl_tab))
#define TYPE_INTERVAL_TAB_BV_FIELD(_v)		(*((_v).intvl_tab))
#define TYPE_POINT_TAB_BV_FIELD(_v)			(*((_v).pt_tab))
#define TYPE_HWND_TAB_BV_FIELD(_v)			(*((_v).hwnd_tab))
#define TYPE_TSTR_TAB_BV_FIELD(_v)			(*((_v).tstr_tab))
#define TYPE_IOBJECT_TAB_BV_FIELD(_v)		(*((_v).iobj_tab))
#define TYPE_INTERFACE_TAB_BV_FIELD(_v)		(*((_v).fpi_tab))
#define TYPE_NAME_TAB_BV_FIELD(_v)			(*((_v).s_tab))
#define TYPE_COLOR_TAB_BV_FIELD(_v)			(*((_v).clr_tab))
#define TYPE_ACOLOR_TAB_BV_FIELD(_v)		(*((_v).aclr_tab))
#define TYPE_FPVALUE_TAB_BV_FIELD(_v)		(*((_v).fpv_tab))
#define TYPE_VALUE_TAB_BV_FIELD(_v)			(*((_v).v_tab))
#define TYPE_DWORD_TAB_BV_FIELD(_v)			(*((_v).d_tab))
#define TYPE_bool_TAB_BV_FIELD(_v)			(*((_v).b_tab))
#define TYPE_INTPTR_TAB_BV_FIELD(_v)		(*((_v).intptr_tab))
#define TYPE_INT64_TAB_BV_FIELD(_v)			(*((_v).i64_tab))
#define TYPE_DOUBLE_TAB_BV_FIELD(_v)		(*((_v).dbl_tab))

// --- type result operators ----------------------

// used to generate an rvalue from the type's corresponding C++ type
// for assignment to the type's carrying field in FPValue.
// mostly empty, used by BY_REF & BY_VAL types to get pointers, since these
// types are actualy carried by pointer fields

// base types
#define TYPE_FLOAT_RSLT				
#define TYPE_INT_RSLT				
#define TYPE_RGBA_RSLT				
#define TYPE_POINT3_RSLT			
#define TYPE_FRGBA_RSLT				
#define TYPE_POINT4_RSLT			
#define TYPE_BOOL_RSLT				
#define TYPE_ANGLE_RSLT				
#define TYPE_PCNT_FRAC_RSLT			
#define TYPE_WORLD_RSLT				
#define TYPE_STRING_RSLT			
#define TYPE_FILENAME_RSLT			
#define TYPE_HSV_RSLT				
#define TYPE_COLOR_CHANNEL_RSLT		
#define TYPE_TIMEVALUE_RSLT			
#define TYPE_RADIOBTN_INDEX_RSLT	
#define TYPE_MTL_RSLT				
#define TYPE_TEXMAP_RSLT			
#define TYPE_BITMAP_RSLT			
#define TYPE_INODE_RSLT				
#define TYPE_REFTARG_RSLT			
#define TYPE_INDEX_RSLT				
#define TYPE_ENUM_RSLT				
#define TYPE_MATRIX3_RSLT			
#define TYPE_VOID_RSLT				
#define TYPE_INTERVAL_RSLT			
#define TYPE_ANGAXIS_RSLT			
#define TYPE_QUAT_RSLT				
#define TYPE_RAY_RSLT				
#define TYPE_POINT2_RSLT			
#define TYPE_BITARRAY_RSLT			
#define TYPE_CLASS_RSLT				
#define TYPE_MESH_RSLT				
#define TYPE_OBJECT_RSLT			
#define TYPE_CONTROL_RSLT			
#define TYPE_POINT_RSLT				
#define TYPE_TSTR_RSLT				
#define TYPE_IOBJECT_RSLT			
#define TYPE_INTERFACE_RSLT			
#define TYPE_HWND_RSLT				
#define TYPE_NAME_RSLT				
#define TYPE_COLOR_RSLT				
#define TYPE_ACOLOR_RSLT				
#define TYPE_FPVALUE_RSLT				
#define TYPE_VALUE_RSLT				
#define TYPE_DWORD_RSLT				
#define TYPE_bool_RSLT				
#define TYPE_INTPTR_RSLT				
#define TYPE_INT64_RSLT				
#define TYPE_DOUBLE_RSLT				

// Tab<>s of the above...

#define TYPE_FLOAT_TAB_RSLT				
#define TYPE_INT_TAB_RSLT				
#define TYPE_RGBA_TAB_RSLT				
#define TYPE_POINT3_TAB_RSLT			
#define TYPE_FRGBA_TAB_RSLT				
#define TYPE_POINT4_TAB_RSLT			
#define TYPE_BOOL_TAB_RSLT				
#define TYPE_ANGLE_TAB_RSLT				
#define TYPE_PCNT_FRAC_TAB_RSLT			
#define TYPE_WORLD_TAB_RSLT				
#define TYPE_STRING_TAB_RSLT			
#define TYPE_FILENAME_TAB_RSLT			
#define TYPE_HSV_TAB_RSLT				
#define TYPE_COLOR_CHANNEL_TAB_RSLT		
#define TYPE_TIMEVALUE_TAB_RSLT			
#define TYPE_RADIOBTN_INDEX_TAB_RSLT	
#define TYPE_MTL_TAB_RSLT				
#define TYPE_TEXMAP_TAB_RSLT			
#define TYPE_BITMAP_TAB_RSLT			
#define TYPE_INODE_TAB_RSLT				
#define TYPE_REFTARG_TAB_RSLT			
#define TYPE_INDEX_TAB_RSLT				
#define TYPE_ENUM_TAB_RSLT				
#define TYPE_MATRIX3_TAB_RSLT			
#define TYPE_INTERVAL_TAB_RSLT			
#define TYPE_ANGAXIS_TAB_RSLT			
#define TYPE_QUAT_TAB_RSLT				
#define TYPE_RAY_TAB_RSLT				
#define TYPE_POINT2_TAB_RSLT			
#define TYPE_BITARRAY_TAB_RSLT			
#define TYPE_CLASS_TAB_RSLT				
#define TYPE_MESH_TAB_RSLT				
#define TYPE_OBJECT_TAB_RSLT			
#define TYPE_CONTROL_TAB_RSLT			
#define TYPE_POINT_TAB_RSLT				
#define TYPE_TSTR_TAB_RSLT				
#define TYPE_IOBJECT_TAB_RSLT			
#define TYPE_INTERFACE_TAB_RSLT			
#define TYPE_HWND_TAB_RSLT				
#define TYPE_NAME_TAB_RSLT				
#define TYPE_COLOR_TAB_RSLT				
#define TYPE_ACOLOR_TAB_RSLT				
#define TYPE_FPVALUE_TAB_RSLT				
#define TYPE_VALUE_TAB_RSLT				
#define TYPE_DWORD_TAB_RSLT				
#define TYPE_bool_TAB_RSLT				
#define TYPE_INTPTR_TAB_RSLT				
#define TYPE_INT64_TAB_RSLT				
#define TYPE_DOUBLE_TAB_RSLT				

// by-pointer
//  foo*  = 
#define TYPE_FLOAT_BP_RSLT				
#define TYPE_INT_BP_RSLT				
#define TYPE_BOOL_BP_RSLT				
#define TYPE_ANGLE_BP_RSLT				
#define TYPE_PCNT_FRAC_BP_RSLT			
#define TYPE_WORLD_BP_RSLT				
#define TYPE_COLOR_CHANNEL_BP_RSLT		
#define TYPE_TIMEVALUE_BP_RSLT			
#define TYPE_RADIOBTN_INDEX_BP_RSLT		
#define TYPE_INDEX_BP_RSLT				
#define TYPE_ENUM_BP_RSLT				
#define TYPE_DWORD_BP_RSLT				
#define TYPE_bool_BP_RSLT				
#define TYPE_INTPTR_BP_RSLT				
#define TYPE_INT64_BP_RSLT				
#define TYPE_DOUBLE_BP_RSLT				

// by-reference 
#define TYPE_FLOAT_BR_RSLT				&	
#define TYPE_INT_BR_RSLT				&
#define TYPE_RGBA_BR_RSLT				&
#define TYPE_POINT3_BR_RSLT				&
#define TYPE_FRGBA_BR_RSLT				&
#define TYPE_POINT4_BR_RSLT				&
#define TYPE_BOOL_BR_RSLT				&
#define TYPE_ANGLE_BR_RSLT				&
#define TYPE_PCNT_FRAC_BR_RSLT			&
#define TYPE_WORLD_BR_RSLT				&
#define TYPE_HSV_BR_RSLT				&
#define TYPE_COLOR_CHANNEL_BR_RSLT		&
#define TYPE_TIMEVALUE_BR_RSLT			&
#define TYPE_RADIOBTN_INDEX_BR_RSLT		&
#define TYPE_BITMAP_BR_RSLT				&
#define TYPE_INDEX_BR_RSLT				&
#define TYPE_ENUM_BR_RSLT				&
#define TYPE_REFTARG_BR_RSLT			&
#define TYPE_MATRIX3_BR_RSLT			&
#define TYPE_ANGAXIS_BR_RSLT			&
#define TYPE_QUAT_BR_RSLT				&
#define TYPE_BITARRAY_BR_RSLT			&
#define TYPE_RAY_BR_RSLT				&
#define TYPE_POINT2_BR_RSLT				&
#define TYPE_MESH_BR_RSLT				&
#define TYPE_INTERVAL_BR_RSLT			&
#define TYPE_POINT_BR_RSLT				&
#define TYPE_TSTR_BR_RSLT				&
#define TYPE_COLOR_BR_RSLT				&
#define TYPE_ACOLOR_BR_RSLT				&
#define TYPE_FPVALUE_BR_RSLT			&
#define TYPE_DWORD_BR_RSLT				&
#define TYPE_bool_BR_RSLT				&
#define TYPE_INTPTR_BR_RSLT				&
#define TYPE_INT64_BR_RSLT				&
#define TYPE_DOUBLE_BR_RSLT				&	

// Tab<> by-reference &
#define TYPE_FLOAT_TAB_BR_RSLT			&
#define TYPE_INT_TAB_BR_RSLT			&
#define TYPE_RGBA_TAB_BR_RSLT			&
#define TYPE_POINT3_TAB_BR_RSLT			&
#define TYPE_FRGBA_TAB_BR_RSLT			&
#define TYPE_POINT4_TAB_BR_RSLT			&
#define TYPE_BOOL_TAB_BR_RSLT			&
#define TYPE_ANGLE_TAB_BR_RSLT			&
#define TYPE_PCNT_FRAC_TAB_BR_RSLT		&
#define TYPE_WORLD_TAB_BR_RSLT			&
#define TYPE_STRING_TAB_BR_RSLT			&
#define TYPE_FILENAME_TAB_BR_RSLT		&
#define TYPE_HSV_TAB_BR_RSLT			&
#define TYPE_COLOR_CHANNEL_TAB_BR_RSLT	&
#define TYPE_TIMEVALUE_TAB_BR_RSLT		&
#define TYPE_RADIOBTN_INDEX_TAB_BR_RSLT &
#define TYPE_MTL_TAB_BR_RSLT			&
#define TYPE_TEXMAP_TAB_BR_RSLT			&
#define TYPE_BITMAP_TAB_BR_RSLT			&
#define TYPE_INODE_TAB_BR_RSLT			&
#define TYPE_REFTARG_TAB_BR_RSLT		&
#define TYPE_INDEX_TAB_BR_RSLT			&
#define TYPE_ENUM_TAB_BR_RSLT			&
#define TYPE_MATRIX3_TAB_BR_RSLT		&
#define TYPE_ANGAXIS_TAB_BR_RSLT		&
#define TYPE_QUAT_TAB_BR_RSLT			&
#define TYPE_BITARRAY_TAB_BR_RSLT		&
#define TYPE_CLASS_TAB_BR_RSLT			&
#define TYPE_RAY_TAB_BR_RSLT			&
#define TYPE_POINT2_TAB_BR_RSLT			&
#define TYPE_MESH_TAB_BR_RSLT			&
#define TYPE_OBJECT_TAB_BR_RSLT			&
#define TYPE_CONTROL_TAB_BR_RSLT		&
#define TYPE_INTERVAL_TAB_BR_RSLT		&
#define TYPE_POINT_TAB_BR_RSLT			&
#define TYPE_HWND_TAB_BR_RSLT			&
#define TYPE_TSTR_TAB_BR_RSLT			&
#define TYPE_IOBJECT_TAB_BR_RSLT		&
#define TYPE_INTERFACE_TAB_BR_RSLT		&
#define TYPE_NAME_TAB_BR_RSLT			&
#define TYPE_COLOR_TAB_BR_RSLT			&
#define TYPE_ACOLOR_TAB_BR_RSLT			&
#define TYPE_FPVALUE_TAB_BR_RSLT		&
#define TYPE_VALUE_TAB_BR_RSLT			&
#define TYPE_DWORD_TAB_BR_RSLT			&
#define TYPE_bool_TAB_BR_RSLT			&
#define TYPE_INTPTR_TAB_BR_RSLT			&
#define TYPE_INT64_TAB_BR_RSLT			&
#define TYPE_DOUBLE_TAB_BR_RSLT			&

// by-value 
#define TYPE_RGBA_BV_RSLT				&
#define TYPE_POINT3_BV_RSLT				&
#define TYPE_HSV_BV_RSLT				&
#define TYPE_FRGBA_BV_RSLT				&
#define TYPE_POINT4_BV_RSLT				&
#define TYPE_BITMAP_BV_RSLT				&
#define TYPE_MATRIX3_BV_RSLT			&
#define TYPE_ANGAXIS_BV_RSLT			&
#define TYPE_QUAT_BV_RSLT				&
#define TYPE_BITARRAY_BV_RSLT			&
#define TYPE_RAY_BV_RSLT				&
#define TYPE_POINT2_BV_RSLT				&
#define TYPE_MESH_BV_RSLT				&
#define TYPE_INTERVAL_BV_RSLT			&
#define TYPE_POINT_BV_RSLT				&
#define TYPE_TSTR_BV_RSLT				&
#define TYPE_COLOR_BV_RSLT				&
#define TYPE_ACOLOR_BV_RSLT				&
#define TYPE_FPVALUE_BV_RSLT			&
#define TYPE_CLASS_BV_RSLT				&

// by-val Tab<> 
#define TYPE_FLOAT_TAB_BV_RSLT			&
#define TYPE_INT_TAB_BV_RSLT			&
#define TYPE_RGBA_TAB_BV_RSLT			&
#define TYPE_POINT3_TAB_BV_RSLT			&
#define TYPE_FRGBA_TAB_BV_RSLT			&
#define TYPE_POINT4_TAB_BV_RSLT			&
#define TYPE_BOOL_TAB_BV_RSLT			&
#define TYPE_ANGLE_TAB_BV_RSLT			&
#define TYPE_PCNT_FRAC_TAB_BV_RSLT		&
#define TYPE_WORLD_TAB_BV_RSLT			&
#define TYPE_STRING_TAB_BV_RSLT			&
#define TYPE_FILENAME_TAB_BV_RSLT		&
#define TYPE_HSV_TAB_BV_RSLT			&
#define TYPE_COLOR_CHANNEL_TAB_BV_RSLT	&
#define TYPE_TIMEVALUE_TAB_BV_RSLT		&
#define TYPE_RADIOBTN_INDEX_TAB_BV_RSLT &
#define TYPE_MTL_TAB_BV_RSLT			&
#define TYPE_TEXMAP_TAB_BV_RSLT			&
#define TYPE_BITMAP_TAB_BV_RSLT			&
#define TYPE_INODE_TAB_BV_RSLT			&
#define TYPE_REFTARG_TAB_BV_RSLT		&
#define TYPE_INDEX_TAB_BV_RSLT			&
#define TYPE_ENUM_TAB_BV_RSLT			&
#define TYPE_MATRIX3_TAB_BV_RSLT		&
#define TYPE_ANGAXIS_TAB_BV_RSLT		&
#define TYPE_QUAT_TAB_BV_RSLT			&
#define TYPE_BITARRAY_TAB_BV_RSLT		&
#define TYPE_CLASS_TAB_BV_RSLT			&
#define TYPE_RAY_TAB_BV_RSLT			&
#define TYPE_POINT2_TAB_BV_RSLT			&
#define TYPE_MESH_TAB_BV_RSLT			&
#define TYPE_OBJECT_TAB_BV_RSLT			&
#define TYPE_CONTROL_TAB_BV_RSLT		&
#define TYPE_INTERVAL_TAB_BV_RSLT		&
#define TYPE_POINT_TAB_BV_RSLT			&
#define TYPE_HWND_TAB_BV_RSLT			&
#define TYPE_TSTR_TAB_BV_RSLT			&
#define TYPE_IOBJECT_TAB_BV_RSLT		&
#define TYPE_INTERFACE_TAB_BV_RSLT		&
#define TYPE_NAME_TAB_BV_RSLT			&
#define TYPE_COLOR_TAB_BV_RSLT			&
#define TYPE_ACOLOR_TAB_BV_RSLT			&
#define TYPE_FPVALUE_TAB_BV_RSLT		&
#define TYPE_VALUE_TAB_BV_RSLT			&
#define TYPE_DWORD_TAB_BV_RSLT			&
#define TYPE_bool_TAB_BV_RSLT			&
#define TYPE_INTPTR_TAB_BV_RSLT			&
#define TYPE_INT64_TAB_BV_RSLT			&
#define TYPE_DOUBLE_TAB_BV_RSLT			&

//  types for each of the fields

#define TYPE_FLOAT_TYPE				float
#define TYPE_INT_TYPE				int
#define TYPE_RGBA_TYPE				Point3
#define TYPE_POINT3_TYPE			Point3
#define TYPE_FRGBA_TYPE				Point4
#define TYPE_POINT4_TYPE			Point4
#define TYPE_BOOL_TYPE				BOOL
#define TYPE_ANGLE_TYPE				float
#define TYPE_PCNT_FRAC_TYPE			float
#define TYPE_WORLD_TYPE				float
#define TYPE_STRING_TYPE			MCHAR*
#define TYPE_FILENAME_TYPE			MCHAR*
#define TYPE_HSV_TYPE				Point3
#define TYPE_COLOR_CHANNEL_TYPE		float
#define TYPE_TIMEVALUE_TYPE			int
#define TYPE_RADIOBTN_INDEX_TYPE	int
#define TYPE_MTL_TYPE				Mtl*
#define TYPE_TEXMAP_TYPE			Texmap*
#define TYPE_BITMAP_TYPE			PBBitmap*
#define TYPE_INODE_TYPE				INode*
#define TYPE_REFTARG_TYPE			ReferenceTarget*	
#define TYPE_INDEX_TYPE				int
#define TYPE_ENUM_TYPE				int
#define TYPE_MATRIX3_TYPE			Matrix*
#define TYPE_VOID_TYPE				void
#define TYPE_INTERVAL_TYPE			Interval*
#define TYPE_ANGAXIS_TYPE			AngAxis*
#define TYPE_QUAT_TYPE				Quat*
#define TYPE_RAY_TYPE				Ray*
#define TYPE_POINT2_TYPE			Point2*
#define TYPE_BITARRAY_TYPE			BitArray*
#define TYPE_CLASS_TYPE				ClassID*
#define TYPE_MESH_TYPE				Mesh*
#define TYPE_OBJECT_TYPE			Object*
#define TYPE_CONTROL_TYPE			Control*
#define TYPE_POINT_TYPE				POINT*
#define TYPE_TSTR_TYPE				MSTR*
#define TYPE_IOBJECT_TYPE			IObject*
#define TYPE_INTERFACE_TYPE			FPInterface*
#define TYPE_HWND_TYPE				HWND
#define TYPE_NAME_TYPE				MCHAR*
#define TYPE_COLOR_TYPE				Color*
#define TYPE_ACOLOR_TYPE			AColor*
#define TYPE_FPVALUE_TYPE			FPValue*
#define TYPE_VALUE_TYPE				Value*
#define TYPE_DWORD_TYPE				DWORD
#define TYPE_bool_TYPE				bool
#define TYPE_INTPTR_TYPE			INT_PTR
#define TYPE_INT64_TYPE				INT64
#define TYPE_DOUBLE_TYPE			double

// Tab<>s of the above...
#define TYPE_FLOAT_TAB_TYPE				Tab<float>*
#define TYPE_INT_TAB_TYPE				Tab<int>*
#define TYPE_RGBA_TAB_TYPE				Tab<Point3>*
#define TYPE_POINT3_TAB_TYPE			Tab<Point3>*
#define TYPE_FRGBA_TAB_TYPE				Tab<Point4>*
#define TYPE_POINT4_TAB_TYPE			Tab<Point4>*
#define TYPE_BOOL_TAB_TYPE				Tab<BOOL>*
#define TYPE_ANGLE_TAB_TYPE				Tab<float>*
#define TYPE_PCNT_FRAC_TAB_TYPE			Tab<float>*
#define TYPE_WORLD_TAB_TYPE				Tab<float>*
#define TYPE_STRING_TAB_TYPE			Tab<MCHAR*>*
#define TYPE_FILENAME_TAB_TYPE			Tab<MCHAR*>*
#define TYPE_HSV_TAB_TYPE				Tab<Point3>*
#define TYPE_COLOR_CHANNEL_TAB_TYPE		Tab<float>*
#define TYPE_TIMEVALUE_TAB_TYPE			Tab<int>*
#define TYPE_RADIOBTN_INDEX_TAB_TYPE	Tab<int>*
#define TYPE_MTL_TAB_TYPE				Tab<Mtl*>*
#define TYPE_TEXMAP_TAB_TYPE			Tab<Texmap*>*
#define TYPE_BITMAP_TAB_TYPE			Tab<PBBitmap*>*
#define TYPE_INODE_TAB_TYPE				Tab<INode*>*
#define TYPE_REFTARG_TAB_TYPE			Tab<ReferenceTarget*>*
#define TYPE_INDEX_TAB_TYPE				Tab<int>*
#define TYPE_ENUM_TAB_TYPE				Tab<int>*
#define TYPE_MATRIX3_TAB_TYPE			Tab<Matrix*>*
#define TYPE_VOID_TAB_TYPE				Tab<void>*
#define TYPE_INTERVAL_TAB_TYPE			Tab<Interval*>*
#define TYPE_ANGAXIS_TAB_TYPE			Tab<AngAxis*>*
#define TYPE_QUAT_TAB_TYPE				Tab<Quat*>*
#define TYPE_RAY_TAB_TYPE				Tab<Ray*>*
#define TYPE_POINT2_TAB_TYPE			Tab<Point2*>*
#define TYPE_BITARRAY_TAB_TYPE			Tab<BitArray*>*
#define TYPE_CLASS_TAB_TYPE				Tab<ClassID*>*
#define TYPE_MESH_TAB_TYPE				Tab<Mesh*>*
#define TYPE_OBJECT_TAB_TYPE			Tab<Object*>*
#define TYPE_CONTROL_TAB_TYPE			Tab<Control*>*
#define TYPE_POINT_TAB_TYPE				Tab<POINT*>*
#define TYPE_TSTR_TAB_TYPE				Tab<MSTR*>*
#define TYPE_IOBJECT_TAB_TYPE			Tab<IObject*>*
#define TYPE_INTERFACE_TAB_TYPE			Tab<FPInterface*>*
#define TYPE_HWND_TAB_TYPE				Tab<HWND>*
#define TYPE_NAME_TAB_TYPE				Tab<MCHAR*>*
#define TYPE_COLOR_TAB_TYPE				Tab<Color*>*
#define TYPE_ACOLOR_TAB_TYPE			Tab<AColor*>*
#define TYPE_FPVALUE_TAB_TYPE			Tab<FPValue*>*
#define TYPE_VALUE_TAB_TYPE				Tab<Value*>*
#define TYPE_DWORD_TAB_TYPE				Tab<DWORD>*
#define TYPE_bool_TAB_TYPE				Tab<bool>*
#define TYPE_INTPTR_TAB_TYPE			Tab<INT_PTR>*
#define TYPE_INT64_TAB_TYPE				Tab<INT64>*
#define TYPE_DOUBLE_TAB_TYPE			Tab<double>*

// by-pointer
//  foo*  = 
#define TYPE_FLOAT_BP_TYPE				float*
#define TYPE_INT_BP_TYPE				int*
#define TYPE_BOOL_BP_TYPE				int*
#define TYPE_ANGLE_BP_TYPE				float*
#define TYPE_PCNT_FRAC_BP_TYPE			float*
#define TYPE_WORLD_BP_TYPE				float*
#define TYPE_COLOR_CHANNEL_BP_TYPE		float*
#define TYPE_TIMEVALUE_BP_TYPE			int*
#define TYPE_RADIOBTN_INDEX_BP_TYPE		int*
#define TYPE_INDEX_BP_TYPE				int*
#define TYPE_ENUM_BP_TYPE				int*
#define TYPE_DWORD_BP_TYPE				DWORD*
#define TYPE_bool_BP_TYPE				bool*
#define TYPE_INTPTR_BP_TYPE				INT_PTR*
#define TYPE_INT64_BP_TYPE				INT64*
#define TYPE_DOUBLE_BP_TYPE				double*

// by-reference 
#define TYPE_FLOAT_BR_TYPE				float&
#define TYPE_INT_BR_TYPE				int&
#define TYPE_RGBA_BR_TYPE				Point3&
#define TYPE_POINT3_BR_TYPE				Point3&
#define TYPE_FRGBA_BR_TYPE				Point4&
#define TYPE_POINT4_BR_TYPE				Point4&
#define TYPE_BOOL_BR_TYPE				int&
#define TYPE_ANGLE_BR_TYPE				float&
#define TYPE_PCNT_FRAC_BR_TYPE			float&
#define TYPE_WORLD_BR_TYPE				float&
#define TYPE_HSV_BR_TYPE				Point3&
#define TYPE_COLOR_CHANNEL_BR_TYPE		float&
#define TYPE_TIMEVALUE_BR_TYPE			int&
#define TYPE_RADIOBTN_INDEX_BR_TYPE		int&
#define TYPE_BITMAP_BR_TYPE				PBBitmap&
#define TYPE_INDEX_BR_TYPE				int&
#define TYPE_ENUM_BR_TYPE				int&
#define TYPE_REFTARG_BR_TYPE			ReferenceTarget&
#define TYPE_MATRIX3_BR_TYPE			Matrix3&
#define TYPE_ANGAXIS_BR_TYPE			AngAxis&
#define TYPE_QUAT_BR_TYPE				Quat&
#define TYPE_BITARRAY_BR_TYPE			BitArray&
#define TYPE_RAY_BR_TYPE				Ray&
#define TYPE_POINT2_BR_TYPE				Point2&
#define TYPE_MESH_BR_TYPE				Mesh&
#define TYPE_INTERVAL_BR_TYPE			Interval&
#define TYPE_POINT_BR_TYPE				POINT&
#define TYPE_TSTR_BR_TYPE				MSTR&
#define TYPE_COLOR_BR_TYPE				Color&
#define TYPE_ACOLOR_BR_TYPE				AColor&
#define TYPE_FPVALUE_BR_TYPE			FPValue&
#define TYPE_DWORD_BR_TYPE				DWORD&
#define TYPE_bool_BR_TYPE				bool&
#define TYPE_INTPTR_BR_TYPE				INT_PTR&
#define TYPE_INT64_BR_TYPE				INT64&
#define TYPE_DOUBLE_BR_TYPE				double&

// Tab<> by-reference 
#define TYPE_FLOAT_TAB_BR_TYPE				Tab<float>&
#define TYPE_INT_TAB_BR_TYPE				Tab<int>&
#define TYPE_RGBA_TAB_BR_TYPE				Tab<Point3>&
#define TYPE_POINT3_TAB_BR_TYPE				Tab<Point3>&
#define TYPE_FRGBA_TAB_BR_TYPE				Tab<Point4>&
#define TYPE_POINT4_TAB_BR_TYPE				Tab<Point4>&
#define TYPE_BOOL_TAB_BR_TYPE				Tab<BOOL>&
#define TYPE_ANGLE_TAB_BR_TYPE				Tab<float>&
#define TYPE_PCNT_FRAC_TAB_BR_TYPE			Tab<float>&
#define TYPE_WORLD_TAB_BR_TYPE				Tab<float>&
#define TYPE_STRING_TAB_BR_TYPE				Tab<MCHAR*>&
#define TYPE_FILENAME_TAB_BR_TYPE			Tab<MCHAR*>&
#define TYPE_HSV_TAB_BR_TYPE				Tab<Point3>&
#define TYPE_COLOR_CHANNEL_TAB_BR_TYPE		Tab<float>&
#define TYPE_TIMEVALUE_TAB_BR_TYPE			Tab<int>&
#define TYPE_RADIOBTN_INDEX_TAB_BR_TYPE		Tab<int>&
#define TYPE_MTL_TAB_BR_TYPE				Tab<Mtl*>&
#define TYPE_TEXMAP_TAB_BR_TYPE				Tab<Texmap*>&
#define TYPE_BITMAP_TAB_BR_TYPE				Tab<PBBitmap*>&
#define TYPE_INODE_TAB_BR_TYPE				Tab<INode*>&
#define TYPE_REFTARG_TAB_BR_TYPE			Tab<ReferenceTarget*>&
#define TYPE_INDEX_TAB_BR_TYPE				Tab<int>&
#define TYPE_ENUM_TAB_BR_TYPE				Tab<int>&
#define TYPE_MATRIX3_TAB_BR_TYPE			Tab<Matrix*>&
#define TYPE_VOID_TAB_BR_TYPE				Tab<void>&
#define TYPE_INTERVAL_TAB_BR_TYPE			Tab<Interval*>&
#define TYPE_ANGAXIS_TAB_BR_TYPE			Tab<AngAxis*>&
#define TYPE_QUAT_TAB_BR_TYPE				Tab<Quat*>&
#define TYPE_RAY_TAB_BR_TYPE				Tab<Ray*>&
#define TYPE_POINT2_TAB_BR_TYPE				Tab<Point2*>&
#define TYPE_BITARRAY_TAB_BR_TYPE			Tab<BitArray*>&
#define TYPE_CLASS_TAB_BR_TYPE				Tab<ClassID*>&
#define TYPE_MESH_TAB_BR_TYPE				Tab<Mesh*>&
#define TYPE_OBJECT_TAB_BR_TYPE				Tab<Object*>&
#define TYPE_CONTROL_TAB_BR_TYPE			Tab<Control*>&
#define TYPE_POINT_TAB_BR_TYPE				Tab<POINT*>&
#define TYPE_TSTR_TAB_BR_TYPE				Tab<MSTR*>&
#define TYPE_IOBJECT_TAB_BR_TYPE			Tab<IObject*>&
#define TYPE_INTERFACE_TAB_BR_TYPE			Tab<FPInterface*>&
#define TYPE_HWND_TAB_BR_TYPE				Tab<HWND>&
#define TYPE_NAME_TAB_BR_TYPE				Tab<MCHAR*>&
#define TYPE_COLOR_TAB_BR_TYPE				Tab<Color*>&
#define TYPE_ACOLOR_TAB_BR_TYPE				Tab<AColor*>&
#define TYPE_FPVALUE_TAB_BR_TYPE			Tab<FPValue*>&
#define TYPE_VALUE_TAB_BR_TYPE				Tab<Value*>&
#define TYPE_DWORD_TAB_BR_TYPE				Tab<DWORD>&
#define TYPE_bool_TAB_BR_TYPE				Tab<bool>&
#define TYPE_INTPTR_TAB_BR_TYPE				Tab<INT_PTR>&
#define TYPE_INT64_TAB_BR_TYPE				Tab<INT64>&
#define TYPE_DOUBLE_TAB_BR_TYPE				Tab<double>&

// by-value 
#define TYPE_RGBA_BV_TYPE				Point3		
#define TYPE_POINT3_BV_TYPE				Point3
#define TYPE_HSV_BV_TYPE				Point3
#define TYPE_FRGBA_BV_TYPE				Point4		
#define TYPE_POINT4_BV_TYPE				Point4
#define TYPE_BITMAP_BV_TYPE				PBBitmap
#define TYPE_MATRIX3_BV_TYPE			Matrix3
#define TYPE_ANGAXIS_BV_TYPE			AngAxis
#define TYPE_QUAT_BV_TYPE				Quat
#define TYPE_BITARRAY_BV_TYPE			BitArray
#define TYPE_RAY_BV_TYPE				Ray
#define TYPE_POINT2_BV_TYPE				Point2
#define TYPE_MESH_BV_TYPE				Mesh
#define TYPE_INTERVAL_BV_TYPE			Interval
#define TYPE_POINT_BV_TYPE				POINT
#define TYPE_TSTR_BV_TYPE				MSTR
#define TYPE_COLOR_BV_TYPE				Color
#define TYPE_ACOLOR_BV_TYPE				AColor
#define TYPE_FPVALUE_BV_TYPE			FPValue
#define TYPE_CLASS_BV_TYPE				ClassID

// by-val Tab<> 
#define TYPE_FLOAT_TAB_BV_TYPE				Tab<float>
#define TYPE_INT_TAB_BV_TYPE				Tab<int>
#define TYPE_RGBA_TAB_BV_TYPE				Tab<Point3>
#define TYPE_POINT3_TAB_BV_TYPE				Tab<Point3>
#define TYPE_FRGBA_TAB_BV_TYPE				Tab<Point4>
#define TYPE_POINT4_TAB_BV_TYPE				Tab<Point4>
#define TYPE_BOOL_TAB_BV_TYPE				Tab<BOOL>
#define TYPE_ANGLE_TAB_BV_TYPE				Tab<float>
#define TYPE_PCNT_FRAC_TAB_BV_TYPE			Tab<float>
#define TYPE_WORLD_TAB_BV_TYPE				Tab<float>
#define TYPE_STRING_TAB_BV_TYPE				Tab<MCHAR*>
#define TYPE_FILENAME_TAB_BV_TYPE			Tab<MCHAR*>
#define TYPE_HSV_TAB_BV_TYPE				Tab<Point3>
#define TYPE_COLOR_CHANNEL_TAB_BV_TYPE		Tab<float>
#define TYPE_TIMEVALUE_TAB_BV_TYPE			Tab<int>
#define TYPE_RADIOBTN_INDEX_TAB_BV_TYPE		Tab<int>
#define TYPE_MTL_TAB_BV_TYPE				Tab<Mtl*>
#define TYPE_TEXMAP_TAB_BV_TYPE				Tab<Texmap*>
#define TYPE_BITMAP_TAB_BV_TYPE				Tab<PBBitmap*>
#define TYPE_INODE_TAB_BV_TYPE				Tab<INode*>
#define TYPE_REFTARG_TAB_BV_TYPE			Tab<ReferenceTarget*>
#define TYPE_INDEX_TAB_BV_TYPE				Tab<int>
#define TYPE_ENUM_TAB_BV_TYPE				Tab<int>
#define TYPE_MATRIX3_TAB_BV_TYPE			Tab<Matrix*>
#define TYPE_VOID_TAB_BV_TYPE				Tab<void>
#define TYPE_INTERVAL_TAB_BV_TYPE			Tab<Interval*>
#define TYPE_ANGAXIS_TAB_BV_TYPE			Tab<AngAxis*>
#define TYPE_QUAT_TAB_BV_TYPE				Tab<Quat*>
#define TYPE_RAY_TAB_BV_TYPE				Tab<Ray*>
#define TYPE_POINT2_TAB_BV_TYPE				Tab<Point2*>
#define TYPE_BITARRAY_TAB_BV_TYPE			Tab<BitArray*>
#define TYPE_CLASS_TAB_BV_TYPE				Tab<ClassID*>
#define TYPE_MESH_TAB_BV_TYPE				Tab<Mesh*>
#define TYPE_OBJECT_TAB_BV_TYPE				Tab<Object*>
#define TYPE_CONTROL_TAB_BV_TYPE			Tab<Control*>
#define TYPE_POINT_TAB_BV_TYPE				Tab<POINT*>
#define TYPE_TSTR_TAB_BV_TYPE				Tab<MSTR*>
#define TYPE_IOBJECT_TAB_BV_TYPE			Tab<IObject*>
#define TYPE_INTERFACE_TAB_BV_TYPE			Tab<FPInterface*>
#define TYPE_HWND_TAB_BV_TYPE				Tab<HWND>
#define TYPE_NAME_TAB_BV_TYPE				Tab<MCHAR*>
#define TYPE_COLOR_TAB_BV_TYPE				Tab<Color*>
#define TYPE_ACOLOR_TAB_BV_TYPE				Tab<AColor*>
#define TYPE_FPVALUE_TAB_BV_TYPE			Tab<FPValue*>
#define TYPE_VALUE_TAB_BV_TYPE				Tab<Value*>
#define TYPE_DWORD_TAB_BV_TYPE				Tab<DWORD>
#define TYPE_bool_TAB_BV_TYPE				Tab<bool>
#define TYPE_INTPTR_TAB_BV_TYPE				Tab<INT_PTR>
#define TYPE_INT64_TAB_BV_TYPE				Tab<INT64>
#define TYPE_DOUBLE_TAB_BV_TYPE				Tab<double>





