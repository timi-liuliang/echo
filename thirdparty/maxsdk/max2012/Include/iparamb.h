/**********************************************************************
 *<
	FILE: iparamb.h

	DESCRIPTION: Interface to Parameter blocks

	CREATED BY: Rolf Berteig

	HISTORY: created 1/25/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "ref.h"
#include "ioapi.h"
// forward declaration
class Control;
class Point3;
class Color;

class UserType : public ReferenceTarget {
	public:		
		virtual ~UserType() {};
		virtual Control* CreateController()=0;
		virtual BOOL operator==( const UserType& t )=0;
		virtual UserType& operator=( const UserType& t )=0;		
	};


// Built in data types
#include "paramtype.h"

// Chunk IDs for loading/saving
#define PB_COUNT_CHUNK			0x0001
#define PB_PARAM_CHUNK			0x0002
#define PB_INDEX_CHUNK			0x0003
#define PB_ANIMATABLE_CHUNK		0x0004
#define PB_VERSION_CHUNK		0x0005
#define PB_LOCKED_CHUNK			0x0006
#define PB_FLOAT_CHUNK			(TYPE_FLOAT + 0x100)
#define PB_INT_CHUNK			(TYPE_INT + 0x100)
#define PB_RGBA_CHUNK			(TYPE_RGBA + 0x100)
#define PB_POINT3_CHUNK			(TYPE_POINT3 + 0x100)
#define PB_BOOL_CHUNK			(TYPE_BOOL + 0x100)

#define PB_TYPE_CHUNK			0x0200
#define PB_TYPE_FLOAT_CHUNK		(PB_TYPE_CHUNK + TYPE_FLOAT)
#define PB_TYPE_INT_CHUNK		(PB_TYPE_CHUNK + TYPE_INT)
#define PB_TYPE_RGBA_CHUNK		(PB_TYPE_CHUNK + TYPE_RGBA)
#define PB_TYPE_POINT3_CHUNK	(PB_TYPE_CHUNK + TYPE_POINT3)
#define PB_TYPE_BOOL_CHUNK		(PB_TYPE_CHUNK + TYPE_BOOL)
#define PB_TYPE_USER_CHUNK		(PB_TYPE_CHUNK + TYPE_USER)


// When a client of a param block receives the \ref REFMSG_GET_PARAM_NAME
// message, the partID field is set to point at one of these structures.
// The client should fill in the parameter name.
/*! \sa  Class ParamDimension.
\par Description:
This class is used to hold a parameter name. When a client of a parameter block
receives the \ref REFMSG_GET_PARAM_NAME message, the partID field is set to
point at one of these structures. The client should fill in the parameter name.
\par Data Members:
<b>MSTR name;</b>\n\n
Assign the parameter name to this variable.\n\n
<b>int index;</b>\n\n
Index of the parameter in the parameter block.  */
class GetParamName: public MaxHeapOperators {
	public:
		MSTR name;
		int index;
		/*! \remarks Constructor. */
		GetParamName(MSTR n,int i) { name=n;index=i; }
	};

// When a client of a param block receives the \ref REFMSG_GET_PARAM_DIM
// message, the partID field is set to point at one of these structs.
// The client should set dim to point at it's dim descriptor.
/*! \sa  Class ParamDimension.
\par Description:
This class is used to store a parameter dimension. When a client of a parameter
block receives the \ref REFMSG_GET_PARAM_DIM message, the PartID (\ref partids) field
is set to point at one of these structures. The client should set <b>dim</b> to
point at its dim descriptor.
\par Data Members:
<b>ParamDimension *dim;</b>\n\n
Assign the dimension to this variable.\n\n
<b>int index;</b>\n\n
Index of the parameter in the parameter block.  */
class GetParamDim: public MaxHeapOperators {
	public:
		ParamDimension *dim;
		int index;
		/*! \remarks Constructor. */
		GetParamDim(int i) {index=i;dim=NULL;}
	};


// To create a parameter block, pass an array of these descriptors
// into the Create function. 
// Items in the parameter block can be refered to by index. The 
// index is derived from the order in which the descriptors appear
// in the array. If a parameter is a UserType, then a pointer to a 
// new UserType must be passed in. The parameter block will be responsible
// for deleting it when it is done with it.

