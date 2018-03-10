/**********************************************************************
 *<
	FILE: iparamb2.h

	DESCRIPTION: Interface to Parameter blocks, 2nd edition

	CREATED BY: Rolf Berteig,
				John Wainwright, 2nd Ed.

	HISTORY: created 1/25/95
			 2nd Ed. 9/2/98

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "PB2Export.h"
#include "maxheap.h"
#include "iparamb.h"
#include "iparamm.h"
#include "assetmanagement\AssetType.h"
#include "assetmanagement\AssetId.h"
#include "strbasic.h"         // MCHAR
#include "iparamb2Typedefs.h"
#include "TabTypes.h"
// forward declarations
class Matrix3;
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}


// per descriptor flags
#define P_CLASS_PARAMS		0x0001		//!< this block holds class-level parameters, attached to ClassDesc
#define P_AUTO_CONSTRUCT	0x0002		//!< instructs ClassDesc2 to autoconstuct this block & wire it in, requires pblock refno
#define	P_AUTO_UI			0x0004		//!< this block support automatic UI rollout managements, requires rollout template res ID, etc.
#define	P_USE_PARAMS		0x0008		//!< this block shares (exactly) the paramdefs from another descriptor, requires address of source descriptor
#define	P_INCLUDE_PARAMS	0x0010		//!< this block loads in a copy the paramdefs from another descriptor, requires address of source descriptor
#define P_MULTIMAP			0x0020		//!< indicates this block as mulitple parameter maps   ### move me
#define P_CALLSETS_ON_LOAD	0x0040		//!< causes CallSets() to be called during load PLCB for this block
#define P_HASCATEGORY		0x0080		//!< indicates, that category field is defined for rollup (after rollupproc)
#define P_TEMPLATE_UI		0x0100		//!< indicates that dialog templates will be provided or constructed
//! \brief Indicates a parameter block version number is specified
/*! When used as a ParamBlockDesc2 flag, a version number is expected immediately after the flags field, before the reference number field.
    This version number will be stored as ParamBlockDesc2::version and returned as IParamBlock2::GetVersion() */
#define P_VERSION			0x0200		//!< indicates a version number is specified (integer value, before pblock refno)

// per descriptor internal flags
#define	P_SCRIPTED_CLASS	0x1000		//!< belongs to a scripted plug-in class
#define	P_TEMPORARY			0x2000		//!< temporary descriptor built during scene load to support schema migration

// per param constructor-specifiable flags (flag value 1)
#define P_ANIMATABLE		0x00000001	//!< animatable param
#define P_TRANSIENT			0x00000002	//!< do not store actual value, PBAccessor-derived
#define P_NO_INIT			0x00000004	//!< do not initialize
#define P_COMPUTED_NAME		0x00000008	//!< call compute name fn to get name
#define P_INVISIBLE			0x00000010	//!< not visible in track view (if an animatable)
#define P_RESET_DEFAULT		0x00000020	//!< do not make create params sticky, reset to defaults always
#define P_SUBANIM			0x00000040	//!< non-animatable reference param is still a subanim (makes it visible in TV)
#define P_TV_SHOW_ALL		0x00000080	//!< for Tab<> animatables, show all entries even if no controller assigned
#define P_NO_REF			0x00000100	//!< for reftarg params do not maintain Reference automatically
#define P_OWNERS_REF		0x00000200	//!< reference param maintained by owner, specify owner's reference number via the p_refno tag
#define P_CAN_CONVERT		0x00000400	//!< indicates the p_classid validator is is in a CanConvertoTo() call, rather than as exact class
#define P_SUBTEX			0x00000800	//!< indicates texmap param is kept by owner using MtlBase::xSubTexmap protocol, give subtex # in p_subtexno
#define P_VARIABLE_SIZE		0x00001000	//!< Tab<> param is variable size allowing scripted changes
#define P_NO_AUTO_LABELS	0x00002000	//!< don't auto-set map & mtl names for associated button UI controls
#define P_SHORT_LABELS		0x00004000	//!< use short auto names for associated button UI controls
#define P_READ_ONLY			0x00008000	//!< this parameter is not assignable through MAXScript (allows try-and-buy 3rd-party plugins)

//! \brief Indicates parameter is obsolete
/* When used as a ParamBlockDesc2 param flag, the param will not be saved to disk or maintain references or animation keys.
When loading older files, the parameter will be reset after the load is completed, but will be valid during a post-load callback */
#define P_OBSOLETE		    0x40000000	// obsolete param, don't maintain refs, a controller or a tab, and don't save to file

//! \brief Indicates that a second per param constructor-specifiable flag value follows the first in the ParamBlockDesc2.
/* If this flag is set on the first per param constructor-specifiable flag value, a second DWORD size per param constructor-specifiable 
   flag value follows the first. The value will be read in, shifted left DWORD places and combined with the first flag value to form the 
   ParamDef flag value.*/
#define P_READ_SECOND_FLAG_VALUE 0x80000000  

//! \brief Indicates whether or not when doing a get or set on a param2 value that we only go through the accessor if one is available.
/* If this flag is set the paramblock2 will only go through the accessor when getting or setting a value, or when determining if a key 
exists at a certain time. If this flag isn't set, the normal operation on a SetValue is to send the value through the accessor first, 
and then set the controller's value; on a GetValue to get the value from the controller and then send it through the accessor.
This flag is specified in the second per param constructor-specifiable flag value*/
#define P_USE_ACCESSOR_ONLY 0x00000001


// per param internal flags
#define P_IS_REF			0x0000000000010000 //!< is a reftarget param
#define P_HAS_DEFAULT		0x0000000000020000 //!< has accessor function => a virtual param
#define P_HAS_CUR_DEFAULT	0x0000000000040000 //!< has a snapshotted current default value
#define P_HAS_MS_DEFAULT	0x0000000000080000 //!< has a MAXScript default
#define P_HAS_RANGE			0x0000000000100000 //!< has a range specified
#define P_HAS_CLASS_ID		0x0000000000200000 //!< a classID validator was given
#define P_HAS_SCLASS_ID		0x0000000000400000 //!< an SClassID validator was given
#define P_UI_ENABLED		0x0000000000800000 //!< indicates whether UI controls are initially enabled or diabled
#define P_HAS_PROMPT	    0x0000000001000000 //!< has status line prompt string res ID for various picker buttons
#define P_HAS_CAPTION	    0x0000000002000000 //!< has caption string res ID for open/save file dlgs
#define P_HAS_FILETYPES	    0x0000000004000000 //!< has file types string res ID for open/save file dlgs (in MAXScript type: form)
#define P_HAS_REFNO		    0x0000000008000000 //!< has refno supplied
#define P_HAS_SUBTEXNO	    0x0000000010000000 //!< has subtexno supplied
 //! \brief [INTERNAL ONLY] Sets a ParamDef as included from another descriptor.
 /*! If this is set on a ParamDef, its member ptrs have been copied
 		 from another descriptor, and will not be released */
#define P_INCLUDED		    0x0000000020000000
#define P_HAS_TOOLTIP	    0x0001000000000000 //!< has ToolTip string
#define P_HAS_ASSETTYPE		0x0002000000000000 //!< has asset type
#define P_HAS_ASSETTYPENAME	0x0004000000000000 //!< has asset type name

// Parameter types
#include "paramtype.h"  // parameter type codes

#define base_type(t)	((ParamType2)((t) & ~(TYPE_TAB)))	//!< get base type ignoring Tab flag
#define root_type(t)	((ParamType2)((t) & ~(TYPE_TAB | TYPE_BY_VAL | TYPE_BY_REF | TYPE_BY_PTR)))	//!< get base type ignoring all flags
#define is_tab(t)		((t) & TYPE_TAB)					//!< is this param a table?
#define is_by_val(t)	((t) & TYPE_BY_VAL)					//!< is this param passed by value?  (only for FnPub)
#define is_by_ref(t)	((t) & TYPE_BY_REF)					//!< is this param passed by reference?  (only for FnPub)
#define is_by_ptr(t)	((t) & TYPE_BY_PTR)					//!< is this param passed by pointer?  (only for FnPub)
#define is_ref(d)		(((d).flags & (P_IS_REF | P_NO_REF | P_OWNERS_REF)) == P_IS_REF) //!< is this param a true local refmaker?
#define has_ui(d)		((d).ctrl_count > 0)				//!< this param has UI info defined
#define reftarg_type(t) (base_type(t) == TYPE_MTL || base_type(t) == TYPE_TEXMAP || base_type(t) == TYPE_INODE || \
	                     base_type(t) == TYPE_REFTARG || base_type(t) == TYPE_PBLOCK2 || base_type(t) == TYPE_OBJECT || base_type(t) == TYPE_CONTROL)

class ParamBlockDesc2;
class ClassDesc;
class PBBitmap;
class ParamMap2UserDlgProc;
class MSPluginClass;
class Value;
class Rollout;
class FPInterface;

// parameter value
#pragma pack(push,parameter_entry)
//#pragma pack(1)	// this messes up Win64 builds & the GreatCircle memory debugger

class Texmap;
class Mtl;
class INode;
class IAutoMParamDlg;
class IAutoSParamDlg;
class IAutoEParamDlg;
struct ParamDef;

/*! \sa Class IParamBlock2, Class PBAccessor, Class PBBitmap, Class ReferenceTarget, Class Control.
\remarks This structure holds the value in a ParamBlock2 or PBAccessor.
*/
struct PB2Value: public MaxHeapOperators {
	union
	{
		/*! This is used by: TYPE_INT, TYPE_BOOL, TYPE_TIMEVALUE, TYPE_RADIOBTN_INDEX, TYPE_INDEX.  */
		int					i;
		/*! This is used by: TYPE_FLOAT, TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_WORLD, TYPE_COLOR_CHANNEL.  */
		float				f;
		/*! This is used by: TYPE_POINT3, TYPE_RGBA, TYPE_HSV.  */
		Point3*				p;
		/*! This is used by: TYPE_POINT4, TYPE_FRGBA.  */
		Point4*				p4;
		/*! This is used by TYPE_TIMEVALUE.  */
		TimeValue			t;
		/*! This is used by TYPE_FILENAME or TYPE_STRING.  */
		MCHAR*				s;
		/*! This is used by TYPE_BITMAP.  */
		PBBitmap*			bm;
		/*! A generic reference target pointer (Mtl*, Texmap*, INode*).  */
		ReferenceTarget*	r;
		// new for R4
		/*! This is used by TYPE_MATRIX3.  Note that TYPE_MATRIX3 items cannot be animated.   */
		Matrix3*			m;
		/*! This value replaces the i, f, p, p4, or t values if they are actually animated (and thus have a controller assigned).  */
		Control*			control;  // replaces i,f,p or t values if animated
		/*! This is used by: TYPE_POINT2.  */
		Point2*				p2;
	};
	/*! These flags are for internal use only, do not alter them.  */
	BYTE flags;

	PB2Export BOOL is_constant();
	PB2Export void Free(ParamType2 type);
};

#pragma pack(pop,parameter_entry)

// defines a parameter alias
/*! \sa Class IParamBlock2, Class ParamBlockDesc2.
\remarks This stucture provides information about a parameter alias. Aliases 
allow individual parameters or Tab\<> parameter elements to be named. 
See the methods IParamBlock2::DefineParamAlias, FindParamAlias, etc.
*/
struct ParamAlias: public MaxHeapOperators {
	/*! The name of the alias.  */
	MCHAR*	alias;
	/*! The permanent ID of the parameter.  */
	ParamID	ID;
	/*! If the parameter is a Tab\<> this is the zero based index into
	the table of the parameter. If the parameter is not a table this is -1.  */
	int		tabIndex;
};