/*! \sa  Class ParamBlockDescID.\n\n
\par Description:
The parameter block descriptor describes each parameter in a parameter block.
<b>class ParamBlockDesc {</b>\n\n
<b></b>\n\n
<b>ParamType type;</b>\n\n
<b>UserType *user;</b>\n\n
<b>BOOL animatable;</b>\n\n
<b>};</b>
\par Data Members:
<b>ParamType type</b>\n\n
The parameter type. See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of
Parameter Types</a>.\n\n
<b>UserType *user</b>\n\n
This value is not used -- it must always be passed as NULL.\n\n
<b>BOOL animatable</b>\n\n
This is a flag indicating if the parameter may be animated or not. Pass TRUE if
the value may be animated and FALSE if it is constant. */
class ParamBlockDesc {
	public:
		ParamType type;
		UserType *user;	
		BOOL animatable;
	};

// This version of the descriptor has an ID for each parameter.
/*! \sa  Class ParamBlockDesc.\n\n
\par Description:
The parameter block descriptor describes each parameter in a parameter block.
This version has an ID used to identify each parameter.\n\n
<b>class ParamBlockDescID {</b>\n\n
<b></b>\n\n
<b>ParamType type;</b>\n\n
<b>UserType *user;</b>\n\n
<b>BOOL animatable;</b>\n\n
<b>DWORD id;</b>\n\n
<b>};</b>
\par Data Members:
<b>ParamType type</b>\n\n
The parameter type. See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of
Parameter Types</a>.\n\n
<b>UserType *user</b>\n\n
This value is not currently used -- it must always be passed as NULL.\n\n
<b>BOOL animatable</b>\n\n
This is a flag indicating if the parameter may be animated or not. Pass TRUE if
the value may be animated and FALSE if it is constant.\n\n
<b>DWORD id</b>\n\n
This is an ID used to identify this parameter. This provides a solution to the
problem of backwards compatibility. If you alter the parameter structure of
your plug-in in the future (by adding or deleting parameters for example)
previously saved 3ds Max files will be incompatible. You can however use a
mechanism which uses these IDs to convert older versions to the current
version. See the Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>
for more detail on how this is done. */
class ParamBlockDescID {
	public:
		ParamType type;
		UserType *user;	
		BOOL animatable;
		DWORD id;
	};

class IParamBlock;

// This class represents a virtual array of parameters.
// Parameter blocks are one such implementation of this class, but
// it can also be useful to implement a class that abstracts non-
// parameter block variables. 
//
// The ParamMap class (see IParamM.h) uses this base class so that
// a ParamMap can be used to control UI for not only parameter blocks
// but variables stored outside of parameter blocks.
/*! \sa  Class IParamBlock, Class IParamMap.\n\n
\par Description:
This class represents a virtual array of parameters. Parameter are accessed
using an integer index and <b>GetValue()</b> and <b>SetValue()</b> methods.
Parameter blocks are one such implementation of this class, but it can also be
useful to implement a class that abstracts non-parameter block variables. The
ParamMap class uses this base class so that a ParamMap can be used to control
UI for not only parameter blocks but also variables stored outside of parameter
blocks. The Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>
discusses how this is done. <br>  Default implementation of these methods are
provided which simply return FALSE. */
#pragma warning(push)
#pragma warning(disable:4100)
class IParamArray : public MaxHeapOperators {
	public:
		/*! \remarks This method is called when the system needs to store a value into a
		variable. There are overloaded functions for each type of value to set
		(int, float, and Point3). Each method has three parameters. Below is
		the float version - the others are similar.
		\par Parameters:
		<b>int i</b>\n\n
		This is the index into the virtual array of the value to set.\n\n
		<b>TimeValue t</b>\n\n
		This is the time at which to set the value.\n\n
		<b>float v</b>\n\n
		This is the value to set.
		\return  This is not currently used. */
		virtual BOOL SetValue( int i, TimeValue t, float v ) {return FALSE;}
		/*! \remarks This is the integer version of above. */
		virtual BOOL SetValue( int i, TimeValue t, int v ) {return FALSE;}
		/*! \remarks This is the Point3 version of above. */
		virtual BOOL SetValue( int i, TimeValue t, Point3& v ) {return FALSE;}
		
		/*! \remarks Whenever the developer needs to retrieve a value from the parameter
		block, the <b>GetValue()</b> method is used. There are overloaded
		functions for each type of value to retrieve (int, float, Point3, and
		Color). Each method has four parameters:
		\par Parameters:
		<b>int i</b>\n\n
		This is the index into the virtual array of the value to retrieve.\n\n
		<b>TimeValue t</b>\n\n
		This is the time at which to retrieve the value. For constants pass
		0.\n\n
		<b>float \&v</b>\n\n
		This is the value to retrieve.\n\n
		<b>Interval \&ivalid</b>\n\n
		This is the validity interval to update to reflect the validity of this parameter.
		\return  If the value was retrieved TRUE is returned; otherwise FALSE is returned. */
		virtual BOOL GetValue( int i, TimeValue t, float &v, Interval &ivalid ) {return FALSE;}
		/*! \remarks This is the integer version of above. */
		virtual BOOL GetValue( int i, TimeValue t, int &v, Interval &ivalid ) {return FALSE;}
		/*! \remarks This is the Point3 version of above. */
		virtual BOOL GetValue( int i, TimeValue t, Point3 &v, Interval &ivalid ) {return FALSE;}
	
		// If it is a param block, this will get a pointer to it, otherwise it will return NULL.
		// Note that casting won't work because of multiple iheritance.
		/*! \remarks Implemented by the System.\n\n
		If the array uses a parameter block, this method will return a pointer
		to it, otherwise it will return NULL. Note that casting won't work
		because of multiple inheritance.
		\return  Returns a pointer to the parameter block if one is used; NULL otherwise. */
		virtual IParamBlock *GetParamBlock() {return NULL;}

		// Checks to see if a keyframe exists for the given parameter at the given time
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Checks to see if a keyframe exists for the given parameter at the given
		time. Returns TRUE if a keyframe exists at the specified time;
		otherwise FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		Zero based index of the parameter to check.\n\n
		<b>TimeValue t</b>\n\n
		The time to check.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL KeyFrameAtTime(int i, TimeValue t) {return FALSE;}
	};