// the interface to a ParamBlock2
/*! \sa  Class ReferenceTarget, Class ParamBlockDesc2, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>,
Structure ParamDef, Class PBValidator, Class PBAccessor, Class PBBitmap, Class ParamBlock2PLCB, Class ClassDesc2, Class Animatable, Structure ParamAlias.\n\n
\par Description:
This class provides an interface for working with parameter block2s. There are
methods for getting and setting parameters, descriptor access, parameter map
access, etc.
\par Method Groups:
See <a href="class_i_param_block2_groups.html">Method Groups for Class IParamBlock2</a>.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class IParamBlock2 : public ReferenceTarget 
{
	public:
		/*! \remarks Returns the version of this parameter block. */
		virtual DWORD		GetVersion()=0;
		/*! \remarks Returns the number of parameters in this parameter block.
		*/
		virtual int			NumParams()=0;
		/*! \remarks Returns the localized name for the parameter block. */
		virtual MCHAR*		GetLocalName()=0;
		// acquire & release the descriptor for this paramblock, get individual paramdefs
		/*! \remarks Aquires the descriptor for this parameter block. Call
		<b>ReleaseDesc()</b> when done. */
		virtual ParamBlockDesc2* GetDesc()=0;
		/*! \remarks Releases the descriptor for this parameter block. See
		<b>GetDesc()</b> above. */
		virtual void		ReleaseDesc()=0;
		/*! \remarks Sets the descriptor associated with the parameter block.
		\param desc Points to the descriptor to set. */
		virtual void		SetDesc(ParamBlockDesc2* desc)=0;
		/*! \remarks Returns a reference to the ParamDef structure for this
		parameter block.
		\param id The parameter ID. */
		virtual ParamDef&	GetParamDef(ParamID id)=0;
		// access block ID
		/*! \remarks Returns the BlockID of the parameter block. Note:
		<b>typedef short BlockID;</b> */
		virtual BlockID		ID()=0;
		// index-to/from-ID conversion
		/*! \remarks Returns the zero based index of the parameter into the
		parameter definitions array of the given parameter ID or -1 if not
		found.
		\param id The parameter ID whose index to return. */
		virtual int			IDtoIndex(ParamID id)=0;
		/*! \remarks Returns the parameter ID of the parameter given its index into
		the parameter definitions array.
		\param i The index of the parameter whose ID is to be returned. */
		virtual ParamID		IndextoID(int i)=0;
		// get object that owns this block
		/*! \remarks Returns a pointer to the owner of this parameter block.
		*/
		virtual ReferenceMaker* GetOwner()=0;

		// Get's the super class of a parameters controller
		/*! \remarks Returns the Super Class ID of the parameter's controller
		(specified by sub-anim number).
		\param anim The sub-anim index of the parameter. */
		virtual SClass_ID GetAnimParamControlType(int anim)=0;
		/*! \remarks Returns the Super Class ID of the parameter's controller
		(specified by paramter ID).
		\param id The ID of the parameter. */
		virtual SClass_ID GetParamControlType(ParamID id)=0;
		// Get the param type & name
		/*! \remarks Returns the type of the specified parameter.
		\param id The ID of the parameter. */
		virtual ParamType2 GetParameterType(ParamID id)=0;
		/*! \remarks Returns the local name for the specified parameter or Tab\<\>
		parameter entry.
		\param id The permanent ID of the parameter.
		\param tabIndex  If the parameter is a table this is the zero based index 
		into the table of the parameter. */
		virtual MSTR GetLocalName(ParamID id, int tabIndex = -1)=0; 

		// parameter accessors, one for each known type
		/*! \remarks Sets the floating point value of the specified parameter 
		at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index 
		into the table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_FLOAT, TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_WORLD, TYPE_COLOR_CHANNEL
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, float v, int tabIndex=0)=0;
		/*! \remarks Sets the integer value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_INT, TYPE_BOOL, TYPE_TIMEVALUE, TYPE_RADIOBTN_INDEX, TYPE_INDEX
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, int v, int tabIndex=0)=0;		
		/*! \remarks Sets the Point3 value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Point3& v, int tabIndex=0)=0;		
		/*! \remarks Sets the Point4 value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Point4& v, int tabIndex=0)=0;		
		/*! \remarks Sets the Color value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Color& v, int tabIndex=0)=0;  // uses Point3 controller
		/*! \remarks Sets the AColor value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, AColor& v, int tabIndex=0)=0;  // uses Point4 controller
		/*! \remarks Sets the string value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_FILENAME or TYPE_STRING
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, const MCHAR* v, int tabIndex=0)=0;
		/*! \remarks Sets the Mtl* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_MTL
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Mtl*	v, int tabIndex=0)=0;
		/*! \remarks Sets the Texmap* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_TEXMAP
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Texmap* v, int tabIndex=0)=0;
		/*! \remarks Sets the PBBitmap* value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_BITMAP
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, PBBitmap* v, int tabIndex=0)=0;
		/*! \remarks Sets the INode* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_INODE
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, INode* v, int tabIndex=0)=0;
		/*! \remarks Sets the ReferenceTarget* value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_MTL, TYPE_TEXMAP, TYPE_INODE, TYPE_REFTARG, TYPE_PBLOCK2, TYPE_OBJECT, TYPE_CONTROL
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, ReferenceTarget*	v, int tabIndex=0)=0;
		/*! \remarks Sets the IParamBlock2* value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_PBLOCK2
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, IParamBlock2* v, int tabIndex=0)=0;
		/*! \remarks Sets the Matrix3 value of the specified parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_MATRIX3
		\note TYPE_MATRIX3 items cannot be animated.
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, Matrix3& v, int tabIndex=0)=0;		

		/*! \remarks Sets the AssetId value of the specified parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to set the value.
		\param v The value to set.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to set.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no set occurs:
		TYPE_FILENAME
		*/
		virtual BOOL SetValue(ParamID id, TimeValue t, const MaxSDK::AssetManagement::AssetUser& v, int tabIndex=0)=0;		


		/*! \remarks Retrieves the floating point value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain 0.0f:
		TYPE_FLOAT, TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_WORLD, TYPE_COLOR_CHANNEL
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the integer value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain 0:
		TYPE_INT, TYPE_BOOL, TYPE_TIMEVALUE, TYPE_RADIOBTN_INDEX, TYPE_INDEX
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Point3 value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain Point3::Origin:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Point3& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Point4 value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain Point4::Origin:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Point4& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Color value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain black:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Color& v, Interval &ivalid, int tabIndex=0)=0; // uses Point3 controller
		/*! \remarks Retrieves the AColor value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the
		retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain black:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, AColor& v, Interval &ivalid, int tabIndex=0)=0; // uses Point4 controller
		/*! \remarks Retrieves the string value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved
		parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will not be modified:
		TYPE_FILENAME or TYPE_STRING
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, const MCHAR*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Mtl* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved
		parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_MTL
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Mtl*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Texmap* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_TEXMAP
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Texmap*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the PBBitmap* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_BITMAP
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, PBBitmap*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the INode* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_INODE
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, INode*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the ReferenceTarget* value of the specified parameter
		at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_MTL, TYPE_TEXMAP, TYPE_INODE, TYPE_REFTARG, TYPE_PBLOCK2, TYPE_OBJECT, TYPE_CONTROL
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, ReferenceTarget*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the IParamBlock2* value of the specified parameter
		at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of
		the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain NULL:
		TYPE_PBLOCK2
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, IParamBlock2*& v, Interval &ivalid, int tabIndex=0)=0;
		/*! \remarks Retrieves the Matrix3 value of the specified parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain IdentityMatrix:
		TYPE_MATRIX3
		\note TYPE_MATRIX3 items cannot be animated.
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, Matrix3& v, Interval &ivalid, int tabIndex=0)=0;

		/*! \remarks Retrieves the AssetUser value of the specified parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param v The value to retrieve is returned here.
		\param ivalid This is the validity interval which is updated by the validity of the retrieved parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  TRUE on success; otherwise FALSE.
		\note The ParamType must be one of the following, otherwise no get occurs and v will contain IdentityMatrix:
		TYPE_FILENAME
		*/
		virtual BOOL GetValue(ParamID id, TimeValue t, MaxSDK::AssetManagement::AssetUser& v, Interval &ivalid, int tabIndex=0)=0;

		// short cut getters for each type
		/*! \remarks Returns the Color value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Color value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of black returned:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual Color		GetColor(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the AColor value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The AColor value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of black returned:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual AColor		GetAColor(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Retrieves the Point3 value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Point3 value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of Point3::Origin returned:
		TYPE_POINT3, TYPE_RGBA, TYPE_HSV
		*/
		virtual Point3		GetPoint3(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Retrieves the Point4 value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Point4 value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of Point4::Origin returned:
		TYPE_POINT4, TYPE_FRGBA
		*/
		virtual Point4		GetPoint4(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the integer value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The integer value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of 0 will be returned:
		TYPE_INT, TYPE_BOOL, TYPE_TIMEVALUE, TYPE_RADIOBTN_INDEX, TYPE_INDEX
		*/
		virtual int			GetInt(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the floating point value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The floating point value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of 0.0f will be returned:
		TYPE_FLOAT, TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_WORLD, TYPE_COLOR_CHANNEL
		*/
		virtual float		GetFloat(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the TimeValue value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The TimeValue value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of 0 will be returned:
		TYPE_INT, TYPE_BOOL, TYPE_TIMEVALUE, TYPE_RADIOBTN_INDEX, TYPE_INDEX
		*/
		virtual TimeValue	GetTimeValue(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the string pointer value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The string pointer value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_FILENAME or TYPE_STRING
		*/
		virtual const MCHAR*		GetStr(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the Mtl* value of the specified parameter at the
		specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Mtl* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_MTL
		*/
		virtual Mtl*		GetMtl(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the Texmap* value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Texmap* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_TEXMAP
		*/
		virtual Texmap*		GetTexmap(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the PBBitmap* value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The PBBitmap* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_BITMAP
		*/
		virtual PBBitmap*	GetBitmap(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the INode* value of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The INode* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_INODE
		*/
		virtual INode*		GetINode(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the ReferenceTarget* value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The ReferenceTarget* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_MTL, TYPE_TEXMAP, TYPE_INODE, TYPE_REFTARG, TYPE_PBLOCK2, TYPE_OBJECT, TYPE_CONTROL
		*/
		virtual ReferenceTarget* GetReferenceTarget(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Returns the IParamBlock2* value of the specified
		parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The IParamBlock2* value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of NULL will be returned:
		TYPE_PBLOCK2
		*/
		virtual IParamBlock2* GetParamBlock2(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		/*! \remarks Retrieves the Matrix3 value of the specified parameter at the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The Matrix3 value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of IdentityMatrix will be returned:
		TYPE_MATRIX3
		\note TYPE_MATRIX3 items cannot be animated.
		*/
		virtual Matrix3		GetMatrix3(ParamID id, TimeValue t=0, int tabIndex=0)=0;

		/*! \remarks Returns the assetuser of the specified parameter at
		the specified time.
		\param id The permanent ID of the parameter.
		\param t The time at which to get the value.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The AssetUser value retrieved.
		\note The ParamType must be one of the following, otherwise no get occurs and a value of kInvalid will be returned:
		TYPE_FILENAME
		*/
		virtual MaxSDK::AssetManagement::AssetUser	GetAssetUser(ParamID id, TimeValue t=0, int tabIndex=0)=0;

		// and one to get the value in a PB2Value 
		/*! \remarks This methods is used for getting a parameter value as a
		PB2Value reference.
		\param id The permanent ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table of the value to get.
		\return  The PB2Value value retrieved.
		*/
		virtual PB2Value& GetPB2Value(ParamID id, int tabIndex=0)=0;


		// parameter Tab management
		/*! \remarks Returns the number of entries being used in the table.
		\param id The permanent ID of the parameter. */
		virtual int		Count(ParamID id)=0;
		/*! \remarks Set the number of elements in the table that are actually
		used to zero.
		\param id The permanent ID of the parameter. */
		virtual void	ZeroCount(ParamID id)=0;
		/*! \remarks Set the number of elements in the table that are actually
		used to <b>n</b>.
		\param id The permanent ID of the parameter.
		\param n The number of elements to set. */
		virtual void	SetCount(ParamID id, int n)=0;
		/*! \remarks List-type delete of <b>num</b> elements starting with
		<b>start</b>
		\param id The permanent ID of the parameter.
		\param start The start position for element deletion.
		\param num The number of elements to delete.
		\return  Returns the number of items left in the table. */
		virtual int		Delete(ParamID id, int start,int num)=0; 
		/*! \remarks Changes the number of allocated items to <b>num</b>.
		\param id The permanent ID of the parameter.
		\param num The new size of the table.
		\return  Nonzero if the array was resized; otherwise 0. */
		virtual int		Resize(ParamID id, int num)=0;
		/*! \remarks Reallocate so there is no wasted space.
		\param id The permanent ID of the parameter. */
		virtual void	Shrink(ParamID id)=0;
		/*! \remarks Sorts the array using the compare function.
		\param id The permanent ID of the parameter.
		\param cmp Type of function to pass to <b>Sort()</b>. Note: <b>Sort()</b> just
		uses the C library <b>qsort</b> function. The developer must implement
		the CompareFnc function.\n\n
		<b>typedef int( __cdecl *CompareFnc) (const void *elem1, const void
		*elem2);</b>\n\n
		The return value of <b>CompareFnc</b> is show below in the relationship
		of <b>elem1</b> to <b>elem2</b>:\n\n
		\< 0\n\n
		if <b>elem1</b> less than <b>elem2</b>\n\n
		0\n\n
		if <b>elem1</b> identical to <b>elem2</b>\n\n
		\> 0\n\n
		if <b>elem1</b> greater than <b>elem2</b>
		\par Sample Code:
		\code
		static int CompTable( const void *elem1, const void *elem2 )
		{
			MCHAR *a = (MCHAR *)elem1;
			MCHAR *b = (MCHAR *)elem2;
			return(_tcscmp(a,b));
		}
		\endcode */
		virtual void	Sort(ParamID id, CompareFnc cmp)=0;
		// Tab Insert for each type
		/*! \remarks Insert <b>num</b> float elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, float* el)=0;
		/*! \remarks Insert <b>num</b> Point3* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, Point3** el)=0;
		virtual int		Insert(ParamID id, int at, int num, Point4** el)=0;
		/*! \remarks Insert <b>num</b> Color* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, Color** el)=0;
		virtual int		Insert(ParamID id, int at, int num, AColor** el)=0;
		/*! \remarks Insert <b>num</b> TimeValue elements at position
		<b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, TimeValue* el)=0;
		/*! \remarks Insert <b>num</b> string (MCHAR*) elements at position
		<b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param vel Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, MCHAR** vel)=0;
		/*! \remarks Insert <b>num</b> string (const MCHAR*) elements at position
		<b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param vel Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, const MCHAR** vel)=0;
		/*! \remarks Insert <b>num</b> Mtl* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, Mtl** el)=0;
		/*! \remarks Insert <b>num</b> Texmap* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, Texmap** el)=0;
		/*! \remarks Insert <b>num</b> PBBitmap* elements at position
		<b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, PBBitmap** el)=0;
		/*! \remarks Insert <b>num</b> INode* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param v Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, INode** v)=0;
		/*! \remarks Insert <b>num</b> ReferenceTarget* elements at position
		<b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, ReferenceTarget** el)=0;
		virtual int		Insert(ParamID id, int at, int num, IParamBlock2** el)=0;
		/*! \remarks Insert <b>num</b> Matrix3* elements at position <b>at</b>.
		\param id The permanent ID of the parameter.
		\param at Zero based array index where to insert the elements.
		\param num Number of elements to insert.
		\param el Array of elements to insert.
		\return  Returns <b>at</b>. */
		virtual int		Insert(ParamID id, int at, int num, Matrix3** el)=0;
		// Tab Insert for each type
		/*! \remarks Append <b>num</b> float elements at the end of the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, float* el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> Point3* elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, Point3** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Point4** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> Color* elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, Color** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, AColor** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> TimeValue elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, TimeValue* el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> string (MCHAR*) elements at the end of
		the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, MCHAR** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> string (const MCHAR*) elements at the end of
		the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, const MCHAR** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> Mtl* elements at the end of the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, Mtl** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> Texmap* elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, Texmap** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> PBBitmap* elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  The number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, PBBitmap** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> INode* elements at the end of the
		array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, INode** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> ReferenceTarget* elements at the end of
		the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, ReferenceTarget** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, IParamBlock2** el, int allocExtra=0)=0;
		/*! \remarks Append <b>num</b> Matrix3* elements at the end of the array.
		\param id The permanent ID of the parameter.
		\param num The number of elements to append to the end of the array.
		\param el The elements to append.
		\param allocExtra If you need to enlarge the array specify an non-zero value and this
		many extra slots will be allocated.
		\return  Returns the number of elements in use prior to appending. */
		virtual int		Append(ParamID id, int num, Matrix3** el, int allocExtra=0)=0;

		/*! Checks to see if a keyframe exists for the given parameter at the given time.
		\param i Zero based index of the parameter to check.
		\param t The time to check.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table to check.
		\return  TRUE if a keyframe exists at the specified time; otherwise
		FALSE. */
		virtual BOOL KeyFrameAtTimeByIndex(int i,      TimeValue t, int tabIndex=0) { return FALSE; }
		//! \deprecated Deprecated in 3ds Max 2012. Use KeyFrameAtTimeByIndex or instead.
		MAX_DEPRECATED BOOL KeyFrameAtTime(int i,      TimeValue t, int tabIndex=0);
		/*! Checks to see if a keyframe exists for the given parameter at the given time. 
		Note this method is slower than KeyFrameAtTimeByIndex.
		\param id The permanent ID of the parameter.
		\param t The time to check.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table to check.
		\return  TRUE if a keyframe exists at the specified time; otherwise
		FALSE. */
		virtual BOOL KeyFrameAtTimeByID(ParamID id, TimeValue t, int tabIndex=0) { return KeyFrameAtTimeByIndex(IDtoIndex(id), t, tabIndex); }
		//! \deprecated Deprecated in 3ds Max 2012. Use KeyFrameAtTimeByID instead.
		MAX_DEPRECATED BOOL KeyFrameAtTime(ParamID id, TimeValue t, int tabIndex=0);
		/*! Removes the 'i-th' controller.
		\param i Specifies which controller using the zero based index of the parameter
		in the block.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table whose controller is removed. */
		virtual void RemoveControllerByIndex(int i, int tabIndex)=0;
		//! \deprecated Deprecated in 3ds Max 2012. Use RemoveControllerByIndex instead.
		MAX_DEPRECATED void RemoveController(int i, int tabIndex);
		/*! Gets a pointer to the controller of the specified parameter.
		Note this method is slower than GetControllerByIndex.
		\param id The permanent ID of the parameter.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table whose controller is returned. */
		virtual Control*	GetControllerByID(ParamID id, int tabIndex=0)=0;
		//! \deprecated Deprecated in 3ds Max 2012. Use GetControllerByID instead.
		MAX_DEPRECATED Control*	GetController(ParamID id, int tabIndex=0);
		/*! Gets a pointer to the controller of the specified parameter.
		\param i Specifies which controller using the zero based index of the parameter
		in the block.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table whose controller is returned. */
		virtual Control*	GetControllerByIndex(int i, int tabIndex=0)=0;
		//! \deprecated Deprecated in 3ds Max 2012. Use GetControllerByIndex instead.
		MAX_DEPRECATED Control*	GetController(int i, int tabIndex=0);
		/*! Sets the indexed parameter controller to the one specified.
		\param i Specifies which controller using the zero based index of the parameter
		in the block.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table.
		\param c The controller to set.
		\param preserveFrame0Value If TRUE the controllers value at frame 0 is preserved. */
		virtual void		SetControllerByIndex(int i, int tabIndex, Control* c, BOOL preserveFrame0Value=TRUE)=0;
		/*! Sets the parameter with the given id to the specified controller.
		Note that	this is slower than calling SetControllerByIndex.
		\param id The parameter id of the controller to be set.
		\param tabIndex If the parameter is a table this is the zero based index of the element
		in the table.
		\param c The controller to set.
		\param preserveFrame0Value If TRUE the controllers value at frame 0 is preserved. */
		virtual void		SetControllerByID(ParamID id, int tabIndex, Control* c, BOOL preserveFrame0Value=TRUE) { SetControllerByIndex(IDtoIndex(id), tabIndex, c, preserveFrame0Value); }
		//! \deprecated Deprecated in 3ds Max 2012. Use SetControllerByIndex instead.
		MAX_DEPRECATED void SetController(int i, int tabIndex, Control *c, BOOL preserveFrame0Value=TRUE);
		//! \deprecated Deprecated in 3ds Max 2012. Use SetControllerByID instead.
		MAX_DEPRECATED void SetController(ParamID id, int tabIndex, Control *c, BOOL preserveFrame0Value=TRUE);
		/*! Swaps the two controllers of the parameters whose indices are passed.
		\param i1 The zero based index of one of the parameters in the parameter block.
		\param tabIndex1 If the parameter is a table this is the zero based index of the element in the table.
		\param i2 The zero based index of one of the other parameters in the parameter block.
		\param tabIndex2 If the parameter is a table this is the zero based index of the element in the table. */
		virtual	void		SwapControllers(int i1, int tabIndex1, int i2, int tabIndex2)=0;

		// Given the param num & optional Tab<> index, what is the refNum?
		/*! \remarks Given a parameter index this method will return the
		reference number of that parameter.
		\param i The zero based index of the parameter in the parameter block.
		\param tabIndex If the parameter is a table this is the zero based index of the element in the table. */
		virtual	int GetRefNum(int i, int tabIndex=0)=0;
		// Given the param num & optional Tab<> index, what is the animated param controller refnum?
		/*! \remarks Returns the reference number of the specified parameter's
		controller or -1 if not found.
		\param i The zero based index into the parameter definitions array of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index in the table of the parameter. */
		virtual	int GetControllerRefNum(int i, int tabIndex=0)=0;

		// Given the parameter ID what is the animNum?
		/*! \remarks Returns the sub-anim number of the parameter whose ID is
		passed or -1 if not found.
		\param id The parameter ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index in the table of the parameter. */
		virtual	int GetAnimNum(ParamID id, int tabIndex=0)=0;

		// Given the animNum what is the parameter index?
		/*! \remarks Returns the index into the parameter definitions array of
		the parameter whose sub-anim index is specified or -1 if not found.
		\param animNum The zero based sub-anim index of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of the parameter. */
		virtual	int AnimNumToParamNum(int animNum, int& tabIndex)=0;

		/*! \remarks Returns the dimension of the parameter whose sub-anim
		index is passed or <b>defaultDim</b> if not found.
		\param subAnim The zero based sub-anim index of the parameter. */
		virtual	ParamDimension* GetParamDimension(int subAnim)=0;

		// This is only for use in a RescaleWorldUnits() implementation:
		// The param block implementation of RescaleWorldUnits scales only tracks
		// that have dimension type = stdWorldDim. If letting the param block handle 
		// the rescaling is not sufficient, call this on just the parameters you need to rescale.
		/*! \remarks This is only for use in a RescaleWorldUnits()
		implementation: The parameter block implementation of RescaleWorldUnits
		scales only tracks that have dimension type = stdWorldDim. If letting
		the parameter block handle the rescaling is not sufficient, call this
		on just the parameters you need to rescale.
		\param paramNum The index into the parmeter block of the parameter to rescale.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the 
		table of the parameter.
		\param f The value to scale by. */
		virtual void RescaleParam(int paramNum, int tabIndex, float f)=0;

		// When a NotifyRefChanged is received from a param block, you 
		// can call this method to find out which parameter generated the notify.
		virtual ParamID LastNotifyParamID()=0;
		/*! \remarks Like <b>LastNotifyParamID()</b>, but takes an <b>int\& tabIndex</b>
		argument so that it can return both the ID of the changing parameter
		(as the result) and the changing element index for <b>Tab\<\></b>
		parameters.\n\n
		If the <b>ParamID</b> returns -1 because no parameter is currently
		changing, <b>tabIndex</b> is not updated. If the change to a
		<b>Tab\<\></b> parameter is not to a single element (such as a sort),
		the <b>tabIndex</b> is set to -1. For multiple inserts, appends,
		deletes, the <b>tabIndex</b> returned is the index of the first element
		inserted, appended, deleted.
		\param tabIndex The index of the changing element for <b>Tab\<\></b> parameters is returned here. */
		virtual ParamID LastNotifyParamID(int& tabIndex)=0;  // variant also returns changing element index
		
		// control notifications, enable/disable send NotifyRefChanged messages when parameters change (via SetValue(), eg)
		/*! \remarks Controls whether <b>NotifyDependents()</b> messages are sent when a
		parameter is changed, such as through\n\n
		a <b>SetValue()</b> call. For example:\n\n
		\code
		pblock->EnableNotifications(FALSE);
		... param change code ...
		pblock->EnableNotifications(TRUE);
		\endcode 
		Notifications are enabled by default. Note that this is a GLOBAL
		enable/disable, ALL paramblocks will be prevented from sending
		notifications while <b>EnableNotifications(FALSE)</b> is in effect.
		\param onOff TRUE to enable notifications, FALSE to disable them. */
		virtual void EnableNotifications(BOOL onOff)=0;

		/*! \brief This method returns a BOOL value to indicate whether the notifications are globally enabled.
		\return TRUE if the notifications are enabled, FALSE otherwise.
		*/
		virtual BOOL IsNotificationEnabled() = 0;

		// allows owner to signal pblock when P_OWNER_REF params are deleted
		/*! \remarks This method should be called when the parameter block
		owner has deleted the reference to a reference target parameter. This
		sets the value to NULL and invalidates the UI associated with the
		pblock. Note that this must only be called on <b>P_OWNERS_REF</b>
		parameters.
		\param id The ID of the reference target parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the table of the parameter. */
		virtual void RefDeleted(ParamID id, int tabIndex=0)=0;

		// ParamMap2 access, 
		/*! \remarks This method sets the parameter map2 associated with this
		parameter block2.
		\param m Points to the parameter map.
		\param map_id Specifies the ID of the map to set. */
		virtual void SetMap(IParamMap2* m, MapID map_id = 0)=0;
		/*! \remarks Returns a pointer to the parameter map2 associated with
		this parameter block.
		\param map_id Specifies the ID of the map to get. */
		virtual IParamMap2* GetMap(MapID map_id = 0)=0;
		// rollout state, normally used by ParamMap2 to automatically save & restore state 
		/*! \remarks Sets the rollout state to open or closed.\n\n
		Note: Normally, developers don't need to call this method (or the
		related ones below) explicitly; they are used internally to keep track
		of rollouts states. Instead, use the <b>ClassDesc2</b> method
		<b>RestoreRolloutState()</b> at the end of a <b>BeginEditParams()</b>
		or <b>CreateParamDlg()</b> to reset the rollouts to the state last used
		for the current object.
		\param open TRUE for open; FALSE for closed.
		\param map_id Specifies the ID of the map/rollout to set open/closed state for. */
		virtual void SetRolloutOpen(BOOL open, MapID map_id = 0)=0;
		/*! \remarks Returns TRUE if the rollout is open; FALSE if closed.
		This is normally used internally -- see the note above in
		<b>SetRolloutOpen()</b>.
		\param map_id Specifies the ID of the map/rollout to get open/closed state for. */
		virtual BOOL GetRolloutOpen(MapID map_id = 0)=0;
		/*! \remarks Sets the rollout scroll position. This is normally used
		internally -- see the note above in <b>SetRolloutOpen()</b>.
		\param pos The position to set.
		\param map_id Specifies the ID of the map/rollout to set scroll position for. */
		virtual void SetRolloutScrollPos(int pos, MapID map_id = 0)=0;
		/*! \remarks Returns the rollout scroll position. This is normally
		used internally -- see the note above in <b>SetRolloutOpen()</b>.
		\param map_id Specifies the ID of the map/rollout to get scroll position for. */
		virtual int GetRolloutScrollPos(MapID map_id = 0)=0;

		// ParamDlg access, 
		/*! \remarks Returns a pointer to the automatic parameter dialog
		object for a plug-in material or texmap (which has its interface in the
		materias editor). See Class IAutoMParamDlg. */
		virtual IAutoMParamDlg* GetMParamDlg()=0;
		/*! \remarks Returns a pointer to the automatic parameter dialog
		object for the rendering effects plug-in. See
		Class IAutoEParamDlg. */
		virtual IAutoEParamDlg* GetEParamDlg()=0;

		// init parameters with MAXScript defaults
		/*! \remarks This method initializes the parameters with MAXScript
		defaults.\n\n
		The <b>ParamBlockDesc2</b> descriptor lets you specify default values
		for parameters (using the tag <b>p_default</b>), and these get
		installed when you first create an object and its paramblocks.
		Sometimes, the default value needed for interactive creation is not the
		one you want when creating an object via the scripter. For example, a
		sphere should start out with radius 0 when you create it interactively,
		but you want a non-zero default if you create it in the scripter, say
		with sphere(), otherwise it would be invisible. There is another tag,
		p_ms_default, that lets you set a separate default for scripter-based
		creation (the <b>p_ms_default</b> for sphere radius is 25). This method
		is used internally by the scripter to set the <b>p_ms_default</b>
		values in after a script-based creation. It is not normally used by
		plug-in developers. */
		virtual void InitMSParameters()=0;

		// alias maintenance
		/*! \remarks This is used to allow parameter 'aliases' to be set up
		for MAXScript use. Individual Tab\<\> parameter elements can have
		aliases. This is used to set up dynamically-varying parameters, such as
		the texture maps in the new Standard material, which sets up aliases
		for elements in the texture map arrays.
		\param alias_name The name of the alias.
		\param id The permanent ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index of the parameter in the table. */
		virtual void DefineParamAlias(MCHAR* alias_name, ParamID id, int tabIndex=-1)=0;
		/*! \remarks Returns a pointer to a
		Structure ParamAlias object which
		describes the paramater alias whose name is passed. This includes the
		name, ParamID and Tab\<\> index.
		\param alias_name The name of the alias to find. */
		virtual ParamAlias* FindParamAlias(MCHAR* alias_name)=0;
		/*! \remarks Finds the name of a parmameter alias using the ID and
		Tab\<\> index passed.
		\param id The permanent ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> this is the zero based index into the
		table. If not a Tab\<\> use the default of -1.
		\return  The name of the alias or NULL if not found. */
		virtual MCHAR* FindParamAlias(ParamID id, int tabIndex=-1)=0;
		/*! \remarks Breaks the association between the aliases and their
		parameters in this block. The method <b>ParamAliasCount()</b> below
		will return 0 following this call. */
		virtual void ClearParamAliases()=0;
		/*! \remarks Returns the number of aliases currently defined. */
		virtual int ParamAliasCount()=0;
		/*! \remarks Returns a pointer to the 'i-th' alias. See
		Structure ParamAlias.
		\param i The zero based index of the alias to return. */
		virtual ParamAlias* GetParamAlias(int i)=0;

		// set subanim number for given param
		/*! \remarks This method allows for the arbitrary ordering of sub-anim
		numbers for parameters and Tab\<\> parameter elements. It sets the
		sub-anim number for the specified parameter.\n\n
		This call lets you set arbitrary sub-anim number ordering for the
		subAnim parameters and Tab\<\> parameter elements in the block. You
		must set numbers for ALL subAnims and take care that all numbers are
		used. Note that in the case of the various ReferenceTarget* parameter
		types, NULL values for any parameter or Tab\<\> elements are <b>not</b>
		included the subAnim count, so if such a parameter is made non-NULL (or
		vice-versa), you need to reassign the subanim numbers to take that
		change into account.
		\param id The permanent parameter ID
		\param subAnimNum The zero based sub-anim number.
		\param tabIndex The zero based index into the table of the parameter. */
		virtual void SetSubAnimNum(ParamID id, int subAnimNum, int tabIndex=0)=0;
		/*! \remarks This method clears any sub-anim map used to allow
		arbitrary ordering of sub-anim numbers for parameters. */
		virtual void ClearSubAnimMap()=0;

		// parameter value copying, copy src_id param from src block to id param in this block
		/*! \remarks This method is used for copying parameter values between
		parameter blocks (which is useful during old-version updating). This
		method copies from the 'src' block 'src_id' parameter into this
		parameter block's 'id' parameter. Developers are responsible for making
		sure the types are the same, otherwise an assert() may occur.
		\param id This ID specifies the destination parameter.
		\param src Points to the source parameter block 2.
		\param src_id The source parameter ID. */
		virtual void Assign(ParamID id, IParamBlock2* src, ParamID src_id)=0;

		// find the param ID & tabIndex for given ReferenceTarget(either as a subanim or reftarg parameter)
		/*! \remarks This method that takes a reference target object
		<b>ref</b> stored somewhere in this parameter block and returns the
		ParamID and tabIndex of the containing parameter, or -1 if not found in
		the parameter block.
		\param ref The reference target to find.
		\param tabIndex The table index if the parameter is a Tab\<\>. */
		virtual ParamID FindRefParam(ReferenceTarget* ref, int& tabIndex)=0;

		// reset params to default values
		/*! \remarks This method resets all the parameters in the block to
		their default values and optionally updates any associated ParamMap2 UI
		that is currently displaying the contents of the block. It also
		optionally causing all the PBAccessor Set() methods to be called after
		the reset.
		\param updateUI  TRUE to update the user inteface; FALSE to not update.
		\param callSetHandlers  TRUE to call <b>PBAccessor::Set()</b> for all the parameters; otherwise FALSE. */
		virtual void ResetAll(BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE)=0;
		/*! \remarks This method resets the single parameter specified to its
		default value. If the parameter is a Tab\<\> and the tabIndex is -1,
		all the elements in the table are reset.
		\param id The ID of the parameter to reset.
		\param tabIndex If the parameter is a Tab\<\> this is the index into the table of the
		parameter to reset. A value of -1 causes all the elements in the table to be reset.
		\param updateUI  Determines if the user inteface is updated for the parameter. TRUE to
		update; FALSE to not update.
		\param callSetHandlers  Determines if the method <b>PBAccessor::Set()</b> should be called on
		the parameter. TRUE to call it; FALSE to not call it. */
		virtual void Reset(ParamID id, int tabIndex=-1, BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE)=0;
		// force a call to the PBAccessor Get()/Set() functions for a param or all params
		/*! \remarks This method forces a call to the <b>PBAccessor::Set()</b>
		method for the specified parameter. If the parameter is a Tab\<\>
		parameter and the tabIndex is -1, all the elements have the appropriate
		functions called.
		\param id The ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> parameter this is the zero based index
		into the table of the element. A value of -1 causes all the appropriate
		<b>Set()</b> methods to be called. */
		virtual void CallSet(ParamID id, int tabIndex=-1)=0;
		/*! \remarks This method forces a call to the <b>PBAccessor::Get()</b>
		method for the specified parameter. If the parameter is a Tab\<\>
		parameter and the tabIndex is -1, all the elements have the appropriate
		functions called.
		\param id The ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> parameter this is the zero based index
		into the table of the element. A value of -1 causes all the appropriate
		<b>Get()</b> methods to be called. */
		virtual void CallGet(ParamID id, int tabIndex=-1)=0;
		/*! \remarks This method forces a call to the <b>PBAccessor::Set()</b>
		method for every parameter in the block. Any parameters which are
		Tab\<\> parameters will have <b>Set()</b> call for every appropriate
		element. */
		virtual void CallSets()=0;
		/*! \remarks This method forces a call to the <b>PBAccessor::Get()</b>
		method for every parameter in the block. Any parameters which are
		Tab\<\> parameters will have <b>Get()</b> call for every appropriate
		element. */
		virtual void CallGets()=0;
		// get validity of all params in all paramblock
		/*! \remarks This method updates the validity interval passed with the
		cumulative interval for every parameter in the parameter block.
		\param t The time about which the interval is computed.
		\param valid The interval to update. */
		virtual void GetValidity(TimeValue t, Interval &valid)=0;
};

// specialize this class to provide a custom SetValue() validator
// the Validate() function should return falsi if the given PB2Value is
// not valid
/*! \sa  Structure ParamDef, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of Type 2 Params</a>,
Class ParamBlockDesc2.\n\n
\par Description:
A pointer to an instance of this class is a data member of struct
<b>ParamDef</b> and is also used by the <b>p_validator</b> tag in a
<b>ParamBlockDesc2</b> constructor. Any parameter can have a custom validator.
This is used by the scripter and node pick button filter for example. If you
want to validate a parameter block 2 value create an instance of this class and
implement the <b>Validate()</b> method.  */
class PBValidator : public InterfaceServer
{
public:
	/*! \remarks Returns TRUE if the given PB2Value if valid; otherwise FALSE.
	\par Parameters:
	<b>PB2Value\& v</b>\n\n
	The value to check. */
	virtual BOOL Validate(PB2Value& v) = 0;
	/*! \remarks	A variant of <b>Validate</b>() method to <b>PBValidate</b> which supplies
	more context than the original <b>Validate</b>(), effectively giving the
	same context information as the <b>Set()</b> \& <b>Get()</b> methods in a
	<b>PBAccessor</b>.
	\par Parameters:
	<b>PB2Value\& v</b>\n\n
	The value to check.\n\n
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index of the parameter
	in the table. */
	virtual BOOL Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex) { return this->Validate(v); }
	/*! \remarks This method that can be used to destroy dynamically allocated
	instances of this class.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void DeleteThis() { };
};

// specialize this class to provide 'virtual' parameter value accessor functions
/*! \sa  Structure ParamDef, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>,
Class ReferenceMaker, Structure PB2Value.\n\n
\par Description:
Any parameter in a block can have an accessor callback object that has its
<b>Get()</b> or <b>Set()</b> method called whenever the parameter is accessed.
This may be used to provide access to dynamically-computed virtual parameters
and sometimes to allow parameter-specific processing by the class as the
parameter in the block is modified (such as keeping object data members
up-to-date).\n\n
The <b>Get()</b> and <b>Set()</b> methods are called at all times when a
parameter is accessed, including parameters that are animated. The <b>Get()</b>
method is called after the controller is accessed, so the current controller
value is seen and can be optionally overridden in the <b>Get()</b> method. Note
that the controller is accessed whenever the 3ds Max time is changed (such as a
frame slider move) and so the <b>Get()</b> method will be called each frame as
this happens.\n\n
A pointer to an instance of this class is a data member of the <b>ParamDef</b>
structure.  */
class PBAccessor : public InterfaceServer
{
public:
	// get into v
	/*! \remarks This method is called when the specified owner object
	accesses (gets) the value v. This is called after the controller is
	accessed, so the current controller value is seen and can be optionally
	overridden in the this method. Note that the controller is accessed
	whenever the 3ds Max time is changed (such as a slider move) and so this
	method will be called each frame as this happens.
	\par Parameters:
	<b>PB2Value\& v</b>\n\n
	The value being accessed.\n\n
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index of the parameter
	in the table.\n\n
	<b>TimeValue t</b>\n\n
	The current time the get is taking place.\n\n
	<b>Interval \&valid</b>\n\n
	The validity interval for the value.
	\par Default Implementation:
	<b>{}</b> */
	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) { }    
	// set from v
	/*! \remarks This method is called when the specified owner objects sets the
	value v. This is called just before calling <b>SetValue()</b> on the parameters
	controller, so it can take note of the value going in and change it if desired.
	\par Parameters:
	<b>PB2Value\& v</b>\n\n
	The value being set.\n\n
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index of the parameter in
	the table.\n\n
	<b>TimeValue t</b>\n\n
	The current time the set is taking place.
	\par Default Implementation:
	<b>{}</b> */
	virtual void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) { }						 
	// computed keyframe presence
	/*! \remarks Checks to see if a keyframe exists for the given parameter at
	the given time. Returns TRUE if a keyframe exists at the specified time;
	otherwise FALSE. For parameters not directly hosted in the parameter block
	that are internally animatable, this provides a keyframe query callback so
	that any ParamMap2 spinners associated with these 'virtual' parameters can
	show keyframe status for the underlying parameter. In these cases,
	developers should implement this method for the parameter usually asking
	the underlying parameter for its keyframe state.
	\par Parameters:
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL KeyFrameAtTime(ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) { return FALSE; }
	// computed parameter localized (subanim) name, only called if P_COMPUTED_NAME is flagged
	/*! \remarks This allows a plug-in to provide a dynamically-created local
	name for a parameter or Tab\<\> parameter entry. If you specify the
	<b>P_COMPUTED_NAME</b> parameter flag, you also need to suppy a
	<b>p_accessorPBAccessor</b> instance pointer that has this method
	implemented.
	\par Parameters:
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index of the parameter
	in the table.
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual MSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex) { return _M(""); }
	// called when a Tab<> parameter has a change made to its table structure
	enum tab_changes { tab_insert, tab_append, tab_delete, tab_ref_deleted, tab_setcount, tab_sort };
	/*! \remarks	This method is called when a <b>Tab\<\></b> parameter has a change made to
	its table structure.
	\par Parameters:
	<b>tab_changes changeCode</b>\n\n
	Describes the change that has just occurred to the <b>Tab\<\></b>
	parameter. One of the following enumerations:\n\n
	<b>enum tab_changes { tab_insert, tab_append, tab_delete, tab_ref_deleted,
	tab_setcount, tab_sort };</b>\n\n
	<b>Tab\<PB2Value\>* tab</b>\n\n
	Points to the actual <b>Tab\<\></b> in the pblock parameter.\n\n
	<b>ReferenceMaker* owner</b>\n\n
	Points to the owner of the parameter.\n\n
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex</b>\n\n
	The start index of the change (for <b>tab_insert, tab_append, tab_delete,
	tab_ref_deleted</b>)\n\n
	<b>int count</b>\n\n
	The number of elements changed (for <b>tab_insert, tab_append,
	tab_delete</b>). */
	virtual void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count) { }
	// implement this if your PBAccessors are dynamically allocated
	/*! \remarks This method that can be used to destroy dynamically allocated
	instances of this class.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void DeleteThis() { };
};

#pragma warning(pop)
// Parameter Block Descriptors
//    in PB2s, there is one ParamBlockDesc2 per entire PB, containing
//    the metadata for all the parameters in the PB.  All the PBs
//    mapped by this structure contain pointers back to it and the 
//    owning class's ClassDesc contains all the PB2Descs for PBs in
//    its objects

// HEY!  for the moment, all the possible optional parameters are in extensis.  If
//       this proves a big memory hog, we can institute some kind of streaming
//       scheme that packs used optionals into a single mem buffer.  The GetDesc() 
//       function should be used to access the PBD in all cases to allow this caching.
//       sizeof(ParmDef) ~ 70 bytes, so for 2500 params =~ 175K bytes
 
#pragma pack(push, parameter_def)
// #pragma pack(1)   // this messes up Win64 builds & the GreatCircle memory debugger

/*! \sa Class ParamBlockDesc2, Structure PB2Value, Class PBValidator, Class PBBitmap, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>,
Class ParamDimension, Template Class Tab.
\remarks The data members of this class provide a definition of a parameter.
An array of these parameter definitions is a data member of class ParamBlockDesc2.
*/
struct ParamDef: public MaxHeapOperators
{
	DWORD size;
	//public:
	/*! This is the permanent, position independent ID of the parameter.  */
	ParamID		ID;				// pos independent ID
	/*! This is a fixed internal name of the parameter. This name is not localized.
	Internal names are meant to be parsable as identifiers. As such they should
        begin with an alpha character, have only alphanumerics, and have no spaces, punctuations, etc.
	The convention for multi-word names is to use studly-caps, eg, paintRadius. */
	MCHAR*		int_name;		// fixed internal name
	/*! This is the type of the parameter. See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType2 Choices</a>.  */
	ParamType2	type;			// parameter type
	/*! They are the per-parameter constructor flags (P_ANIMATABLE, P_TRANSIENT, etc.)
        Normally, the flags are set up as a result of things you specify in the
        ParamBlockDesc2 constructor and should generally be read-only at runtime.  */
	ULONGLONG	flags;			// status flags
	// optional
	/*! This is the string table resource ID for the localized (sub-anim) name.  */
	StringResID	local_name;		// localized (subabim) name (string res id)
	/*! This is the parameter dimension. See Class ParamDimension.  */
	ParamDimension* dim;		// parameter dimension
	/*! The default value for the parameter.  */
	PB2Value	def;			// default value
	/*! This is the default value for MAXScript and the MacroRecorder.  */
	PB2Value	ms_def;			// default value for MAXScript & MacroRecorder
	/*! This is the current 'sticky' default value, used to maintain creation defaults within a session.  */
	PB2Value	cur_def;		// current 'sticky' default value, used to maintain creation defaults within a session
	/*! This is a one sentence description. Use a string resource ID.  */
	int			description;	// one sentence description (string res id)
	/*! This indicates the low allowable range used in MAXScript validation and spinner setup.  */
	PB2Value	range_low;		// range values
	/*! This indicates the high allowable range used in MAXScript validation and spinner setup.  */
	PB2Value	range_high;
	/*! Points to an instance of the validator object. This object has a
        Validate() method used to check if the parameter is valid.  */
	PBValidator* validator;		// validator object
	/*! Points to an instance of an accessor object. Any parameter in a block
        can have an accessor callback object that has its Get() or Set() method
        called whenever the parameter is accessed. This may be used to provide access
        to dynamically-computed virtual parameters and sometimes to allow parameter-specific
        processing by the class as the parameter in the block is modified.  */
	PBAccessor*	accessor;		// virtual param accessor object
	/*! If the parameter is a table (Tab\<>) this is the initial table size.  */
	short		tab_size;		// initial table size
	/*! This is a block-owner's reference number for non-hosted ReferenceTargets parameters.  */
	short		ref_no;			// block-owner's refno for non-hosted ReferenceTargets
	/*! This is a block-owner's SubTex index for Texmap parameters in Mtl owners.  */
	short		subobj_no;		// block-owner's SubTex/SubMtl index for Texmap/Mtl parameters in Mtl owners
	/*! This is the Class_ID validator for reference targets.  */
	Class_ID	class_ID;		// validator for reftargs
	/*! This is the SClass_ID validator for reference targets (similar to above).  */
	SClass_ID	sclass_ID;		//    "       "     "
	// UI optional
	/*! This is the type of user interface control.  */
	ControlType2 ctrl_type;		// type of UI control
	/*! This is the spinner type if the associated UI control is a spinner.
	One of the following values may be used:
	- EDITTYPE_INT \n
	Any integer value.
	- EDITTYPE_FLOAT \n
	Any floating point value.
	- EDITTYPE_UNIVERSE \n
	This is a value in world space units. \n
	It respects the system's unit settings (for example feet and inches).
	- EDITTYPE_POS_INT \n
	Any integer >= 0
	- EDITTYPE_POS_FLOAT \n
	Any floating point value >= 0.0
	- EDITTYPE_POS_UNIVERSE \n
	This is a positive value in world space units. \n
	It respects the system's unit settings (for example feet and inches).
	- EDITTYPE_TIME \n
	This is a time value. It respects the system time settings (SMPTE for example).
        */
	EditSpinnerType spin_type;	// spinner type if spinner
	/*! This is the array of control IDs for this parameter.  */
	int*		ctrl_IDs;		// array of control IDs for this control  (or ui element names if for scripted plugin)
	/*! This is the number of controls in the ctrl_IDs array above.  */
	short		ctrl_count;		// number of controls
	/*! These are radio button vals or bit numbers for int bits controlled
        by multiple checkboxes. The numbers in the int array are used to indicate
        which bit to flip in the TYPE_INT parameter depending on the state of the
        associated (by order) checkbox. This is not yet implemented.  */
	union { // radiobutton vals or bit numbers for int bits controlled by multiple checkboxes
		int* val_bits;
		float* val_bits_float;
		};

	/*! This is the scale given to the ISpinnerControl, as is used in SetupFloatSpinner(), for example.  */
	float		scale;			// display scale
// begin - mjm 12.19.98
	/*! This is the slider segments count.  */
	int		numSegs;		// slider segments
// end - mjm 12.19.98
	/*! The array of which other parameters have their UI controls automatically enabled by this parameter.  */
	ParamID*	enable_ctrls;	// array of which other params ahave their UI ctrls automatically enabled by this param
	/*! This is the number of parameter IDs in the enable_ctrls array above.  */
	short		enable_count;	// count of enable control params
	/*! The status line prompt string resource ID for various picker buttons.  */
	int		prompt;			// status line prompt string res ID for various picker buttons
	/*! The caption string resource ID for open/save file dialogs.  */
	int		caption;		// caption string res ID for open/save file dlgs
	/*! The ToolTip string res ID.  */
	int	toolTip;
	/*! The initial filename for open/save file dialogs.  */
	MCHAR*		init_file;		// initial filename for open/save file dlgs
	/*! The file types string resource ID for open/save file dialogs (in MAXScript type: form)  */
	int		file_types;		// file types string res ID for open/save file dlgs (in MAXScript type: form)
	// new for R4
	/*! Maps IDs if in a multi-map block (block flag P_MULTIMAP)  */
	Tab<MapID>	maps;

	/*! A list of file types to be used for Open/Save dialogs */
	MCHAR *		dyn_file_types;	

	/*! parameter type (only for TYPE_FILENAME and TYPE_FILENAME_TAB)*/
	MaxSDK::AssetManagement::AssetType asset_type_id;

	/*! \remarks This function deletes this instance of the structure. */
	PB2Export void DeleteThis();
	ParamDef() { size = sizeof(ParamDef);}
};
		
#pragma pack(pop, parameter_def)

/* ----------------------- ClassDesc2 ------------------------------------*/

class MtlBase;
class SpecialFX; // mjm - 07.06.00

// 2nd Edition of ClassDesc with necessary extra stuff for ParamBlock2 support
/*! \sa  Class ClassDesc, Class Animatable, Class ParamBlockDesc2, Class IParamMap2, Class IObjParam, Class ParamMap2UserDlgProc, Class IAutoMParamDlg.\n\n
\par Description:
A subclass of ClassDesc which you specialize to provide a class descriptor for
plug-in classes that will use the ParamBlock2 system. It contains a table of
ParamBlockDesc2s for all the parameter blocks used in the plug-in and a number
of sets of methods including access to the block descriptors, auto user
interface management, auto param block2 construction, and access to any
automatically-maintained ParamMap2s.  */
class ClassDesc2 : public ClassDesc 
{
	private:
		Tab<ParamBlockDesc2*>	pbDescs;		// parameter block descriptors
		Tab<IParamMap2*>		paramMaps;		// any current param maps
		IAutoMParamDlg*			masterMDlg;		// master material/mapParamDlg if any
		IAutoEParamDlg*			masterEDlg;		// master EffectParamDlg if any

	protected:
		
		//! \brief Sets the root level material ParamDlg. 
		/*!	This method may be called by derived classes that want
			to handle the material ParamDlg creation themselves, 
			instead of using one of the "paramdlg" creation methods of class ClassDesc2.
			\param [in] dlg - The new master Material dialog pointer*/
		void SetMParamDlg(IAutoMParamDlg* dlg) { masterMDlg = dlg; }

		/*! \brief Sets the root level effect ParamDlg.
		/*!	This method may be called by derived classes that want
			to handle the effect ParamDlg creation themselves,
			instead of using one of the "paramdlg" creation methods of class ClassDesc2.
			\param [in] dlg - The new master effect dialog pointer */
		void SetEParamDlg(IAutoEParamDlg* dlg) { masterEDlg = dlg; }

		/*! \remarks Allows access to the parameter maps currently posted to the UI.
			\returns A Tab containing any current parameter maps */
		Tab<IParamMap2*>& GetParamMaps() { return paramMaps; }

	public:
		/*! \remarks Constructor.  The master ParamDlg and Effect
		ParamDlg pointers are set to NULL. */
		PB2Export				ClassDesc2();
		/*! \remarks Destructor. */
		PB2Export			   ~ClassDesc2();
		/*! \remarks This method may be called to restore all sticky
		parameters to their default values.
		\param fileReset This parameter is not used. */
		PB2Export void			ResetClassParams(BOOL fileReset);
		
		// ParamBlock2-related metadata
		// access parameter block descriptors for this class
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns the number of parameter block2 descriptors used by this plug-in
		class. */
		PB2Export int			NumParamBlockDescs() { return pbDescs.Count(); }
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns a pointer to the 'i-th' parameter block2 descriptor.
		\param i The zero based index of the parameter block2 descriptor to return. */
		PB2Export ParamBlockDesc2*	GetParamBlockDesc(int i) { return pbDescs[i]; }
		
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns a pointer to the parameter block2 descriptor as specified by
		its <b>BlockID</b>.\n\n
		Note: <b>typedef short BlockID;</b>
		\param id The permanent ID for the parameter block. */
		PB2Export ParamBlockDesc2*	GetParamBlockDescByID(BlockID id);
		/*! \remarks Returns a pointer to the parameter block2 descriptor as
		specified by the descriptor's internal name.
		\param name The internal name of the parameter block descriptor. */
		PB2Export ParamBlockDesc2*	GetParamBlockDescByName(MCHAR* name);
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Adds a parameter block2 to the list of those maintained by this class
		descriptor.
		\param pbd Points to the parameter block2 descriptor of the parameter block2 to add. */
		PB2Export void			AddParamBlockDesc(ParamBlockDesc2* pbd);
		/*! \remarks Implemented by the System.\n\n
		Removes all the parameter block 2 descriptors maintained by this
		plug-in. */
		PB2Export void			ClearParamBlockDescs() { pbDescs.ZeroCount(); }
		// automatic command panel UI management
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		This method is called to handle the beginning of the automatic command
		panel user interface management provided by the param map 2 system.
		This method is called by the plug-in from its
		<b>Animatable::BeginEditParams()</b> method. The parameters passed to
		that method are simply passed along to this method.
		\param ip The interface pointer passed to the plug-in.
		\param obj Points to the plug-in class calling this method.
		\param flags The flags passed along to the plug-in in <b>Animatable::BeginEditParams()</b>.
		\param prev The pointer passed to the plug-in in <b>Animatable::BeginEditParams()</b>. */
		PB2Export void			BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
		
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		This method is called to handle the ending of the automatic command
		panel user interface management provided by the param map 2 system.
		This method is called by the plug-in from its
		<b>Animatable::EndEditParams()</b> method. The parameters passed to
		that method are simply passed along to this method.
		\param ip The interface pointer passed to the plug-in.
		\param obj Points to the plug-in class calling this method.
		\param flags The flags passed along to the plug-in in <b>Animatable::EndEditParams()</b>.
		\param prev The pointer passed to the plug-in in <b>Animatable::EndEditParams()</b>. */
		PB2Export void			EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
		
		/*! \remarks Implemented by the System.\n\n
		This invalidates the entire UI for <b>every</b> parameter map of the
		plug-in. */
		PB2Export void			InvalidateUI();
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		This is called if the user interface parameters needs to be updated.
		This invalidates the entire UI managed by the param map whose
		description is passed.
		\param pbd Points to the parameter block descriptor for the rollup. */
		PB2Export void			InvalidateUI(ParamBlockDesc2* pbd);
		
		/*! \remarks Implemented by the System.\n\n
		This is called if a certain user interface parameter of the specified
		parameter map needs to be updated. The parameter ID of the control is
		passed. If the parameter is a Tab\<\> then the index into the table of
		the parameter is passed.
		\param pbd Points to the parameter block descriptor for the rollup.
		\param id The permanent parameter ID of the parameter.
		\param tabIndex If the parameter is a Tab\<\> then this is the zero based index into
		the table. The default value of -1 indicates it is not a table. */
		PB2Export void			InvalidateUI(ParamBlockDesc2* pbd, ParamID id, int tabIndex=-1); // nominated param
		
		// automatic ParamBlock construction
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		This method is called to create the parameter blocks for the plug-in.
		\param owner Points to the plug-in class calling this method. */
		PB2Export void			MakeAutoParamBlocks(ReferenceMaker* owner);
		
		// access automatically-maintained ParamMaps, by simple index or by associated ParamBlockDesc
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns the number of automatically-maintained parameter map2s. */
		PB2Export int			NumParamMaps() { return paramMaps.Count(); }
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2 system.\n\n
		\param i Points to the parameter block descriptor2 associated with this parameter map. 
		\return A pointer to the parameter map2 as specified by the parameter block2 pointer passed.
		*/
		PB2Export IParamMap2*	GetParamMap(int i) { return paramMaps[i]; }
		
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns a pointer to the parameter map2 as specified by the parameter
		block2 pointer passed.
		\param pbd Points to the parameter block descriptor2 associated with this parameter map.
		\param map_id This parameter specifies the ID of the map/rollout to get. */
		PB2Export IParamMap2*	GetParamMap(ParamBlockDesc2* pbd, MapID map_id = 0);
		
		// maintain user dialog procs on automatically-maintained ParamMaps
		/*! \remarks This overload of <b>SetUserDlgProc()</b> has a new parameter,
		<b>map_id</b>, that specifies the ID of the parameter map/rollup to set
		the user dialog proc for. See original function for the rest of the
		description. */
		PB2Export void			SetUserDlgProc(ParamBlockDesc2* pbd, MapID map_id, ParamMap2UserDlgProc* proc=NULL);
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		This method allows the developer to provide special handling for
		controls not processed automatically by the parameter map (or those
		that need additional processing). The developer provides a dialog proc
		to process the messages from the controls. This method is used to tell
		the parameter map that the developer defined method should be called.
		The given proc will be called after default processing is done.\n\n
		Note, in version 4.0 and later, this actually maps to a call on the
		explicit map ID overload of <b>SetUserDlgProc()</b> with default map ID
		of 0.
		\param pbd Points to the parameter block descriptor for the parameter map.
		\param proc Points to the class derived from <b>ParamMap2UserDlgProc</b> which handles the controls. */
		inline void SetUserDlgProc(ParamBlockDesc2* pbd, ParamMap2UserDlgProc* proc=NULL) { SetUserDlgProc(pbd, 0, proc); }
		
		/*! \remarks Implemented by the System.\n\n
		This is a method of the base class ClassDesc. This class provides an
		implementation of the method used by plug-ins using the ParamBlock2
		system.\n\n
		Returns a pointer to the user dialog proc associated with the parameter
		map as specified by the parameter block descriptor2 pointer.
		\param pbd Points to the parameter block descriptor for the parameter map.
		\param map_id Specifies the ID of the map/rollout to get the user dialog proc for. */
		PB2Export ParamMap2UserDlgProc*	GetUserDlgProc(ParamBlockDesc2* pbd, MapID map_id = 0);
		// 	automatic UI management 
		/*! \remarks Implemented by the System.\n\n
		This method creates and returns a pointer to the object which handles the
		automatic processing of the user interface in the materials editor. This method
		loops over all parameter blocks which specify <b>AUTO_UI</b> and makes the
		AutoMParamDlgs for them. The first one becomes the master and the others are
		added to it.
		\param hwMtlEdit The window handle of the materials editor.
		\param imp The interface pointer provided for calling methods in 3ds Max.
		\param obj Points to the plug-in class calling this method. */
		PB2Export IAutoMParamDlg* CreateParamDlgs(HWND hwMtlEdit, IMtlParams *imp, ReferenceTarget* obj);
		/*! \remarks Implemented by the System.\n\n
		This method creates and returns a pointer to the object which handles
		the automatic processing of the user interface in the materials editor.
		This method makes an AutoMParamDlg for the specified parameter block.
		\param id The permanent ID of the parameter block.
		\param hwMtlEdit The window handle of the materials editor.
		\param imp The interface pointer provided for calling methods in 3ds Max.
		\param obj Points to the plug-in class calling this method.
		\param mapID Specifies the ID of the map/rollout in the parameter block to create AutoMParamDlg for. */
		PB2Export IAutoMParamDlg* CreateParamDlg(BlockID id, HWND hwMtlEdit, IMtlParams *imp, ReferenceTarget* obj, MapID mapID=0);
		/*! \remarks Implemented by the System.\n\n
		This method creates and returns a pointer to the object which handles
		the automatic processing of the user interface in the rendering effects
		dialog. This method loops over all parameter blocks which specify
		<b>AUTO_UI</b> and makes the AutoMParamDlgs for them. The first one
		becomes the master and the others are added to it.
		\param ip The interface pointer provided for calling methods in 3ds Max.
		\param obj Points to the plug-in class calling this method. See Class SpecialFX. */
		PB2Export IAutoEParamDlg* CreateParamDialogs(IRendParams *ip, SpecialFX* obj); // mjm - 07.06.00
		
		/*! \remarks Implemented by the System.\n\n
		This method creates and returns a pointer to the object which handles
		the automatic processing of the user interface in the rendering effects
		dialog. This method makes an AutoEParamDlg for the specified parameter
		block.
		\param id The permanent ID of the parameter block.
		\param ip The interface pointer provided for calling methods in 3ds Max.
		\param obj Points to the plug-in class calling this method. See Class SpecialFX.
		\param mapID Specifies the ID of the map/rollout in the parameter block to create AutoEParamDlg for. */
		PB2Export IAutoEParamDlg* CreateParamDialog(BlockID id, IRendParams *ip, SpecialFX* obj, MapID mapID=0); // mjm - 07.06.00
		
		/*! \remarks Implemented by the System.\n\n
		This method is called when an AutoMParamDlg is deleted.
		\param dlg Pointer to the object which handles the automatic processing of the
		user interface in the materials editor. */
		PB2Export void			MasterDlgDeleted(IAutoMParamDlg* dlg);
		
		/*! \remarks Implemented by the System.\n\n
		This method is called when an AutoEParamDlg is deleted.
		\param dlg Pointer to the object which handles the automatic processing of the
		user interface in the rendering effects dialog. */
		PB2Export void			MasterDlgDeleted(IAutoEParamDlg* dlg);
		
		/*! \remarks Implemented by the System.\n\n
		Returns the master dialog processing routine for the materials editor
		plug-in. */
		PB2Export IAutoMParamDlg* GetMParamDlg() { return masterMDlg; }
		/*! \remarks Implemented by the System.\n\n
		Returns the master dialog processing routine for the rendering effects
		plug-in. */
		PB2Export IAutoEParamDlg* GetEParamDlg() { return masterEDlg; }
		// restore any saved rollout state
		/*! \remarks This method may be called to restore any saved rollout
		state (open/closed condition and scrolling position) for any parameter
		map maintained by the plug-in. */
		PB2Export void			RestoreRolloutState();
		// find last modified param in all blocks, same as on IParamBlock2, but scans all pb's in the object
		/*! \remarks This method scans all the parameter blocks in the owner and
		returns the ParamID and parameter block making the most recent change
		notification.
		\param owner The owner of the parameter blocks.
		\param pb The parameter block which made the most recent notification.
		\return  The parameter ID of the parameter which made the most recent notification. */
		PB2Export ParamID		LastNotifyParamID(ReferenceMaker* owner, IParamBlock2*& pb);
		// reset all params of all known paramblocks to default values, update any UI
		/*! \remarks This method may be called to reset all the parameters of
		all known parameter blocks to their default values and optionally update
		the user interface.
		\param owner The owner of this ClassDesc2.
		\param updateUI  If TRUE to user interface is updated. If FALSE it's not.
		\param callSetHandlers  TRUE to call <b>PBAccessor::Set()</b> for all the parameters; otherwise FALSE. */
		PB2Export void			Reset(ReferenceMaker* owner, BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE);
		
		// get validity of all params in all owner's paramblocks
		/*! \remarks This method updates the validity interval passed with the
		cumulative validity interval of all the owner's parameter blocks.
		\param owner The owner of this ClassDesc2.
		\param t The time about which to compute the interval.
		\param valid The interval to update. */
		PB2Export void			GetValidity(ReferenceMaker* owner, TimeValue t, Interval &valid);
		
		/*! \remarks Implemented by the System.\n\n
		Removes a parameter block descriptor from the list of those maintained by this class 
		descriptor, but does not delete it. This method would typically be used when a 
		ParamBlockDesc2 is dynamically created and its life cycle is controlled by the plugin. 
		You would then be able to remove the parameter block descriptor from the class descriptor
		when no instances of the plug-in require it.
		\param pbd A pointer to the parameter block descriptor to remove. */
		PB2Export void			RemoveParamBlockDesc(ParamBlockDesc2* pbd);

};

// use the constructors to build both static & dynamic descriptors
/*! \sa  Class ClassDesc2, Class ParamMap2UserDlgProc, Class Mtl,  Class Texmap,  Class PBBitmap, Class PBAccessor, Class INode,  Class Color,  Class Point3, Class ReferenceTarget, Structure ParamDef, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_type_2_params.html">List of ParamType Choice</a>,
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_paramtags_choices.html">List of ParamTags
Choices</a>.\n\n
\par Description:
In the Parameter Block2 scheme there is one ParamBlockDesc2 object per entire
Parameter Block2.\n\n
This class is used when adding a paramblock descriptor for each parameter
block2. This is usually done in the form of a static instance of this class.
The constructor takes a number of fixed, block-related arguments and then a
varargs-based variable list of arguments that define the block and its
parameters.\n\n
All methods of this class are implemented by the System.
\par Data Members:
<b>ParamDef* paramdefs;</b>\n\n
Array of parameter definitions.\n\n
<b>ClassDesc2* cd;</b>\n\n
This is the class descriptor of the class which owns this parameter block
descriptor.\n\n
<b>MCHAR* int_name;</b>\n\n
This is the internal name of this parameter descriptor. This name is not
localized. Internal names are meant to be parsable as identifiers. As such they
should begin with an alpha character, have only alphanumerics, and have no
spaces, punctuations, etc. The convention for multi-word names is to use
studly-caps, eg, paintRadius.\n\n
<b>int local_name;</b>\n\n
This is the string table resource ID for the localized (sub-anim) name.\n\n
<b>BlockID ID;</b>\n\n
The permanent parameter block ID.\n\n
<b>USHORT count;</b>\n\n
The number of parameters in block.\n\n
<b>ULONG version;</b>\n\n
This is the parameter block version.\n\n
<b>BYTE flags;</b>\n\n
One or more of the following 11 values (see the constructor argument
<b>flags</b> below for details):    <b><b>P_CLASS_PARAMS</b></b>
<b><b>P_AUTO_CONSTRUCT</b></b>   <b><b>P_AUTO_UI</b></b>
<b><b>P_USE_PARAMS</b></b>   <b><b>P_INCLUDE_PARAMS</b></b>
<b><b>P_SCRIPTED_CLASS</b></b>   <b><b>P_TEMPORARY</b></b>
<b><b>P_HASCATEGORY</b></b>   <b><b>P_CALLSETS_ON_LOAD</b></b>
<b><b>P_TEMPLATE_UI</b></b>   <b><b>P_VERSION</b></b> (New in Max8)  Note: The
following data members are optional and used by the auto-construct code:\n\n
<b>int ref_no;</b>\n\n
The reference number for the auto-constructed parameter block.\n\n
<b>int dlg_template;</b>\n\n
The rollout dialog template resource.\n\n
<b>int title;</b>\n\n
String table resource ID of the rollout title.\n\n
<b>int test_flags;</b>\n\n
The <b>ClassDesc2::Begin/EndEditParams()</b> test flags. See the
<b>flag_mask</b> parameter description below for details.\n\n
<b>int rollup_flags;</b>\n\n
This is used to control rollup creation. See the <b>rollup_flags</b> parameter
description below for details.\n\n
<b>ParamMap2UserDlgProc* dlgProc;</b>\n\n
Points to the parameter map user dialog proc (if used).\n\n
<b>MSPluginClass* pc;</b>\n\n
If this ParamBlockDesc2 belongs to a scripted plug-in this points to the
scripted class (or NULL otherwise). SeeMAXScript SDK.\n\n
<b>Value* rollout;</b>\n\n
If this ParamBlockDesc2 belongs to a scripted plug-in this points to rollout
name. See MAXScript SDK.\n\n
<b>IParamBlock2* class_params;</b>\n\n
Pointer to class parameter block if the <b>CLASS_PARAM</b> flag is specified
for the block. See the flags descriptions below for details. */
class ParamBlockDesc2 : public BaseInterfaceServer
{
private:
	va_list check_param(va_list ap, ParamID id);
	va_list scan_param(va_list ap, ParamID id, ParamDef* p);
	va_list scan_option(va_list ap, int tag, ParamDef* p, MCHAR* parm_name, int& optionnum);
	//! This verifies that all the ParamID's in the descriptor are unique.
	bool check_param_block_desc();

public:
	ParamDef*	paramdefs;		// parameter definitions
	ClassDesc2* cd;				// owning class
	MCHAR*      int_name;      // fixed internal name
	StringResID	local_name;		// localized (subanim) name string resource ID
	BlockID		ID;				// ID
	USHORT		count;			// number of params in block
	ULONG		version;		// paramblock version
	USHORT		flags;			// block type flags
	// auto-construct optional
	int			ref_no;			// reference number for auto-constructed pb
	// auto-ui optional
	int			dlg_template;	// rollout dialog template resource
	int			title;			// rollout title
	int			test_flags;		// BeginEditParams test flags
	int			rollup_flags;	// add rollup page flags
	ParamMap2UserDlgProc* dlgProc;  // IParamMap2 dialog proc
	int			category;		// category for rollup

	// rollout specs if multi-map block (R4 extension) (flags & P_MULTIMAP)
	struct map_spec: public MaxHeapOperators {
		MapID		map_id;			// ID of the map associated with this rollout
		int			dlg_template;	// rollout dialog template resource
		int			title;			// rollout title
		int			test_flags;		// BeginEditParams test flags
		int			rollup_flags;	// add rollup page flags
		ParamMap2UserDlgProc* dlgProc;  // IParamMap2 dialog proc
		int			category;		// category for rollup
	};
	Tab<map_spec> map_specs;	
	// scripted plug-in stuff if this belongs to a scripted plug-in class
	MSPluginClass* pc;			// the scripted class if non-NULL (gc-protected by the scripted plugin class)
	Rollout*	rollout;		// rollout if specified (gc-protected by the scripted plugin class)
	// class param optional
	IParamBlock2* class_params;	// pointer to class paramblock if P_CLASS_PARAMS descriptor

	// constructors
	ParamBlockDesc2();

/*! \remarks This constructor takes a number of fixed, block-related arguments
and then a varargs-based variable list of arguments that define the block and
its parameters.\n\n
The format of the definitions in this shown below, but basically is in the form
of a sequence of fixed specs followed by a variable number of tagged optional
specs for each parameter.
\par Parameters:
The generic form for the parameters of this constructor is shown below: \n
\code
   ParamBlockDesc2 (
   <required_block_specs> ,
   [<version_number> ,]
   [<auto_construct_block_refno> ,]
   [<auto_ui_parammap_specs> ,]
   {<required_param_specs> ,
   {<optional_tagged_param_specs> ,}
end
,}
end);
\endcode
That is: \n
1. Required block specs & per-block flags, followed by, \n
2. Optional version number if P_Version is set, followed by, \n
3. Optional owning object reference number for the block if auto-construct, followed by, \n
4. Optional parameter map specs if auto-ui, followed by, \n
5. Zero or more parameter specs, comprising: \n
  a. Required parameter specs, followed by, \n
  b. Zero or more optional parameter specs, each with is own leading tag, the list terminated by an 'end' tag, followed by, \n
6. an 'end' tag
\par \<required_block_spec>
The following required first arguments to the constructor form the "required_block_spec" and "per block flags": \n\n
<b>BlockID ID</b>  \n
The permanent block ID for the parameter block2. \n\n
<b>MCHAR* int_name</b> \n
The internal name string. This name is not localized. Internal names are meant to be parsable
as identifiers. As such they should begin with an alpha character, have only alphanumerics,
and have no spaces, punctuations, etc. The convention for multi-word names is to use studly-caps, eg, paintRadius. \n\n
<b>int local_name</b>  \n
The resource ID of the localized (sub-anim) name string. \n\n
<b>ClassDesc2* cd</b> \n
Points to the class descriptor2 of the owning class. This is used to add this descriptor to the 
ClassDesc2's table of block descriptors for the class. Note: This value may be passed as
NULL for situations where the blocks owning ClassDesc2 is not available for static initializations 
(such as in a separate file). Before using the descriptor for any block construction, the 
ClassDesc2* must be initialized with the method: \n
<code>void ParamBlockDesc2::SetClassDesc(ClassDesc2* cd);</code> \n
You can only call this method once on a descriptor and then only if it has been constructed initially with a NULL <b>cd</b>. \n\n
<b>BYTE flags</b> \n
Per block/descriptor flags. One or more of the following values (they may be added together as in P_AUTO_CONSTRUCT + P_AUTO_UI). \n
- P_VERSION \n
New in max 8. When used as a ParamBlockDesc2 flag, a version number is expected immediately after the
flags field, before the reference number field. This version number will be stored as
ParamBlockDesc2::version and returned as IParamBlock2::GetVersion(). \n\n
- P_CLASS_PARAMS \n
Indicates this block holds class-level parameters which are attached to the ClassDesc2 for the plug-in.
Such class level parameters are shared by each instance of the class. The block is automatically allocated by
and stored in the descriptor. You get at its parameters via GetValue()/SetValue() calls on the descriptor. \n\n
- P_AUTO_CONSTRUCT \n
Indicates the parameter block2 will be constructed and referenced automatically to its owner in the call to
ClassDesc2::MakeAutoParamBlocks(). If this flag is set, the parameter block's reference number in the owning
object should be given immediately following the flag word in the descriptor constructor. See \<auto_construct_block_refno> . \n\n
- P_AUTO_UI \n
Indicates this block supports automatic UI rollout management in calls to ClassDesc2::BeginEditParams(), 
ClassDesc2::EndEditParams(),ClassDesc2::CreateParamDlg(), ClassDesc2::CreateParamDialog(), etc. \n
If set, the \<auto_ui_parammap_specs> must be supplied in the descriptor constructor. \n\n
- P_HASCATEGORY \n
The category field that can be used to order rollups (see Class IRollupWindow) to various Parameter Map creation 
methods. In order to use the category field with AutoUI, this flag has to be declared together with P_AUTO_UI 
in the ParamBlockDesc2. An additional int, that describes the category has to be appended to the parameter 
list after the ParamMap2UserDlgProc* proc parameter. The same thing is true for multimaps. The P_HASCATEGORY 
field can only be declared for the whole pblock. That means, that every multimap has to have the category 
parameter. To use the standard value ROLLUP_CAT_STANDARD can be used. In the example below 4900 is 
used as the integer describing the category; \n
\code
static ParamBlockDesc2 std2_shader_blk ( std2_shader,
_M("shaderParameters"), 0, &stdmtl2CD,
P_AUTO_CONSTRUCT + P_AUTO_UI + P_HASCATEGORY, SHADER_PB_REF,
//rollout
IDD_DMTL_SHADER4, IDS_KE_SHADER, 0, 0, &shaderDlgProc, 4900,
// params
std2_shader_type, _M("shaderType"), TYPE_INT, 0, IDS_JW_SHADERTYPE,
\endcode
- P_USE_PARAMS \n
Indicates that this block shares (exactly) the paramdefs from another descriptor. This is used 
to specify an already established ParamBlockDesc2 instance whose parameters you wish to share. 
This effectively gives the referring descriptor a pointer to the established descriptors 
'paramdefs' array. In this case no other parameters definition can be supplied to referencing 
descriptors constructor. See the sample code below. \n\n
- P_INCLUDE_PARAMS \n
Indicates that this block loads in a copy the paramdefs from another descriptor. This is 
used to take a copy of an already established descriptor's parameters, to which you can add 
extra parameter definitions in the referencing descriptors constructor. This provides a 
kind of poor-man's factoring of common parameters, but note it is a copy; any subsequent
changes to the establised descriptor's parameter definitions are not reflected in the 
referencing descriptor. \n\n
You give the pointer to the ParamBlockDesc2 supplying the existing parameter definitions
following the other optional block-level parameters: following the flag word is the 
block's reference number if P_AUTO_CONSTRUCT is specified, then the rollout dialog 
template info if P_AUTO_UI is specified, then the pointer to the sourcing descriptor 
if P_USE_PARAMS or P_INCLUDE_PARAMS is specified. For example: \n
\code
static ParamBlockDesc2 metal2_param_blk ( shdr_params, _M("shaderParameters"), 0, &metalCD, P_AUTO_CONSTRUCT + P_USE_PARAMS,
// pblock refno 0,
// use params from existing descriptor
&const_param_blk
);
\endcode 
- P_SCRIPTED_CLASS \n
This is for internal use only. It means that the descriptor was defined on the 
fly as a side-effect of writing a scripted plug-in. \n\n
- P_TEMPORARY \n
This is for internal use only. \n\n
- P_TEMPLATE_UI \n
This indicates that dialog templates will be provided or constructed. \n\n
- P_CALLSETS_ON_LOAD \n
Signals that this block should have CallSets() called on it during post-load processing 
on scene loads & merges. This effectively ensures that all PBAccessor::Set() methods 
will be called after the flagged pblock2 is fully loaded, so that they can track loaded 
param values, for example. This allows a single point of param value tracking in the 
PBAccessor::Set() and precludes the need for individual objects to implement PLCBs to 
do this tracking themselves. \n\n
- P_MULTIMAP \n
Indicates that the block being described will have more than one rollup/map. If you
specify this flag, the constructor interprets the rollup template and parameter definitions 
arguments in a modified syntax. Here's a sample rework of the main pblock in GeoSphere into two rollups: \n
\code
enum { geo_map_1, geo_map_2 }; // enum IDs for the 2 parammaps

static ParamBlockDesc2 geo_param_blk ( geo_params, _M("GeosphereParameters"),
                   0, &gsphereDesc, P_AUTO_CONSTRUCT +
                   P_AUTO_UI + P_MULTIMAP, PBLOCK_REF_NO,
// map rollups
2,
geo_map_1, IDD_GSPHERE_1, IDS_PARAMETERS_1, 0, 0, NULL,
geo_map_2, IDD_GSPHERE_2, IDS_PARAMETERS_2, 0, 0, NULL,

// params
geo_hemi, _M("hemisphere"), TYPE_BOOL, P_ANIMATABLE, IDS_HEMI,
   p_default, FALSE,
   p_ui, geo_map_2, TYPE_SINGLECHEKBOX, IDC_HEMI,
   end,
geo_segs, _M("segs"), TYPE_INT, P_ANIMATABLE, IDS_RB_SEGS,
   p_default, 4,
   p_range, MIN_SEGMENTS, MAX_SEGMENTS,
   p_ui, geo_map_1, TYPE_SPINNER, EDITTYPE_INT,
   IDC_SEGMENTS, IDC_SEGSPINNER, 0.05f,
   end,
geo_radius, _M("radius"), TYPE_FLOAT, P_ANIMATABLE + P_RESET_DEFAULT, IDS_RB_RADIUS,
   p_default, 0.0,
   p_ms_default, 25.0,
   p_range, MIN_RADIUS, MAX_RADIUS,
   p_ui, geo_map_1, TYPE_SPINNER, EDITTYPE_UNIVERSE,
   IDC_RADIUS, IDC_RADSPINNER, 1.0,
   p_uix, geo_map_2,
   end,
//...
\endcode \n
First, there is an enum to provide IDs for the two maps in the main block, geo_map_1 
and geo_map_2. The P_MULTIMAP flag is added to the block flags in the main descriptor 
constructor arguments to indicate multiple pmaps present. If P_AUTO_UI is specified, 
the usual single rollup template spec is replaced by a count (of rollups) followed 
by that many sets of rolup specs, each beginning with the associated mapID. The auto 
UI mechanism will add the rollups in the order given in this list. \n\n
In the parameter definition section, the only change is to the p_ui option, which
now requires a map ID before the rest of the UI specification to say which rollup/map
the spec relates to. In this case, we've put the hemisphere checkbox in the 2nd rollup
and the segs and radius spinners in the first. \n\n
There is also a new option, p_uix, which is used to say that the parameter is to appear
in more than one rollup. In this case, the radius spinner also shows up in the 2nd rollup,
geo_map_2. When you do this, all the controls that connect to this parameter are ganged
together; they all change when any one of them changes and all show keyframe highlights
and so on. The current limitations on this are that the type of UI control and its various
dialog template item IDs must be the same in each rollup in which it appears. \n\n
\par [\<version_number>]
If P_VERSION is used as a ParamBlockDesc2 flag, a version number is expected immediately 
after the flags field, before the reference number field. This version number will be 
stored as ParamBlockDesc2::version and returned as IParamBlock2::GetVersion() \n\n
\par [\<auto_construct_block_refno>]
If P_AUTO_CONSTRUCT is specified in the required per block / descriptor flags (BYTE flags above) 
then the integer reference number of the parameter block2 in the plug-in needs to be specified: \n
- int ref_no \n
This is the same number that the plug-in would use to get and set the parameter block referrence 
in GetReference() and SetReference(). \n\n
\par [\<auto_ui_parammap_specs>]
If P_AUTO_UI is specified in the required per block / descriptor flags (BYTE flags above) then 
the following arguments to the constructor are required: \n
\code
int dialog_template_ID,
int dialog_title_res_ID,
int flag_mask,
int rollup_flags,
ParamMap2UserDlgProc* proc
\endcode
Each of these is described below: \n
- int dialog_template_ID \n
The ID of the dialog template (eg IDD_something). \n\n
- int dialog_title_res_ID \n
The string table resource ID for the title of the dialog. \n\n
- int flag_mask \n
This is used by ClassDesc2::BeginEditParams() and ClassDesc2::EndEditParams() to determine 
whether the ParamMap2 shold be created/deleted on this call. All the bits in the supplied 
mask must be on in the Begin/EndEditParams flag longword for the action to take place. \n\n
- int rollup_flags \n
This flag is used to control rollup creation. You may pass: \n
<b>APPENDROLL_CLOSED</b> \n
to have the rollup added in the closed (rolled up) state. Otherwise pass 0. \n\n
- ParamMap2UserDlgProc* proc \n
If there are controls in the dialog that require special processing this user dialog proc can 
be implemented to process them. See Class ParamMap2UserDlgProc. If not used then NULL should be passed. \n\n
\par \<required_param_specs>
The required parameter spec is formatted as shown below. There is one of these for each of the controls 
managed by the parameter map. They are followed by an \<optional_tagged_param_spec>. \n
\code
ParamID id,
MCHAR* internal_name,
ParamType type,
[int table_size,]
int flags,
int local_name_res_ID,
\endcode
Each of these is described below:\n
- ParamID id \n
The permanent, position-independent ID for the parameter. \n\n
- MCHAR* internal_name \n
The internal name for the parameter. \n\n
- ParamType type \n
The type of parameter. See List of ParamType Choices. \n\n
- [int table_size] \n
If the type is one of the Tab\<> types, you need to supply an initial table size which can be 0. \n\n
- int flags \n
The per parameter flag bits. This should be a bitwise OR of one or more of the following: \n\n
<b>P_ANIMATABLE</b> \n
Indicates the parameter is animatable. \n\n
<b>P_TRANSIENT</b> \n
Indicates the parameter should not be saved in the scene file. One might do this, for example, 
for virtual parameters that don't actually hold data but reflect data held elsewhere in the
object (and accessed via a PBAccessor) which is saved explicitly by it. This might save 
file space. In some cases, parameters might be provided that are derived from other state 
in the object that is computed each time you load the object and made available say as 
a help to script authors. In these cases also, you might decide not to take up file space. \n\n
<b>P_NO_INIT</b> \n
This is obsolete. \n\n
<b>P_COMPUTED_NAME</b> \n
Indicates to call a compute name function to get a dynamically-computed name. This allows a 
plug-in to provide a dynamically-created local name for a parameter or Tab\<> parameter 
entry. If you specify this parameter flag, you also need to suppy a p_accessor 
PBAccessor instance pointer that has the GetLocalName() method implemented. \n\n
<b>P_INVISIBLE</b> \n
Not visible in track view (if an animatable). \n\n
<b>P_RESET_DEFAULT</b> \n
Indicates to not make creation parameters sticky, rather always reset to default values. \n\n
<b>P_SUBANIM</b> \n
Indicates this is a non-animatable reference target parameter to be published as a 
sub-anim (which makes it visible in Track View) \n\n
<b>P_TV_SHOW_ALL</b> \n
This is used for Tab\<> animatables, and indicates to show all entries in Track View
even if no controller assigned. \n\n
<b>P_NO_REF</b> \n
For reference target parameters, this indicates to not maintain the reference automatically 
(rather simply keep a copy of the pointer). \n\n
<b>P_OWNERS_REF</b> \n
Indicates this is a reference target parameter owned by the parameter block's owner not 
the block itself. Make sure to supply the owner's reference number in a p_refno specification. 
If neither p_refno or P_OWNERS_REF is set, the parameter block owns and maintains the reference. \n\n
<b>P_CAN_CONVERT</b> \n
Indicates the p_classID validator is used in a CanConvertoTo() call, rather than as exact classID match. \n\n
<b>P_SUBTEX</b> \n
Indicates a texmap parameter is kept by the owner using the MtlBase::xSubTexmap methods. 
Provide the integer index of the sub-texmap using the param tag p_subtexno. \n\n
You would use this flag in materials or texmaps that contain other texmaps as parameters, 
in which these 'sub' maps are not stored in the ParamBlock2 and are not accessible as direct 
references on the parent map or material, but are accessible via the 
MtlBase::GetSubTexmap()/SetSubTexmap() protocol. \n\n
Specifying this P_SUBTEX flag signals this situation and looks for a p_subtexno parameter 
option to give the sub-texmap number for the map. For example, the Standard material 
stores all its maps in a separate structure that appears as a single reference in 
the Standard material. In this case, the individual texmaps are not direct references 
on the material, but are accessible on the material via GetSubTexmap()/SetSubTexmap(). \n\n
Note that P_OWNERS_REF and P_SUBTEX are both primarly intended for use when re-coding 
existing plug-ins in which the sub-maps are already managed by the owner in some way. 
If you are implementing a new plug-in, you should just let the ParamBlock2 host them 
for you and then you don't have to bother with either of these flags. \n\n
<b>P_VARIABLE_SIZE</b> \n
Indicates a Tab\<> parameter is variable in size. This allows scripted changes. \n\n
<b>P_NO_AUTO_LABELS</b> \n
Disables the automatic setting of button text for texmaps, materials, files, etc. 
You can use the method IParamMap2::SetText() to set it by hand. \n\n
<b>P_SHORT_LABELS</b> \n
This is for use with TYPE_TEXMAP, TYPE_MTL and TYPE_BITMAP parameters that are associated 
with ParamMap2 picker buttons. If you specify this flag, a shortened form of the object 
name is installed in the picker button label. For texmaps and materials, the 
MtlBase::GetName() rather than MtlBase::GetFullName() is used, and for bitmaps just 
the filename rather than the full pathname is used. \n\n
<b>P_READ_ONLY</b> \n
This parameter is not assignable through MAXScript. It allows try-and-buy 3rd-party plugins. \n\n
<b>P_OBSOLETE</b> \n
New in Max 8.0. When used as a ParamBlockDesc2 param flag, the param will not be saved to disk 
or maintain references or animation keys. When loading older files, the parameter will be reset 
after the load is completed, but will be valid during a post-load callback \n\n
- int local_name_res_ID \n
The localized name for the parameter. This is a ID for the resource in the string table. \n\n
\par \<optional_tagged_param_specs>
There may be zero or more optional tagged parameter specs, each with its own leading tag, with
the entire list terminated by an 'end' tag. This list of tagged arguments has the following form: \n
<b>\<tag>, \<optional_param_spec>, </b> \n
For the possible tags and the specification arguments see <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_paramtags_choices.html">List of ParamTags Choices</a>.
*/
	PB2Export   ParamBlockDesc2(BlockID ID, MCHAR* int_name, StringResID local_name, ClassDesc2* cd, USHORT flags, ...);
	PB2Export  ~ParamBlockDesc2();

	// building descriptors incrementally
	/*! \remarks This method is used for building descriptors incrementally.
	It adds a parameter to an existing descriptor.\n\n
	Note that you must not modify a descriptor with this function once it has
	been used to construct a ParamBlock2 (for instance in object creation);
	there is no version control in place and crashes or unpredictable results
	can occur.
	\par Parameters:
	<b>ParamID id, ...</b>\n\n
	This function takes a single parameter definition in exactly the same
	varargs format as the ParamBlockDesc2 constructor. See the Constructors section.
	*/
	PB2Export void AddParam(ParamID id, ...);
	/*! \remarks This method is used for modifying a descriptors
	incrementally. It overrides an existing parameter definition of same ID
	passed. Note: You must not modify a descriptor with this method once it has
	been used to construct a ParamBlock2.
	\par Parameters:
	<b>ParamID id, ...</b>\n\n
	This function takes a single parameter definition in exactly the same
	varargs format as the ParamBlockDesc2 constructor. See the Constructors section.
	*/
	PB2Export void ReplaceParam(ParamID id, ...);
	/*! \remarks Deletes the specified parameter from the descriptor.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter to delete. */
	PB2Export void DeleteParam(ParamID id);
	/*! \remarks This method is used for modifying a descriptor incrementally.
	It alters a parameter definition optional information tag of an existing
	descriptor. Note: You must not modify a descriptor with this method once it
	has been used to construct a ParamBlock2.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int option_tag, ...</b>\n\n
	This method takes a single tagged option in the same varargs format as the
	tagged parameter options in the ParamBlockDesc2 constructor. See the 
	<b>\<optional_tagged_param_specs></b> section in the constructor's documentation.
	*/
	PB2Export void ParamOption(ParamID id, int option_tag, ...);

	/*! \brief Modifies the list of controls enabled by a certain control
	This method allows to modify the list of controls that are enabled/disabled by a given control
	that represents a boolean parameter. Any existent list of dependent controls are replaced.
	\par pID The identifier of the control to be modified. The id must designate a control in this parameter block descriptor.
	\par dependentPIDs Identifiers that depend on the control specified by pID. 
	The parameters must be part of the same parameter block descriptor as the parameter that controls them (pID).
	\return True if the control was modified successfully, false otherwise
	\note A parameter descriptor should not be modified after a parameter block has been created based on it.
	*/
	PB2Export bool ParamOptionEnableCtrls(ParamID pID, const Tab<ParamID>& dependentPIDs);

	/*! \brief This method is used for modifying a descriptor incrementally.
	It allows for specifying the content of certain controls such as list and combo boxes 
	by supplying a list of string resource ids.
	This method alters a parameter definition optional information tag of an existing
	descriptor. Note: You must not modify a descriptor with this method once it
	has been used to construct a ParamBlock2.
	\param id - The permanent ID of the parameter to be modified
	\param stringItems - Resource ids of the strings that represent the user visible values for this parameters
	\param stringItemValues - Optional list of integer values to be associated with the 
	corresponding item in the stringItems parameter. If this parameter is not specified, 
	the items in stringItems will be associated with their zero based index.
	*/
	PB2Export bool ParamOptionContentValues (ParamID id, Tab<int>& stringItems, Tab<int>* stringItemValues = NULL);

	// for delayed setting of ClassDesc2
	/*! \remarks This method sets the ClassDesc2 pointer maintained by this
	class. You can only call this method once on a descriptor and then only if
	it has been constructed initially with a NULL <b>cd</b>. See the notes in
	the constructor.
	\par Parameters:
	<b>ClassDesc2* cd</b>\n\n
	Points to the ClassDesc to set. */
	PB2Export void SetClassDesc(ClassDesc2* cd);

	// param metrics
	/*! \remarks Returns the number of parameters in the block. */
	USHORT		Count() { return count; }
	/*! \remarks Returns the version of the parameter block. */
	DWORD		Version() { return version; }
	/*! \remarks Returns the index into the parameter definition array of the
	parameter whose ID passed.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent parameter ID. */
	PB2Export int IDtoIndex(ParamID id);
	PB2Export int NameToIndex(MCHAR* name);
	/*! \remarks Returns the permanent parameter ID of the parameter whose
	index is passed.
	\par Parameters:
	<b>int i</b>\n\n
	The zero based index of the parameter in the <b>paramdefs</b> array. */
	ParamID		IndextoID(int i) { return (((i >= 0) && (i < Count())) ? paramdefs[i].ID : -1); }
	/*! \remarks This method is used for accessing a parameter's ParamDef
	structure.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter. */
	ParamDef&	GetParamDef(ParamID id) { int i = IDtoIndex(id); DbgAssert(i >= 0); return paramdefs[i]; }

	// parameter accessors for static class param blocks, these bounce off to the class paramblock
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the floating point value of the specified parameter at the specified
	time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>float v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, float v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the integer value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>int v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, int v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the Point3 value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>Point3\& v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, Point3& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	BOOL SetValue(ParamID id, TimeValue t, Point4& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the Color value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>Color\& v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, Color& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }  // uses Point3 controller
	BOOL SetValue(ParamID id, TimeValue t, AColor& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }  // uses Point4 controller
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the string value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>MCHAR* v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, const MCHAR* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the Mtl* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>Mtl*v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, Mtl*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the Texmap* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>Texmap* v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, Texmap* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the PBBitmap* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>PBBitmap* v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, PBBitmap* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the INode* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>INode* v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, INode* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Sets the ReferenceTarget* value of the specified parameter at the specified
	time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>ReferenceTarget*v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, ReferenceTarget*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, IParamBlock2*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	/*! \remarks	This method is used with static class parameter blocks only.\n\n
	Sets the Matrix3 value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>Matrix3\& v</b>\n\n
	The value to set.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to set.
	\return  TRUE on success; otherwise FALSE. */
	BOOL SetValue(ParamID id, TimeValue t, Matrix3&	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }

	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Retrieves the floating point value of the specified parameter at the
	specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>float\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the
	retrieved parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Retrieves the integer value of the specified parameter at the specified
	time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>int\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the
	retrieved parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Retrieves the Point3 value of the specified parameter at the specified
	time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>Point3\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the
	retrieved parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, Point3& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Point4& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Retrieves the Color value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>Color\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the
	retrieved parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, Color& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, AColor& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the string value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>MCHAR*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, const MCHAR*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the Mtl* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>Mtl*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, Mtl*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the Texmap* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>Texmap*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, Texmap*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the PBBitmap* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>PBBitmap*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, PBBitmap*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the INode* value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>INode*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, INode*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks This method is used with static class parameter blocks only.\n\n
	Retrieves the ReferenceTarget* value of the specified parameter at the
	specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>ReferenceTarget*\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the retrieved
	parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table of
	the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, ReferenceTarget*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, IParamBlock2*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	/*! \remarks	This method is used with static class parameter blocks only.\n\n
	Retrieves the Matrix3 value of the specified parameter at the specified
	time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to get the value.\n\n
	<b>Matrix3\& v</b>\n\n
	The value to retrieve is returned here.\n\n
	<b>Interval \&ivalid</b>\n\n
	This is the validity interval which is updated by the validity of the
	retrieved parameter.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  TRUE on success; otherwise FALSE. */
	BOOL GetValue(ParamID id, TimeValue t, Matrix3& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }

	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the Color value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The Color value of the parameter. */
	PB2Export Color			GetColor(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export AColor		GetAColor(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Retrieves the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The Point3 value of the parameter */
	PB2Export Point3		GetPoint3(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Point4		GetPoint4(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The integer value of the parameter. */
	PB2Export int			GetInt(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The floating point value of the parameter. */
	PB2Export float			GetFloat(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The TimeValue value of the parameter. */
	PB2Export TimeValue		GetTimeValue(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The TimeValue value of the parameter. */
	PB2Export const MCHAR*		GetStr(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  A pointer to the Mtl object. */
	PB2Export Mtl*			GetMtl(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  A pointer to the Texmap. */
	PB2Export Texmap*		GetTexmap(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  A pointer to the PBBitmap object. */
	PB2Export PBBitmap*		GetBitmap(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  A pointer to the INode. */
	PB2Export INode*		GetINode(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks This method is used with static class parameter blocks
	only.\n\n
	Returns the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  A pointer to the ReferenceTarget. */
	PB2Export ReferenceTarget* GetReferenceTarget(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export IParamBlock2* GetParamBlock2(ParamID id, TimeValue t=0, int tabIndex=0);
	/*! \remarks	This method is used with static class parameter blocks only.\n\n
	Retrieves the value of the specified parameter at the specified time.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to get the value.\n\n
	<b>int tabIndex=0</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index into the table
	of the value to get.
	\return  The Matrix3 value of the parameter */
	PB2Export Matrix3		GetMatrix3(ParamID id, TimeValue t=0, int tabIndex=0);

	// get a string resource from plug-in module's resource
	/*! \remarks Returns a string resource from plug-in module's resource.
	\par Parameters:
	<b>StringResID id</b>\n\n
	The permanent ID of the parameter. */
	PB2Export MCHAR* GetString(StringResID id) { return cd->GetRsrcString(id); }
	// invalidate any current UI (parammap2) currently open for this descriptor
	/*! \remarks This method invalidates any current parameter map2 user
	interface currently open for this descriptor. */
	PB2Export void InvalidateUI() { cd->InvalidateUI(this); }
	/*! \remarks This method invalidates the control whose parameter ID is
	specified.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID of the parameter.\n\n
	<b>int tabIndex=-1</b>\n\n
	If the parameter is a Tab\<\> this is the zero based index of parameter
	whose associated control is to be redrawn. */
	PB2Export void InvalidateUI(ParamID id, int tabIndex=-1) { cd->InvalidateUI(this, id, tabIndex); } // nominated param
	// get/set user dialog proc for the param map currently open this descriptor
	/*! \remarks	This overload of <b>SetUserDlgProc()</b> has a new parameter,
	<b>map_id</b>, that specifies the ID of the parameter map/rollup to set the
	user dialog proc for. See original function for the rest of the
	description. */
	PB2Export void SetUserDlgProc(MapID map_id, ParamMap2UserDlgProc* proc=NULL);
	/*! \remarks This method allows for special handling for a set of
	controls. The developer provides a dialog proc object to process the
	message from the controls. This method is used to tell the parameter map
	that the developer defined method should be called. The given proc will be
	called after default processing is done. Note that if the proc is non-NULL
	when the ParamMap is deleted its DeleteThis() method will be called.\n\n
	Note, in version 4.0 and later, this actually maps to a call on the
	explicit map ID overload of <b>SetUserDlgProc()</b> with default map ID of
	0.
	\par Parameters:
	<b>ParamMap2UserDlgProc* proc=NULL</b>\n\n
	A pointer to the user dialog proc object to process the control. */
	inline void		SetUserDlgProc(ParamMap2UserDlgProc* proc=NULL) { SetUserDlgProc(0, proc); }
	/*! \remarks Returns the user dialog proc for the parameter map associated
	with this descriptor.
	\par Parameters:
	<b>MapID map_id</b>\n\n
	Specifies the ID of the map/rollout to get the user dialog proc for. */
	PB2Export ParamMap2UserDlgProc* GetUserDlgProc(MapID id = 0);
	// dynamically access the P_OWNERS_REF refno for given RefTarg parameter 
	/*! \remarks This method allows dynamic setting of the <b>P_OWNERS_REF</b>
	reference number for given Reference Target parameter.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID for the parameter.\n\n
	<b>int refno</b>\n\n
	The reference number to set. */
	PB2Export void SetOwnerRefNo(ParamID id, int refno);
	/*! \remarks This method returns the <b>P_OWNERS_REF</b> reference number
	for given Reference Target parameter.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID for the parameter. */
	PB2Export int  GetOwnerRefNo(ParamID id);
	// dynamically access the p_subtexno/p_submtlno number for given map/mtl parameter 
	/*! \remarks Sets the sub-texture number for the specified texmap
	parameter. You can use this to dynamically change a parameter's sub object
	number.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The parameter ID for the texmap.\n\n
	<b>int texno</b>\n\n
	The sub-texture number to set. */
	PB2Export void SetSubTexNo(ParamID id, int texno);
	/*! \remarks Sets the sub-material number for the specified texmap
	parameter. You can use this to dynamically change a parameter's sub object
	number.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The parameter ID for the material.\n\n
	<b>int mtlno</b>\n\n
	The sub-material number to set. */
	PB2Export void SetSubMtlNo(ParamID id, int mtlno);
	/*! \remarks Returns the sub-texture number for the specified parameter.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The ID of the parameter. */
	PB2Export int  GetSubTexNo(ParamID id);
	/*! \remarks Returns the sub-material number for the specified parameter.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The ID of the parameter. */
	PB2Export int  GetSubMtlNo(ParamID id);
	// dynamically access the TYPE_OPEN/SAVEFILEBUTTON p_init_file field 
	/*! \remarks This method allows dynamic setting of the
	<b>TYPE_OPEN/SAVEFILEBUTTON p_init_file</b> field.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID for the parameter.\n\n
	<b>MCHAR* s</b>\n\n
	The string to set. */
	PB2Export void SetInitFile(ParamID id, MCHAR* s);
	/*! \remarks This method returns the <b>TYPE_OPEN/SAVEFILEBUTTON
	p_init_file</b> field.
	\par Parameters:
	<b>ParamID id</b>\n\n
	The permanent ID for the parameter. */
	PB2Export MCHAR* GetInitFile(ParamID id);

	// Interface support
	//! \brief Adds an interface to the descriptor's BaseInterfaceServer interfaces list.
	/*! The interface can later be obtained from the list using GetInterface()
	    \param[in] iface The interface to add */
	PB2Export void AddInterface(BaseInterface* iface);

	//! \brief Removes an interface from the descriptor's BaseInterfaceServer interfaces list.
	/*! \param[in] iface The interface to remove */
	PB2Export void RemoveInterface(BaseInterface* iface);

	//!\brief Allows a custom filter list to be supplied to the File dialog routines
	/*!
	\param id The parameter that defines a file dialog
	\param fileType A string containing a correctly formated filter list.  This buffer is maintained by the calling code
	*/
	PB2Export void SetFileTypes(ParamID id, MCHAR * fileType);
};

/*! \remarks This method is used to create a parameter block2.
\par Parameters:
<b>ParamBlockDesc2 *pdesc</b>\n\n
This is an array of parameter block descriptors.\n\n
<b>ReferenceMaker* iowner</b>\n\n
Points to the owner of the parameter block.
\return  A pointer to the created parameter block. On error NULL is returned.
*/
PB2Export IParamBlock2* CreateParameterBlock2(ParamBlockDesc2* pdesc, ReferenceMaker* iowner);
class MacroRecorder;
/*! \remarks This function for internal use only. */
PB2Export void SetPB2MacroRecorderInterface(MacroRecorder* mri);
						
/*! \remarks This function updates or creates a new ParamBlock2, based on an existing
ParamBlock of an earlier version. The new parameter block inherits any
parameters from the old parameter block whose parameter IDs match. This may
also be used to partially update an existing ParamBlock2.
The parameter type of the ParamBlock and ParamBlock2 entries must match, with a 
special case that if the ParamBlock parameter type is TYPE_RGBA, the ParamBlock2 
parameter type can be either TYPE_RGBA or TYPE_FRGBA
\par Parameters:
<b>ParamBlockDescID *pdescOld</b>\n\n
The array of parameter block descriptors which describes each parameter in the
old parameter block.\n\n
<b>int oldCount</b>\n\n
The number of elements in the array above.\n\n
<b>IParamBlock *oldPB</b>\n\n
The old parameter block.\n\n
<b>ParamBlockDesc2* pdescNew</b>\n\n
Points to the new parameter block 2 descriptor.\n\n
<b>IParamBlock2* newPB=NULL</b>\n\n
Points to an existing IParamBlock2 indicating that this paramblock should be
filled in from the old ParamBlock, rather than creating a new one.
\return  The new parameter block2. */
PB2Export IParamBlock2* UpdateParameterBlock2(
	ParamBlockDescID *pdescOld, int oldCount, IParamBlock *oldPB,
	ParamBlockDesc2* pdescNew,
	IParamBlock2* newPB=NULL);

/*! \remarks This function updates a ParamBlock based on an existing ParamBlock2. 
The ParamBlock inherits any parameters from the ParamBlock2 whose parameter IDs match. 
The parameter type of the ParamBlock and ParamBlock2 entries must match, with a 
special case that if the ParamBlock parameter type is TYPE_RGBA, the ParamBlock2 
parameter type can be either TYPE_RGBA or TYPE_FRGBA
This is mainly intended for use in Save To Previous, where the current version is 
ParamBlock2 based, and the old version is ParamBlock based.
Note that if a ParamBlock2 parameter is animated, the ParamBlock2 will hold a reference
to a controller for that parameter. This function does not clone the controller and 
have the ParamBlock hold a reference to that clone, rather the ParamBlock will hold a
reference to the original controller. This is normally ok, since the ParamBlock will be 
deleted at the end of the save process. If the ParamBlock is being created other than
for Save To Previous, you should consider replacing any references held by the ParamBlock
with a clone of that reference.
\par Parameters:
<b>IParamBlock2* pb2</b>\n\n
Points to source IParamBlock2.
<b>IParamBlock* pb1</b>\n\n
Points to destination IParamBlock.
<b>ParamBlockDescID *pdescPB1</b>\n\n
The array of parameter block descriptors which describes each parameter in the
ParamBlock.\n\n
<b>int pb1Count</b>\n\n
The number of elements in the array of parameter block descriptors.\n\n 
\return  The number of parameters copied from the IParamBlock2 to the IParamBlock. 

\code
	bool Swirl::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
	{
		// if saving to previous version that used pb1 instead of pb2...
		DWORD saveVersion = GetSavingVersion();
		if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R13)
		{
			// create the pb1 instance
			IParamBlock* paramBlock1 = CreateParameterBlock( pbdesc,swirl_num_params_ver1,1);
			DbgAssert(paramBlock1 != NULL);
			if (paramBlock1)
			{
				// copy data from the pb2 to the pb1
				int numParamsCopied = CopyParamBlock2ToParamBlock(pblock,paramBlock1,pbdesc,swirl_num_params_ver1);
				DbgAssert(numParamsCopied == swirl_num_params_ver1);
				// register the reference slot replacement
				referenceSaveManager.ReplaceReferenceSlot(PBLOCK_REF,paramBlock1);
			}
		}
		return Texmap::SpecifySaveReferences(referenceSaveManager);
	}
\endcode
*/
PB2Export int CopyParamBlock2ToParamBlock(
	IParamBlock2* pb2,
	IParamBlock *pb1, ParamBlockDescID *pdescPB1, int pb1Count);

// This post-load callback handles conversion of pre-ParamBlock2 versions
// of an object to a ParamBlock2 version.
// NOTE: this thing deletes itself when it's done.
/*! \sa  Class PostLoadCallback, Class ILoad.\n\n
\par Description:
This is a post load call back for fixing up parameter block2s. This callback
handles conversion of pre-ParamBlock2 versions of an object to a ParamBlock2
version. NOTE: this thing deletes itself when it's done.
\par Data Members:
All data members are public.\n\n
<b>ParamVersionDesc* versions;</b>\n\n
This is an array of ParamVersionDesc2s.\n\n
<b>int count;</b>\n\n
This is the number in the array specified above.\n\n
<b>ParamBlockDesc2* curdesc;</b>\n\n
This is a pointer to the current version of the description.\n\n
<b>ReferenceTarget* targ;</b>\n\n
This is a pointer to a reference target. This is usually the this pointer of
the object.\n\n
<b>int pbRefNum;</b>\n\n
This is the reference index of the parameter block.  */
class ParamBlock2PLCB : public PostLoadCallback 
{
public:
	ParamVersionDesc* versions;
	int				  count;
	ParamBlockDesc2*  curdesc;
	ReferenceTarget*  targ;
	int				  pbRefNum;
	
	/*! \remarks Constructor. The data members are initialized to the values
	passed. */
	ParamBlock2PLCB(ParamVersionDesc *v, int cnt, ParamBlockDesc2* pd, ReferenceTarget *t, int refNum)
	{ 
		versions = v; count = cnt; curdesc = pd; targ = t; pbRefNum = refNum;
	}

	/*! \remarks This methods handles the conversion of edition 1 ParamBlocks
	to the new ParamBlock2s.
	\par Parameters:
	<b>ILoad *iload</b>\n\n
	An interface for loading files. */
	PB2Export void proc(ILoad *iload);
	int Priority() { return 0; }
	PB2Export INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);
};


// This post-load info about a param block as it was loaded from disk.
// The info can be accessed from an IParamBlock2 or ParamBlockDesc2 using GetInterface
#define IPARAMBLOCK2POSTLOADINFO_ID Interface_ID(0x6c7b290a, 0x7c56423c)

//! \brief Informational class about ParamBlock2 objects loaded from file.
/*! This allows a plugin to detect when an obsolete version of itself has been loaded,
    so the plugin can migrate obsolete parameters to the newer version.
    The information is available only under the following conditions, indicating that an
    obsolete file was loaded:
    1) The current version of the ParamBlock2 uses the P_VERSION tag
    2) The loaded version either did not use P_VERSION, or the version numbers don't match
    3) The information is available only during a PostLoadCallback; it is deleted after load.
    To obtain a pointer to this class, query the plugin's ParamBlockDesc2 as follows:
    postLoadInfo = (IParamBlock2PostLoadInfo*)desc->GetInterface(IPARAMBLOCK2POSTLOADINFO_ID); */
class IParamBlock2PostLoadInfo : public BaseInterface
{
public:
	//! \brief Returns the interface ID of this class, IPARAMBLOCK2POSTLOADINFO_ID
	virtual Interface_ID	GetID() {return IPARAMBLOCK2POSTLOADINFO_ID;}
	//! \brief This method allows for future extention.
	/*! Returns itself if the parameter is IPARAMBLOCK2POSTLOADINFO_ID.
		Otherwise returns BaseInterface::GetInterface(id)
		\param[in] id The ID of the interface */
	virtual BaseInterface*	GetInterface(Interface_ID id)=0;

	//! \brief Returns the current ParamBlockDesc2 which this information is associated with
	virtual ParamBlockDesc2* GetParent()=0;
	//! \brief Returns the version number of the ParamBlock2 loaded from file.
	/*! This is either the P_VERSION number from the obsolete plugin, or if the plugin
	    did not use P_VERSION it is equal to the value 3DSMAX_VERSION when the file was saved */
	virtual DWORD			GetVersion()=0;
	//! \brief Returns the ID numbers of the parameters in the ParamBlock2 loaded from file.
	/*! The numbers are in their original order used by the obsolete plugin. */
	virtual IntTab&			GetParamLoaded()=0;  // Loaded param ID numbers
	//! \brief Returns a mapping from old parameter indices to current parameter indices.
	/*! Each entry is an index into the current ParamBlock2; the number of entries equals
	    the current number of parameters.  The parameters are listed in an order that matches
	    the order from the obsolete plugin.
	    This is used internally during load.  The reference numbering of the current ParamBlock2
	    is temporarily reordered, allowing objects and controllers referenced by the ParamBlock2
	    to load correctly even when the parameter order changes between plugin versions. */
	virtual IntTab&			GetParamReorder()=0; // Current param index numbers; ordered to match loaded params
};


/*! \remarks This function determines whether a param type is animatable. */
PB2Export bool IsParamTypeAnimatable(const ParamType2 t);