#pragma warning(pop)
/*! \sa  Class ReferenceTarget, Class IParamArray, <a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>,
Class ParamBlockDescID, Class Control.\n\n
\par Description:
This class provides methods to work with parameter blocks. For more details on
parameter blocks see
<a href="ms-its:3dsmaxsdk.chm::/paramblocks2_root.html">Parameter
Blocks</a>.  */
class IParamBlock : 			
			public ReferenceTarget,
			public IParamArray {
	public:
		// Get's the super class of a parameters controller
		/*! \remarks Returns the super class ID of a parameters controller.
		\par Parameters:
		<b>int anim</b>\n\n
		Specifies the parameter whose controller super class ID is returned. */
		virtual SClass_ID GetAnimParamControlType(int anim)=0;

		// Get the param type
		/*! \remarks Returns the type of the 'i-th' parameter.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the parameter to retrieve. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_parameter_types.html">List of Parameter Types</a>. */
		virtual ParamType GetParameterType(int i)=0;

		// one for each known type
		/*! \remarks Implemented by the System.\n\n
		Whenever the developer needs to store a value into the parameter block,
		the <b>SetValue()</b> method is used. There are overloaded functions
		for each type of value to set (int, float, Point3, and Color). Each
		method has three parameters. Below is the float version - the others
		are similar.
		\par Parameters:
		<b>int i</b>\n\n
		This is the index into the <b>ParamBlockDesc</b> array of the parameter
		to set.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value.\n\n
		<b>float v</b>\n\n
		The value to store.
		\return If the value was set TRUE is returned; otherwise FALSE is returned. */
		virtual BOOL SetValue( int i, TimeValue t, float v )=0;
		/*! \remarks This is the integer version of above. */
		virtual BOOL SetValue( int i, TimeValue t, int v )=0;		
		/*! \remarks This is the Point3 version of above. */
		virtual BOOL SetValue( int i, TimeValue t, Point3& v )=0;		
		/*! \remarks This is the Color version of above. */
		virtual BOOL SetValue( int i, TimeValue t, Color& v )=0; // uses Point3 controller
		
		// one for each known type
		/*! \remarks Implemented by the System.\n\n
		Whenever the developer needs to retrieve a value from the parameter
		block, the <b>GetValue()</b> method is used. There are overloaded
		functions for each type of value to retrieve (int, float, Point3, and Color).
		<b>Important Note: Developers need to do range checking on values
		returned from a parameter block -- a spinner custom control will not
		necessarily ensure that the values entered by a user and stored by a
		parameter block are fixed to the values allowed by the spinner. For
		example, the spinner control ensures that it only displays, and the
		user is only allowed to input, values within the specified ranges.
		However the spinner is just a front end to a controller which actually
		controls the value. The user can thus circumvent the spinner
		constraints by editing the controller directly (via function curves in
		track view, key info, etc.). Therefore, when a plug-in gets a value
		from a controller (or a parameter block, which may use a controller) it
		is its responsibility to clamp the value to a valid range.</b>\n\n
		The <b>GetValue()</b> method updates the interval passed in. This
		method is frequently used by developers to 'whittle' down an interval.
		When a parameter of a parameter block is animated, for any given time
		there is a interval over which the parameter is constant. If the
		parameter is constantly changing the interval is instantaneous. If the
		parameter does not change for a certain period the interval will be
		longer. If the parameter never changes the interval will be
		<b>FOREVER</b>. By passing an interval to the GetValue() method you ask
		the parameter block to 'intersect' the interval passed in with the
		interval of the parameter. Intersecting two intervals means returning a
		new interval whose start value is the greater of the two, and whose end
		value is smaller of the two. In this way, the resulting interval
		represents a combined period of constancy for the two intervals.\n\n
		This technique is used to compute a validity interval for an object.
		The developer starts an interval off as <b>FOREVER</b>, then intersects
		this interval with each of its animated parameters (by calling
		<b>GetValue()</b>). <b>GetValue()</b> 'whittles' down the interval with
		each call. When all the parameters have been intersected the result is
		the overall validity interval of an object at a specific time.
		\par Parameters:
		<b>int i</b>\n\n
		Index into the <b>ParamBlockDesc</b> array of the parameter to
		retrieve.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to retrieving a value.\n\n
		<b>float v</b>\n\n
		The value to store for the parameter at the time.\n\n
		<b>Interval \&ivalid</b>\n\n
		The interval to update.
		\return  The return value is TRUE if a value was retrieved. Otherwise
		it is FALSE.\n\n
		\sa
		<a href="ms-its:3dsmaxsdk.chm::/paramblocks2_root.html">Parameter
		Blocks</a>in the Advanced Topics section,
		<a href="ms-its:3dsmaxsdk.chm::/start_conceptual_overview.html#heading_08">Intervals</a>.
		*/
		virtual BOOL GetValue( int i, TimeValue t, float &v, Interval &ivalid )=0;
		/*! \remarks This is the integer version of above. */
		virtual BOOL GetValue( int i, TimeValue t, int &v, Interval &ivalid )=0;
		/*! \remarks This is the Point3 version of above. */
		virtual BOOL GetValue( int i, TimeValue t, Point3 &v, Interval &ivalid )=0;
		/*! \remarks This is the Color version of above. */
		virtual BOOL GetValue( int i, TimeValue t, Color &v, Interval &ivalid )=0; // uses Point3 Controller

		virtual Color  GetColor(int i, TimeValue t=0)=0;
		virtual Point3 GetPoint3(int i, TimeValue t=0)=0;
		virtual int    GetInt(int i, TimeValue t=0)=0;
		virtual float  GetFloat(int i, TimeValue t=0)=0;

		/*! \remarks Implemented by the System.\n\n
		Returns the parameter block version. */
		virtual DWORD GetVersion()=0;
		virtual int NumParams()=0;

		/*! \remarks Implemented by the System.\n\n
		Removes the 'i-th' controller.
		\par Parameters:
		<b>int i</b>\n\n
		The parameter index of the controller to remove. */
		virtual void RemoveController(int i)=0;
		/*! \remarks Implemented by the System.\n\n
		Returns the controller of the 'i-th' parameter.
		\par Parameters:
		<b>int i</b>\n\n
		The parameter index of the controller to return. */
		virtual Control* GetController(int i)=0;
		/*! \remarks Implemented by the System.\n\n
		Sets the 'i-th' parameter controller to <b>c</b>.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the parameter to set.\n\n
		<b>Control *c</b>\n\n
		The controller to set.\n\n
		<b>BOOL preserveFrame0Value=TRUE</b>\n\n
		If TRUE the controllers value at frame 0 is preserved. */
		virtual void SetController(int i, Control *c, BOOL preserveFrame0Value=TRUE)=0;
		/*! \remarks Implemented by the System.\n\n
		Swaps the two controllers of the parameters whose indices are passed.
		\par Parameters:
		<b>int j, int k</b>\n\n
		The parameter indices whose controllers should be swapped. */
		virtual	void SwapControllers(int j, int k )=0;

		// Given the parameter index, what is the refNum?
		/*! \remarks Implemented by the System.\n\n
		Given a parameter index this method will return the reference number of
		that parameter.
		\par Parameters:
		<b>int paramNum</b>\n\n
		The parameter index. */
		virtual	int GetRefNum(int paramNum)=0;

		// Given the parameter index what is the animNum?
		/*! \remarks Implemented by the System.\n\n
		Given a parameter index this method will return the anim number.
		\par Parameters:
		<b>int paramNum</b>\n\n
		The parameter index. */
		virtual	int GetAnimNum(int paramNum)=0;

		// Given the animNum what is the parameter index?
		/*! \remarks Implemented by the System.\n\n
		Given an anim number this method will return the parameter index.
		\par Parameters:
		<b>int animNum</b>\n\n
		The anim number. */
		virtual	int AnimNumToParamNum(int animNum)=0;

		// Inherited from IParamArray
		IParamBlock *GetParamBlock() {return this;}
				
		// This is only for use in a RescaleWorldUnits() implementation:
		// The param block implementation of RescaleWorldUnits scales only tracks
		// that have dimension type = stdWorldDim. If letting the param block handle 
		// the rescaling is not sufficient, call this on just the parameters you need to rescale.
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		This is only for use in a <b>ReferenceMaker::RescaleWorldUnits()</b>
		implementation: The parameter block implementation of
		<b>RescaleWorldUnits()</b> scales only tracks that have dimension type
		= <b>stdWorldDim</b>. If letting the parameter block handle the
		rescaling is not sufficient, call this on just the parameters you need to rescale. 
		\par Parameters:
		<b>int paramNum</b>\n\n
		The index into the parmeter block of the parameter to rescale.\n\n
		<b>float f</b>\n\n
		The value to scale by. */
		virtual void RescaleParam(int paramNum, float f)=0;

		// When a NotifyRefChanged is received from a param block, you 
		// can call this method to find out which parameter generated the notify.
		/*! \remarks This method is available in release 2.0 and later only.
		Returns the index into a parameter block of the parameter that
		generated a notification. You can call this method when you get a
		<b>NotifyRefChanged()</b> message from your parameter block to
		determine exactly which parameter it was that changed. When you have a
		very complicated dialog, for instance the Standard material, you can
		use this to selectively update controls in the dialog instead of
		updating all of them, which can feel pretty slow to the user. For
		smaller dialogs it's not worth the trouble.
		\par Sample Code:
		\code
		case REFMSG_CHANGE:
			if (hTarget == pblock)
			{
				int np = pblock->LastNotifyParamNum();
				// ...
			}
			break;
		\endcode
		*/
		virtual int LastNotifyParamNum()=0;
		};


/*! \remarks Implemented by the System.\n\n
This method is used to create a parameter block.
\par Parameters:
<b>ParamBlockDesc *pdesc</b>\n\n
This is an array of parameter block descriptors.\n\n
<b>int count</b>\n\n
This is the number in the array.
\return  A pointer to the created parameter block. On error NULL is returned.
*/
CoreExport IParamBlock *CreateParameterBlock(ParamBlockDesc *pdesc, int count);

// Note: version must fit into 16 bits. (5/20/97)
/*! \remarks Implemented by the System.\n\n
This method is used to create a parameter block with a version number to aide
in backwards compatibility.
\par Parameters:
<b>ParamBlockDesc *pdesc</b>\n\n
This is an array of parameter block descriptors.\n\n
<b>int count</b>\n\n
This is the number in the array.\n\n
<b>DWORD version</b>\n\n
This is used to indicate a version of the parameter block. This is used for
backwards compatibility when loading 3ds Max files that were saved with a
previous version of the parameter block structure. There is a mechanism which
allows the older format to be converted to the newer format so the older files
may still be loaded and used. See the Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>
for more information.
\return  A pointer to the created parameter block. On error NULL is returned.
*/
CoreExport IParamBlock *CreateParameterBlock(ParamBlockDescID *pdesc, int count, DWORD version);

// This creates a new parameter block, based on an existing parameter block of
// a later version. The new parameter block inherits any parameters from
// the old parameter block whose parameter IDs match.
/*! \remarks Implemented by the System.\n\n
This creates a new parameter block, based on an existing parameter block of a
later version. The new parameter block inherits any parameters from the old
parameter block whose parameter IDs match.
\par Parameters:
<b>ParamBlockDescID *pdescOld</b>\n\n
The existing parameter block descriptor.\n\n
<b>int oldCount</b>\n\n
The number of old parameters.\n\n
<b>IParamBlock *oldPB</b>\n\n
The old parameter block.\n\n
<b>ParamBlockDescID *pdescNew</b>\n\n
The new parameter block descriptor.\n\n
<b>int newCount</b>\n\n
The number of new parameters.\n\n
<b>DWORD newVersion</b>\n\n
The version of the new parameter block.
\return  The new parameter block. */
CoreExport IParamBlock *UpdateParameterBlock(
	ParamBlockDescID *pdescOld, int oldCount, IParamBlock *oldPB,
	ParamBlockDescID *pdescNew, int newCount, DWORD newVersion);


// ----------------------------------------------------------
// A handy post load call back for fixing up parameter blocks.

// This structure describes a version of the parameter block.
/*!
\par Description:
This structure describes a version of the parameter block.
\par Data Members:
<b>ParamBlockDescID *desc;</b>\n\n
This is an array of parameter block descriptors.\n\n
<b>int count;</b>\n\n
This is the number of items in the array.\n\n
<b>DWORD version;</b>\n\n
This is the version number.  */
class ParamVersionDesc: public MaxHeapOperators {
	public:
		ParamBlockDescID *desc;
		int count;
		DWORD version;
		/*! \remarks Constructor.
		\par Parameters:
		<b>ParamBlockDescID *d</b>\n\n
		This is an array of parameter block descriptors.\n\n
		<b>int c</b>\n\n
		This is the number of items in the array.\n\n
		<b>int v</b>\n\n
		This is the version number. */
		ParamVersionDesc(ParamBlockDescID *d,int c,int v) {desc=d;count=c;version=v;}
	};

// This will look up the version of the loaded callback and 
// fix it up so it matches the current version.
// NOTE: this thing deletes itself when it's done.
/*! \sa  Class ParamVersionDesc.
\par Description:
This is a handy post load call back for fixing up parameter blocks. This will
look up the version of the loaded callback and fix it up so it matches the
current version. NOTE: this deletes itself when its done. See
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>
for more details.  */
class ParamBlockPLCB : public PostLoadCallback {
	public:
		ParamVersionDesc *versions;
		int count;
		ParamVersionDesc *cur;		
		ReferenceTarget *targ;
		int pbRefNum;

		/*! \remarks Constructor.
		\par Parameters:
		<b>ParamVersionDesc *v</b>\n\n
		This is an array of ParamVersionDescs.\n\n
		<b>int cnt</b>\n\n
		This is the number of elements in the array specified above.\n\n
		<b>ParamVersionDesc *c</b>\n\n
		This is a pointer to the current version of the ParamVersionDesc.\n\n
		<b>ReferenceTarget *t</b>\n\n
		This is a pointer to a reference target. This is usually the
		<b>this</b> pointer of the object.\n\n
		<b>int refNum</b>\n\n
		This is the reference index of the parameter block.\n\n
		\sa  Advanced Topics section under
		<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter
		Maps</a> for an explanation of how this is used. */
		ParamBlockPLCB(
			ParamVersionDesc *v,int cnt,ParamVersionDesc *c,
			ReferenceTarget *t,int refNum)
			{versions=v;count=cnt;cur=c;targ=t;pbRefNum=refNum;}
		CoreExport void proc(ILoad *iload);
		int Priority() { return 0; }
		CoreExport INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);
	};




