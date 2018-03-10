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
#pragma once

#include "CoreExport.h"
// for base classes
#include "Noncopyable.h"
#include "baseinterface.h"
// for members
#include "autoptr.h"
#include "strclass.h"
#include "AnimPropertyList.h"
#include "interval.h"
#include "TrackHitRecord.h"
// cannot forward declare a Rect, because it gets confused with the windows Rect.
#include "box2.h"
#include "svcore.h"
#include "FileEnumConstants.h"
#include "ParamDimension.h"
// Includes for constants used with the Animatable Class
#include "TrackFlags.h"
#include "SysNodeContext.h"
#include "EditParamFlags.h"
#include "AnimatableFlags.h"
#include "AnimatableInterfaceIDs.h"

// forward declarations
class IObjParam;
class AnimEnum;
class BitArray;
class TimeMap;
class TrackClipObject;
class NoteTrack;
class AssetEnumCallback;
class INodeTab;
class AppDataChunk;
class IParamBlock2;
class ISave;
class ILoad;
class ICustAttribContainer;
class ParamBlockDesc2;

typedef UINT_PTR AnimHandle;

//
// This solves an incompatibility between projects compiled with /clr and without.  
// Under /clr, _M_CEE is defined and the windows header files define GetClassName 
// as an inline function.  Without /clr, the windows headers define GetClassName 
// as a macro, expanding to either GetClassNameA or GetClassNameW.  The macro 
// affects Animatable's interface by replacing its GetClassName function, which 
// then causes symbol mismatches at link time.  For compilations without /clr, the 
// following code eliminates the macro and replaces it with the inline function 
// defined by the window headers for /clr builds.  The code is copied from 
// winuser.h.
//
#ifdef GetClassName
#undef GetClassName
#endif
#ifndef _M_CEE
__inline
int
GetClassName(
			 HWND hWnd,
			 LPTSTR lpClassName,
			 int nMaxCount
			 )
{
#ifdef UNICODE
	return GetClassNameW(
#else
	return GetClassNameA(
#endif
		hWnd,
		lpClassName,
		nMaxCount
		);
}
#endif

/*! \sa  Class Interface, Class INode, \ref AnimatableFlags,
Class IGraphObjectManager, Class IGraphNode, Class Object\n\n
\par Description:
This is the base class for all animatable scene entity types, and for most plugin types. 
Methods are available for getting the ClassID and SuperClassID of the plug-in, deleting
this instance of the plug-in class, and parameter editing in the command panel.
There are also methods that deal with the sub-animatables of the plug-in. Most
of the track view related methods are here as well.
\par Method Groups:
See <a href="class_animatable_groups.html">Method Groups for Class Animatable</a>.
\par Data Members:
protected:\n\n
<b>unsigned long aflag;</b>\n\n
The flags. These may be manipulated using the methods <b>SetAFlag()</b>,
<b>ClearAFlag()</b>, and <b>TestAFlag()</b>. See
List of Animatable Flags.\n\n
<b>AnimPropertyList aprops;</b>\n\n
This is a table of properties that a plug-in may use for its own storage needs.
This table is also used by the system (for example Note Tracks and APP_DATA). A
plug-in may use this, for example, when a class has some data that is used
while its user interface is up. It can store the UI data on the property list
temporarily and not have to carry around the data when it is not needed. See
the methods of Template Class Tab for how to
add and delete items from the list. Also see the methods <b>GetProperty()</b>
and <b>SetProperty()</b> and Class AnimPropertyList. 
Note that it's not safe to copy Animatable objects because the properties
held in AnimPropertyList do not have a well-defined copy semantics*/
#pragma warning(push)
#pragma warning(disable:4100)
class Animatable : public InterfaceServer, public MaxSDK::Util::Noncopyable
{
		//! \todo: Remove reference to internal classes ISaveImp, ILoadImp
		friend class ISaveImp;
		friend class ILoadImp;

	public:	
		//! \brief A callback class for enumerating animatables.
		/*! This class is a callback object for the Animatable::EnumerateAllAnimatables() 
		method. The proc() method is called by the system. The Animatables are enumerated
		in order of creation*/
		class EnumAnimList: public MaxHeapOperators {
		public:
			//! \brief Destructor
			virtual ~EnumAnimList() {;}
			//! \brief This is the method called by system from Animatable::EnumerateAllAnimatables().
			/*! \param theAnim - A pointer to the Animatable
			\return Return true to continue the enumeration, false to stop the enumeration.
			*/
			virtual	bool proc(Animatable *theAnim)=0;
		};

	private:
		class AnimatableImplData;
		const MaxSDK::Util::AutoPtr<AnimatableImplData> mAnimatableImplData;
		DWORD tvflags1, tvflags2;
		DWORD aflag_ex;

	protected:
		DWORD aflag;
		AnimPropertyList aprops;

//! \name Construction, destruction of instances
//@{
		//! \brief Destructor
		/*! Instances of class Animatable and classes directly derived from it should be 
		deleted by calling Animatable::DeleteThis() on them, rather then calling the 
		delete operator.
		*/
		CoreExport virtual ~Animatable() = 0;

	public:	
		//! \brief Constructor
		CoreExport Animatable();

		//! \brief Deletes an instance of this class.
		/*! 3ds Max calls this method when it needs to delete a plugin object 
		 (an instance of a class derived from Animatable). 
		 Similarly, plugins that need to delete instances of an Animatable or 
		 a class directly derived from it via an Animatable pointer, should call this 
		 method instead of calling directly operator delete. Following these rules 
		 will ensure that the same memory manager is used to allocate and deallocate the object. 
		 The default implementation of this method deletes the object. Plugin instances
		 that never need to be deleted from the heap can overwrite this method to do nothing.
		 \note See the method ClassDesc::Create() for details on how Max allocates plugin objects.
		 \note See ReferenceMaker::DeleteMe() and ReferenceTarget::MaybeAutoDelete() for information 
		 on how plugin instances are deleted by the system.
		\remarks See
		<a href="ms-its:3dsmaxsdk.chm::/alloc_memory_allocation.html">Memory
		Allocation</a>.\n\n
		\sa
		<a href="ms-its:3dsmaxsdk.chm::/start_dll_functions.html">Plugin DLL Functions</a>, 
		Class ClassDesc.
		<a name="A_GM_anim_rollup" id="A_GM_anim_rollup"></a> 
		 */
		CoreExport virtual void DeleteThis();
//@}

//! \name Class identification
//@{
		//! \brief Retrieves the name of the plugin class
		/*! This name is usually used	internally for debugging purposes. 
		For Material plug-ins this method is used to put up the material "type" name 
		in the Material Editor.
		\param s Reference to a string filled in with the name of the plugin class
		*/
		virtual void GetClassName(MSTR& s) { s = _M("Animatable"); }  

		//! \brief Retrieves a constant representing the type of the plugin 
		/*! \return A super class id that uniquely identifies the type (category) of the 
		plugin. Note that several plugin classes can be of the same type, thus return
		the same super class id. Plugins are uniquely identified by their class ids.
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_super_class_ids.html">List of Super Class
		IDs</a>.
		\see SClass_ID
		*/
		CoreExport virtual SClass_ID SuperClassID() = 0;

		//! \brief Retrieves a constant that uniquely identifies the plugin class
		/*! This method must return the unique ID for the plugin class. If two ClassIDs 
		conflict, the system will only load the first conflicting one it finds. 
		A program (gencid.exe) is provided to generate unique class id values. 
		\return A class id that uniquely identifies a plugin class
		\sa  Class ClassID, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class IDs</a>.
		*/
		CoreExport virtual Class_ID ClassID();
//@}

		//! \brief Get the unique handle for an Animatable object
		/*! Every Animatable is given a handle value when allocated.
		The values are unique within a scene, but are <i>not</i> saved with the scene,
		and no guarantees are provided about handle values from one load to another.
		Handles are intended for use as sort/hash keys, when load and save are not required.
		They are safer than pointers, since the Animatable may be deleted, in which case
		the handle value is never reused for another Animatable within the current session,
		and searching for the Animatable via GetAnimByHandle() simply yields NULL.
		\param anim The Animatable object
		\return The unique handle value for the Animatable */
		CoreExport static AnimHandle GetHandleByAnim( Animatable* anim );

		//! \brief Get an Animatable object from its unique handle
		/*! Every Animatable is given a handle value when allocated.
		The values are unique within a scene, but are <i>not</i> saved with the scene,
		and no guarantees are provided about handle values from one load to another.
		Handles are intended for use as sort/hash keys, when load and save are not required.
		They are safer than pointers, since the Animatable may be deleted, in which case
		the handle value is never reused for another Animatable within the current session,
		and searching for the Animatable via GetAnimByHandle() simply yields NULL.
		\param handle The unique handle value for the Animatable
		\return The Animatable object, or NULL if the Animatable has been deleted,
		or if the handle is invalid */
		CoreExport static Animatable* GetAnimByHandle( AnimHandle handle );

		//! \brief Sets one or more bits in the Animatable flags
		/*! \param mask The bits to turn on in the Animatable flags*/
		void SetAFlag(DWORD mask) { 
			aflag |= mask; 
		}
		//! \brief Clears one or more bits in the Animatable flags
		/*! \param mask The bits to turn off in the Animatable flags*/
		void ClearAFlag(DWORD mask) { 
			aflag &= ~mask; 
		}

		//! \brief Tests one or more bits in the Animatable flags
		/*! \param mask The bits to test in the Animatable flags
		\return TRUE if one or more bit specified in the mask are also set in the 
		Animatable flags, otherwise, false.*/
		BOOL TestAFlag(DWORD mask) { 
			return (aflag & mask ? TRUE : FALSE); 
		}

		//! \brief Sets one or more bits in the Animatable extended flags
		/*! \param mask The bits to turn on in the Animatable extended flags*/
		void SetAFlagEx(DWORD mask) { 
			aflag_ex |= mask; 
		}

		//! \brief Clears one or more bits in the Animatable extended flags
		/*! \param mask The bits to turn off in the Animatable extended flags*/
		void ClearAFlagEx(DWORD mask) { 
			aflag_ex &= ~mask; 
		}

		//! \brief Tests one or more bits in the Animatable extended flags
		/*! \param mask The bits to test in the Animatable extended flags
		\return TRUE if one or more bit specified in the mask are also set in the 
		Animatable extended flags*/
		BOOL TestAFlagEx(DWORD mask) { 
			return (aflag_ex & mask ? TRUE : FALSE); 
		}

		//! \brief Requests an unique flag bit index 
		/*! The flag bit index is used with the Animatable::TestFlagBit(), Animatable::SetFlagBit(), 
		Animatable::ClearFlagBit(), and Animatable::ClearFlagBitInAllAnimatables() methods. 
		The flag bit index must be released using the	Animatable::ReleaseFlagBit() method. 
		The intended usage of the flag bit index is in enumerations where you want to 
		visit each Animatable only once, and the usage of the flag bit is of a temporary, 
		short term usage. The flag bits are not saved with the Animatable, nor copied via 
		the assign operator. The flag bits referred to for these methods are not associated 
		with the aflag or aflag_ex data members.
		\return The unique flag bit index*/
		CoreExport static int RequestFlagBit();
		
		//! \brief Releases the flag bit index
		/*! \param index The flag bit index to release*/
		CoreExport static void ReleaseFlagBit(int index);
		
		//! \brief Tests the specified flag bit
		/*! \param index The flag bit index to test
		\return TRUE if the flag bit is set*/
		CoreExport bool TestFlagBit(int index);
		
		//! \brief Sets the specified flag bit
		/*! \param index The flag bit index to set
		/*! \param newValue The value to set the flag bit index to*/
		CoreExport void SetFlagBit(int index, bool newValue = true);
		
		//! \brief Clears the specified flag bit
		/*! \param index The flag bit index to clear*/
		CoreExport void ClearFlagBit(int index);
		
		//! \brief Clears the specified flag bit in all Animatables
		/*! \param index The flag bit index to clear*/
		CoreExport static void ClearFlagBitInAllAnimatables(int index);
		
		//! \brief Enumerator to enumerate across all animatables.
		/*! Implemented by the System. The Animatables are enumerated in order of creation. 
		Use Animatable::EnumAnimTree when the enumerating the Animatables based on their
		hierarchy is required.
		EnumerateAllAnimatables is designed to be safe with recursive calls to 
		EnumerateAllAnimatables, and is designed to be safe when Animatables are added/deleted 
		during an enumeration.
		\param enumProcObject - The callback object called for each animatable.*/
		CoreExport static void EnumerateAllAnimatables(EnumAnimList &enumProcObject);


		/*! \remarks This is called to delete any item that can be rebuilt. For example, the
		procedural sphere object has a mesh that it caches. It could call
		<b>Mesh::FreeAll()</b> on the mesh from this method. This will free the
		vertex/face/uv arrays. If the sphere is ever evaluated again it can
		just rebuild the mesh. If an object (like a sphere) has modifiers
		applied to it, and those modifiers are not animated, then the result of
		the pipeline is cached in the node. So there is no reason for the
		sphere to also have a cache of its representation. Therefore when this
		method is called, the sphere can free the data of the mesh.
		\par Default Implementation:
		<b>{}</b> */
		virtual void FreeCaches() {}

		// 'create' is TRUE if parameters are being edited in the create branch.
		// 'removeUI' is TRUE if the object's UI should be removed.
		/*! \remarks This method is called by the system when the user may edit the item's
		(object, modifier, controller, etc.) parameters.
		\param ip Interface pointer. 
		The developer can use it to call methods such as <b>AddRollupPage()</b>. Note
		that this pointer is only valid between <b>BeginEditParams()</b> and
		<b>EndEditParams()</b>. It should not be used outside this interval.
		\param flags 
		Describe which branch of the command panel or dialog the
		item is being edited in. The following are possible values:\n\n
		<b>BEGIN_EDIT_CREATE</b>\n
		Indicates an item is being edited in the create branch.\n\n
		<b>BEGIN_EDIT_MOTION</b>\n
		Indicates a controller is being edited in the motion branch.\n\n
		<b>BEGIN_EDIT_HIERARCHY</b>\n
		Indicates a controller is being edited in the Pivot subtask of the
		hierarchy branch.\n\n
		<b>BEGIN_EDIT_IK</b>\n
		Indicates a controller is being edited in the IK subtask of the
		hierarchy branch.\n\n
		<b>BEGIN_EDIT_LINKINFO</b>\n
		Indicates a controller is being edited in the Link Info subtask of the
		hierarchy branch.\n\n
		\param prev	Pointer to an Animatable object. 
		This parameter may be used in the motion and hierarchy branches of the
		command panel. This pointer allows a plug-in to look at the ClassID of
		the previous item that was being edited, and if it is the same as this
		item, to not replace the entire UI in the command panel, but simply
		update the values displayed in the UI fields. This prevents the UI from
		'flickering' when the current item begins its edit. For example, if you
		are in the motion branch and are looking at an item's PRS controller
		values, and then select another item that is displayed with a PRS
		controller, the UI will not change - only the values displayed in the
		fields will change. If however you selected a target camera that has a
		lookat controller (not a PRS controller) the UI will change because a
		different set of parameters need to be displayed. Note that for items
		that are edited in the modifier branch this field can be ignored. */
		virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev = NULL) {} 
		/*! \remarks This method is called when the user is finished editing an objects
		parameters. The system passes a flag into the <b>EndEditParams()</b>
		method to indicate if the rollup page should be removed. If this flag
		is TRUE, the plug-in must un-register the rollup page, and delete it
		from the panel.
		\param ip An interface pointer. The developer may use the
		interface pointer to call methods such as
		<b>DeleteRollupPage()</b>.\n\n
		\param flags
		The following flag may be set:\n\n
		<b>END_EDIT_REMOVEUI</b>\n
		If TRUE, the item's user interface should be removed.\n\n
		\param next Animatable pointer.
		Can be used in the motion and hierarchy branches of the
		command panel. It allows a plug-in to look at the ClassID of
		the next item that was being edited, and if it is the same as this
		item, to not replace the entire UI in the command panel. Note that for
		items that are edited in the modifier branch this field can be ignored. */
		virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next = NULL) {} 

		// OLE-like method for adding new interfaces
		/*! \remarks This method provides a mechanism for extending the class in the future.
		In 3ds Max 4.0 there are new interfaces that are accessed by passing an
		id to this method and it will respond by returning the corresponding
		interface pointer.\n\n
		This method has been used however for a different purpose. It currently
		is used to determine if an object is of a particular class. With
		controllers for example, there is one base class Control, however there
		are many super classes (<b>CTRL_FLOAT_CLASS_ID</b>,
		<b>CTRL_SCALE_CLASS_ID</b>, etc.). If you wanted to find out if a given
		Animatable was a controller you would need to compare its SuperClassID
		to all the known types and only if it wasn't one of the known types
		could you be sure it wasn't a controller. Having to do this is
		inconvenient for a developer.\n\n
		Instead the Control class implements this method. It looks at the id,
		and if it matches a predefined constant <b>I_CONTROL</b>, it returns
		its <b>this</b> pointer. In this way, given any Animatable, it is easy
		to find out if it is a controller by simply asking for the control
		interface. There is a macro that does this:\n\n
		<b>#define GetControlInterface(anim)</b>\n\n
		<b>((Control*)anim-\>GetInterface(I_CONTROL))</b>\n\n
		A plug-in developer may use this macro as follows:\n\n
		<b>Control *c = GetControlInterface(anim);</b>\n\n
		This will either be NULL or a pointer to a valid controller.\n\n
		Note: Plug-in defined interfaces should be greater than the following
		value:\n\n
		<b>#define I_USERINTERFACE 0x0000ffff</b>\n\n
		If a plug-in implements this method for its own purposes, it would, in
		general, switch on the id and if it is not aware of the id it would
		call this method on the base class. Otherwise it could respond to the
		id as it needed. See the sample code below for the how the Control
		class implements this method.
		\param id The id of the interface.
		\par Default Implementation:
		<b>{ return NULL; }</b>
		\par Sample Code:
		The following is the Control class implementation of this method. It
		looks at the <b>id</b> passed, and if it matches <b>I_CONTROL</b> it
		returns its <b>this</b> pointer. Otherwise it calls the base class
		method.\n\n
		\code
		void* Control::GetInterface(ULONG id)
		{
		    if (id==I_CONTROL) {
		        return this;
		    }
		    else {
		        return Animatable::GetInterface(id);
		    }
		}
		\endcode  */
		CoreExport virtual void* GetInterface(ULONG id);
		/*! \remarks This method is not currently used. It is reserved for future use. Its
		purpose is for releasing an interface created with <b>GetInterface()</b>. */
		CoreExport virtual void ReleaseInterface(ULONG id, void *i);
		// from InterfaceServer
		/*! \remarks		Returns a pointer to the Base Interface for the interface ID passed.
		The default implementation of this method retrieves this information
		from the ClassDesc for the plug-in.\n\n
		Any future object-based interfaces should be allocated unique
		Interface_IDs (you can use Gencid.exe for this) and made available
		through this call.\n\n
		The default implementation of GetInterface(Interface_ID) looks up a
		standalone interface of the given ID on the object's ClassDesc. This
		gives access to standalone interfaces via any of a plug-in's objects,
		without having to dig around for the ClassDesc, so you should fall back
		to calling the default implementation if you don't recognize an ID in
		your implementation of GetInterface(Interface_ID).
		\param id The unique ID of the interface to get. See Class Interface_ID. */
		CoreExport virtual BaseInterface* GetInterface(Interface_ID id);

		// General method for adding properties, when
		// defining a new Interface would be too cumbersome
		/*! \remarks This is a general method for adding properties, when defining a new
		Interface would be too cumbersome. This method provides another way to
		extend the class without actually adding any methods. Sample code that
		implements this method to add properties to the property list is in
		<b>/MAXSDK/SAMPLES/CONTROLLERS/PATHCTRL.CPP</b>. See below.
		\param id The id for the property.
		\param data A pointer to the data to store.
		\return  Nonzero if the property was set; otherwise zero.
		\par Default Implementation:
		<b>{ return 0; }</b>
		\par Sample Code:
		This code is from <b>/MAXSDK/SAMPLES/CONTROLLERS/PATHCTRL.CPP</b>. It
		is used to save the inverse kinematics user interface parameters of the
		path controller. It saves the property data on the <b>aprops</b>
		property list. See the Data Members at the beginning of Animatable for
		details on <b>aprops</b>.\n\n
		\code
		int PathPosition::SetProperty(ULONG id, void *data)
		{
			if (id==PROPID_JOINTPARAMS) {
				if (!data) {
					int index = aprops.FindProperty(id);
					if (index>=0) {
						aprops.Delete(index,1);
					}
				}
				else {
					JointParamsPath *jp = (JointParamsPath*)GetProperty(id);
					if (jp) {
						*jp = *((JointParamsPath*)data);
						delete (JointParamsPath*)data;
					}
					else {
						aprops.Append(1,(AnimProperty**)&data);
					}
				}
				return 1;
			} else
			if (id==PROPID_INTERPUI) {
				if (!data) {
					int index = aprops.FindProperty(id);
					if (index>=0) {
						aprops.Delete(index,1);
					}
				}
				else {
					InterpCtrlUI *ui = (InterpCtrlUI*)GetProperty(id);
					if (ui) {
						*ui = *((InterpCtrlUI*)data);
					}
					else {
						aprops.Append(1,(AnimProperty**)&data);
					}
				}
				return 1;
			}
			else {
				return Animatable::SetProperty(id,data);
			}
		}
		\endcode  */
		CoreExport virtual int SetProperty(ULONG id, void *data);
		/*! \remarks This method is used to retrieve a property specified by the id passed
		(as stored by <b>SetProperty()</b>).\n\n
		Note for 3ds Max version 1.1:\n\n
		Two new property IDs have been added:\n\n
		<b>PROPID_CLEARCACHES</b>: When passed to a texture map or material,
		the material should dump any of its caches. For example, the bitmap
		texture responds to this by freeing the bitmap from memory. For sample
		code see <b>/MAXSDK/SAMPLES/MATERIALS/BMTEX.CPP</b>.\n\n
		<b>PROPID_HAS_WSM</b>: When passed to an INode, will return TRUE if the
		node has World Space Modifiers applied to it or FALSE if it does not.
		For sample code see <b>/MAXSDK/SAMPLES/IMPEXP/3DSEXP.CPP</b>.\n\n
		Note for 3ds Max version 1.2:\n\n
		A new <b>id</b> has been created and assigned the constant:\n\n
		<b>#define PROPID_EVAL_STEPSIZE_BUG_FIXED 0x1000</b>.\n\n
		This only effects the evaluation of objects when rendering them using
		motion blur. Motion blur works by evaluating the object numerous times
		(at fractions of a frame apart) and combining these images by blending
		them together.\n\n
		Originally, 3ds Max would make these evaluations in reverse order
		within a sub-frame -- from the last one, to the second to the last one,
		back to the first one. There is a problem with this for certain
		plug-ins that need to compute their state from time 0 forward. For
		these objects, the backwards approach may be too computationally
		intensive.\n\n
		Both the forward and backward approaches exist in 3ds Max and the
		developer may choose which method to use. 3ds Max interrogates the
		object to see how it should handle the evaluation process -- either
		going backwards or forwards. It calls this method with <b>id</b> set to
		the constant <b>PROPID_EVAL_STEPSIZE_BUG_FIXED</b>. If a plug-in
		implements this method to return nonzero, it means the plug-in works
		correctly using forward stepping, and 3ds Max will use that approach.
		If a plug-in does not implement this method and handle the <b>id</b> of
		<b>PROPID_EVAL_STEPSIZE_BUG_FIXED</b> it will return the default value
		of zero. This means the older method of backwards evaluation will be
		used.\n\n
		Therefore, a plug-in object that wants to handle motion blur using
		forward stepping should implement this method, and if passed an
		<b>id</b> of <b>PROPID_EVAL_STEPSIZE_BUG_FIXED</b>, should return
		nonzero.
		\param id The id of the property to retrieve.
		\par Default Implementation:
		<b>{ return NULL; }</b>
		\par Sample Code:
		This code is from <b>/MAXSDK/SAMPLES/CONTROLLERS/PATHCTRL.CPP</b>. It
		is used to restore the inverse kinematics user interface parameters of
		the path controller. It retrieves the property data on the
		<b>aprops</b> property list. See the Data Members at the beginning of
		Animatable for details on <b>aprops</b>.\n\n
		\code
		void* PathPosition::GetProperty(ULONG id)
		{
			if (id==PROPID_INTERPUI || id==PROPID_JOINTPARAMS) {
				int index = aprops.FindProperty(id);
				if (index>=0) {
					return aprops[index];
				}
				else {
					return NULL;
				}
			}
			else {
				return Animatable::GetProperty(id);
			}
		}
		\endcode */
		CoreExport virtual void *GetProperty(ULONG id);

		/*! \remarks The system uses a virtual array mechanism to access the sub-anims of a
		plug-in. This method returns the total number of sub-anims maintained
		by the plug-in. If a plug-in is using a parameter block to manage its
		parameters it should just return 1 for all the parameters directed by
		the parameter block.
		\return  The number of sub-anims used by the plug-in.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual	int NumSubs()  { return 0; }     

		// access the ith sub-animatable
		/*! \remarks This method returns a pointer to the 'i-th' sub-anim. If a plug-in is
		using a parameter block to manage all its parameters it should just
		return a pointer to the parameter block itself from this method. This
		method may return NULL so developers need to check the return value
		before calling other sub anim methods (such as <b>SubAnimName()</b>).
		\param i This is the index of the sub-anim to return.
		\par Default Implementation:
		<b>{ return NULL };</b> */
		virtual	Animatable* SubAnim(int i) { return NULL; }  
		// get name of ith subanim
		/*! \remarks This method returns the name of the 'i-th' sub-anim to appear in track
		view. The system has no idea what name to assign to the sub-anim (it
		only knows it by the virtual array index), so this method is called to
		retrieve the name to display. Developer need to make sure the 'i-th'
		<b>SubAnim()</b> is non-NULL or this method will fail.
		\param i The index of the parameter name to return
		\return  The name of the 'i-th' parameter. */
		CoreExport virtual MSTR SubAnimName(int i);  
		// return TRUE and you won't appear in the TreeView however your children will.
		/*! \remarks This method indicates to the system that this anim should not appear in
		the Track %View. Note: Track %View was formally referred to as Tree
		View. This is what parameter blocks do for example. They don't show up
		in track view, just their sub-anims do. This prevents the extra level
		of the parameter block from appearing.
		\return  Return TRUE to not appear in the Track %View. Note that if you
		return TRUE your children will appear in the track view regardless.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL BypassTreeView() { return FALSE; } 
		// return TRUE and you won't appear in the TrackBar however your children will.
		/*! \remarks				This method indicates to the system that this anim should not appear in
		the Track Bar. The anim won't show up in the Track Bar, just its
		sub-anims will. This function is similar to BypassTreeView(), but
		refers to the Track Bar instead of the Track %View.
		\return  Return TRUE to not appear in the Track Bar. Note that if you
		return TRUE your children will appear in the Track Bar regardless.
		\par Default Implementation:
		<b>{ return BypassTreeView(); }</b> */
		virtual BOOL BypassTrackBar() { return BypassTreeView(); } 
		// return TRUE and you won't appear as a property in the scripter however your children will.
		/*! \remarks				Use this method in order to cause parameters in this Animatable (as a
		sub-anim) to appear to reside at the level of the parent Animatable in
		the scripter. Return TRUE and this Animatable won't appear as a
		property in the scripter however it's sub-anims children will. The
		default implementation returns FALSE indicating it will appear normally.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL BypassPropertyLevel() { return FALSE; } 
		// return TRUE and you won't appear as a property in the scripter.
		/*! \remarks				This method controls the visibility of this Animatable and all of it
		sub-anims to appear as properties in the scripter. Return TRUE and it
		won't nor will it's sub-anims. Returning FALSE (the default
		implementation) causes this Animatable and it's sub-anims to appear as normal.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL InvisibleProperty() { return FALSE; } 
		/*! \remarks This method is called to assign the controller to the sub-anim whose index is passed.
		\param control	The controller to assign.
		\param subAnim The index of the sub-anim to assign the controller to.
		\par Default Implementation:
		<b>{ return FALSE; }</b>
		\return  Returns TRUE if the controller was assigned; otherwise FALSE. */
		virtual BOOL AssignController(Animatable *control,int subAnim) { return FALSE; }

		//! \brief Return true if we can reassign the subanim specified.
		/*! Implement this function to return false if we wish to prevent any 
		subanim from being replaced.  If this method returns false for any id, that subanim will not
		be able to be re-assigned via the UI or MaxScript controls.  Note that this will
		not prevent re-assigning in C++ in any way (via the ReplaceReference or AssignController
		functions).
		\param subAnum - The ID of the SubAnim to check.
		\return TRUE if it is legal to assign a new Animatable to the specified SubAnim */
		virtual BOOL CanAssignController(int subAnim) { return TRUE; }

		// Used to allow deletion of controllers from the track view
		/*! \remarks Returns TRUE if the specified sub-anim controller can be deleted; otherwise FALSE.\n\n
		A new "Delete Controller" button has been added to the Track %View
		toolbar that is enabled when one or more delete-able tracks are
		selected. This method allows a plug-in to indicate to the Track %View
		that one or more of its sub-controllers are delete-able. This provides
		a way to allow the user to delete node sub-controllers such as the
		visibility track, "Image Motion Blur Multiplier", "Object Motion Blur
		On/Off", etc. If the user selects one of the above-mentioned tracks in
		the Track %View the "Delete Controller" button will become available.
		\param i The zero based index of the sub-anim.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL CanDeleteSubAnim(int i) { return FALSE; }
		/*! \remarks				This method is called to delete the specified sub-anim controller. See
		the remarks in <b>CanDeleteSubAnim()</b> above.
		\param i The zero based index of the sub-anim.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteSubAnim(int i) {}
			
		/*! \remarks				Return the suggested color to draw a sub-anim's function curve. For
		example, the independent X, Y, Z position controller implements this
		method to return the suggested color for each of it's sub-controllers.
		The Euler Angle Controller uses these so its 3 sub-controllers are
		drawn in different colors.
		\param subNum The index of the sub-anim.
		\return  One of the following values:\n\n
		<b>PAINTCURVE_GENCOLOR</b>\n\n
		<b>PAINTCURVE_XCOLOR</b>\n\n
		<b>PAINTCURVE_YCOLOR</b>\n\n
		<b>PAINTCURVE_ZCOLOR</b>
		\par Default Implementation:
		<b>{return PAINTCURVE_GENCOLOR;}</b> */
		virtual DWORD GetSubAnimCurveColor(int subNum) { return PAINTCURVE_GENCOLOR; }

		// Converts an anim index to a ref index or returns -1 if there is no
		// corrispondance. This is used for copying and pasting in the track
		// view. If a client does not wish an anim to be copy/pastable then
		// it can return -1 even if there is a corrisponding ref num.
		/*! \remarks This method is used for copying and pasting in the track view. It
		converts an anim index to a reference index or returns -1 if there is
		no correspondence. If a client does not wish an anim to be copied or
		pasted then it can return -1 even if there is a corresponding reference
		num.
		\param subNum The anim index to return the corresponding reference index of.
		\par Default Implementation:
		<b>{ return -1}</b>
		\return  The reference index corresponding to the anim index passed.
		Return -1 if there is no correspondence. */
		virtual int SubNumToRefNum(int subNum) { return -1; }

		// In addition to SubNumToRefNum(), if an anim doesn't want to be coppied it 
		// can return FALSE from this function
		/*! \remarks In addition to <b>SubNumToRefNum()</b>, if an anim doesn't want to be
		copied (via Track %View or the Edit %Modifier Stack 'Copy' button) it
		can return FALSE from this method, otherwise it can use the default
		implementation to return TRUE.
		\par Default Implementation:
		<b>{return TRUE;}</b> */
		virtual BOOL CanCopyAnim() { return TRUE; }

		// An anim can implement this to reutrn FALSE to prohibit make unique
		/*! \remarks An anim can implement this method to return FALSE to prohibit make
		unique from being applied to it.
		\par Default Implementation:
		<b>{return TRUE;}</b> */
		virtual BOOL CanMakeUnique() { return TRUE; }
		
		// Non-zero only for nodes.
		/*! \remarks This method is used internally. */
		virtual	int NumChildren() { return 0; }   
		// access the ith child
		/*! \remarks This method is used internally. */
		virtual Animatable* ChildAnim(int i) { return NULL; } 
		// For nodes only
		/*! \remarks This method is used internally. */
		CoreExport virtual MSTR NodeName(); 	   

		/*! \remarks Implemented by the System.\n\n
		This method recursively enumerates the Animatable hierarchy. It will
		call the <b>AnimEnum::proc()</b> method passing it the anim, that
		anim's parent (the client), and the sub-anim index of that anim to the
		client, for every anim and sub-anim in the hierarchy.
		\param animEnum The callback object, called once for each sub anim from 0 to subNum-1.
		See Class AnimEnum.\n\n
		\param client The client anim. This is the Animatalbe whose sub-anims are enumerated.\n\n
		\param subNum The sub-anim index of the client at which to begin the enumeration.
		Pass 0 to do them all.
		\return  One of the following values:\n\n
		<b>ANIM_ENUM_PROCEED</b>\n\n
		<b>ANIM_ENUM_STOP</b>\n\n
		<b>ANIM_ENUM_ABORT</b> */
		CoreExport int EnumAnimTree(AnimEnum *animEnum, Animatable *client, int subNum);
		/*! \remarks Implemented by the System.\n\n
		This method is used to determine if this Animatable has children or
		sub-anims. The type passed indicates what is tested.
		\param type	One of the following values:\n\n
		<b>0</b>: Test for node children.\n\n
		<b>1</b>: Test for sub-anims.
		\return  Nonzero if the item has children or sub-anims; otherwise zero.
		*/
		CoreExport int HasSubElements(int type=0); 		

		/*! \remarks This method is called once at the beginning of each render. A plug-in
		can use this method to do any work required before a rendering actually
		begins. For example, some of the standard 3ds Max plug-ins use this
		method to toggle between their 'viewport' state and the 'rendering'
		state. The Optimize modifier has two settings, one for the viewports
		and one for the rendering. When this method is called it then performs
		the switch from viewport to renderer.
		\param t The time that the render is beginning.
		\param flags The following flag value can be checked:\n\n
		<b>RENDERBEGIN_IN_MEDIT</b>\n
		Indicates that the render is occurring in the Material Editor.
		\return  Nonzero if the method is implemented; otherwise 0.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int RenderBegin(TimeValue t, ULONG flags=0) { return 0; }
		// called once at the end of each render
		/*! \remarks This method is called once at the end of each render.
		\param t The time of the last rendered frame.
		\return  Nonzero if the method is implemented; otherwise 0.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int RenderEnd(TimeValue t) { return 0; }

		// edit the track or parameters
		virtual void EditTrack() { assert(0); } 
		
		// Get the number of keys and the time of the ith key.
		/*! \remarks This method returns the number of keys managed by the plug-in, or
		<b>NOT_KEYFRAMEABLE</b> if it does not work with keys.
		\par Default Implementation:
		<b>{return NOT_KEYFRAMEABLE;}</b> */
		virtual int NumKeys() { return NOT_KEYFRAMEABLE; }
		/*! \remarks This method returns the time of the key specified by <b>index</b>.
		\param index Specifies the key whose time should be returned.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual TimeValue GetKeyTime(int index) { return 0; }
		/*! \remarks Returns the index of the key at time <b>t</b> or <b>-1</b> if no key is
		found at the specified time.
		\param t Specifies the time at which to retrieve the key index.
		\par Default Implementation:
		<b>{return -1;}</b> */
		virtual int GetKeyIndex(TimeValue t) { return -1; }
		/*! \remarks An item should implement this method to allow the Key Mode button in
		3ds Max's UI to function properly. If Key Mode is set, and the user
		clicks the Previous Key or Next Key button, this method will be called
		to retrieve the next or previous key.
		\param t The current time (frame slider position).
		\param flags One or more of the following values:\n\n
		<b>NEXTKEY_LEFT</b> \n
		Search to the left.\n\n
		<b>NEXTKEY_RIGHT</b> \n
		Search to the right.\n\n
		<b>NEXTKEY_POS</b> \n
		Next position key.\n\n
		<b>NEXTKEY_ROT</b> \n
		Next rotation key.\n\n
		<b>NEXTKEY_SCALE</b> \n
		Next scale key.\n
		\param nt The time of the previous or next key is returned here.
		\return  TRUE if the key time was retrieved; otherwise FALSE.
		\par Default Implementation:
		<b>{ return FALSE;}</b> */
		virtual BOOL GetNextKeyTime(TimeValue t, DWORD flags, TimeValue &nt) { return FALSE; }
		/*! \remarks This method is called to copy or interpolate a new key from a source
		time to a destination time.
		\param src The source time.
		\param dst The destination time.
		\param flags These filter flags are passed to a transform (Matrix3) controller. The
		TM can decide what to do with them. They have obvious meaning for the
		PRS controller. One or more of the following values:\n\n
		<b>COPYKEY_POS</b>\n Copy the position key.\n\n
		<b>COPYKEY_ROT</b>\n Copy the rotation key.\n\n
		<b>COPYKEY_SCALE</b> Copy the scale key. */
		virtual void CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags) {} 
		/*! \remarks This method is called to delete the key at the specified time.
		\param t Specifies the time to delete the key.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteKeyAtTime(TimeValue t) {}
		/*! \remarks Returns TRUE if there is a key of the specified type at the specified
		time; otherwise FALSE.
		\param t Specifies the time to check for a key.
		\param flags One or more of the following values:\n\n
		<b>KEYAT_POSITION</b>\n
		<b>KEYAT_ROTATION</b>\n
		<b>KEYAT_SCALE</b>
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL IsKeyAtTime(TimeValue t, DWORD flags) { return FALSE; }
		
		// The value returned from these two methods should be the number of keys 
		// skipped because their times were before range.Start(). So sel[i] is the 
		// selection state for the offset+ith key.
		/*! \remarks This method is called to build a table of time values, one time for
		each key within the interval passed. The plug-in should load up the
		table passed with the time of each key present over the specified time
		range.
		\param times The table of time values to build. See Class Tab.\n\n
		\param range The range of time over which to retrieve the key times. See
		Class Interval.\n\n
		\param flags One of the following values:\n\n
		<b>KEYAT_POSITION </b>\n Return for Position keys only.\n\n
		<b>KEYAT_ROTATION </b>\n Return for Rotation keys only.\n\n
		<b>KEYAT_SCALE </b>\n Return for Scale keys only.
		\return  The plug-in should return an offset so the system can access
		the keys using an index. Thus it should return the number of keys
		skipped because their times were before <b>range.Start()</b>. For
		example, say the first keyframe in the interval passed was actually the
		third key overall. The plug-in should return 2 (two keys preceded the
		first one stored). In this way, the system can access the key as the
		i-th key in the table plus 2.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int GetKeyTimes(Tab<TimeValue>& times, Interval range, DWORD flags) { return 0; }		
		/*! \remarks When this method is called, the plug-in should update the BitArray
		<b>sel</b> to indicate if its keys present in the interval passed are
		selected or deselected.
		\param sel The bit array to update, one bit for each key within the interval
		<b>range</b>. If the key is selected, the corresponding bit should be
		1, otherwise it should be 0. See Class BitArray.
		\param range The range of time over which to retrieve the key selected state. See
		Class Interval.
		\param flags One or more of the following values:\n\n
		<b>KEYAT_POSITION </b>\n Return for Position keys only.\n\n
		<b>KEYAT_ROTATION </b>\n Return for Rotation keys only.\n\n
		<b>KEYAT_SCALE </b> Return for Scale keys only.
		\note If the flags are passed as 0, use ALL keys within the range.
		\return  The number of keys skipped because their times were before
		<b>range.Start()</b>.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int GetKeySelState(BitArray& sel, Interval range, DWORD flags) { return 0; }

		// TreeView Methods
		/////////////////////////////////////////////////////////////
		// the 'type' parameter specifies whether to to operate on the tree based on 
		// the node parent/child relationship (TRACKVIEW_NODE) or the animatable/subanim 
		// relationship (TRACKVIEW_ANIM). The 'tv' parameter specifies which track view.
		// Each track view uses one bit, there can be up to MAX_TRACK_VIEWS
		/*! \remarks Implemented by the System.\n\n
		This method may be called to open the specified Track %View entry. The
		type parameter indicates if the child tree or the sub-anim (parameter)
		tree is opened.
		\param type This value may be either 0 or 1. If 0, the child tree is opened. If 1,
		the sub-anim tree is opened.
		\param tv This parameter specifies which Track %View(s) are altered, one bit for each Track
		%View. The open/closed state is independent for each
		Track %View. The low-order 16 bits represent the 16 track views. */
		CoreExport void OpenTreeEntry(int type, DWORD tv);
		/*! \remarks Implemented by the System.\n\n
		This method may be called to close the specified Track %View entry. The
		type parameter indicates if the child tree or the sub-anim tree is
		closed.
		\param type This value may be either 0 or 1. If 0, the child tree is closed. If 1,
		the sub-anim (parameter) tree is closed.
		\param tv This parameter specifies which Track %View(s) are altered, one bit for each Track
		%View. The low-order 16 bits represent the 16 track views. */
		CoreExport void CloseTreeEntry(int type, DWORD tv);
		/*! \remarks Implemented by the System.\n\n
		Returns nonzero if the specified tree is opened for this item, and zero
		if it is closed.
		\param type This value may be either 0 or 1. If 0, the child tree is checked. If 1,
		the sub-anim (parameter) tree is checked.
		\param tv This parameter is available in release 2.0 and later only.
		Specifies which Track %View to check -- one bit per Track %View. */
		CoreExport int IsTreeEntryOpen(int type, DWORD tv);
		
		// Track view selected state
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Implemented by the System.\n\n
		Returns TRUE if this animatable is selected in the specified Track
		%View; FALSE if not selected.
		\param tv Specifies which Track %View to check -- one bit per Track %View. */
		CoreExport BOOL GetSelInTrackView(DWORD tv);
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Implemented by the System.\n\n
		Sets the state of this animatable to selected or deselected in the
		specified Track %View.
		\param tv Specifies which Track %View to check -- one bit per Track %View.
		\param sel TRUE to select; FALSE to deselect. */
		CoreExport void SetSelInTrackView(DWORD tv, BOOL sel);

		// Track view selection sets: 'which' should be >=0 and <MAX_TRACKVIEW_SELSETS
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Implemented by the System.\n\n
		Returns TRUE if this animatable is in the specified selection set;
		otherwise FALSE.
		\param which Indicates the Track %View selection set to check -- this should be
		<b>\>=0</b> and <b>\<MAX_TRACKVIEW_SELSETS</b> */
		CoreExport BOOL InTrackViewSelSet(int which);
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Implemented by the System.\n\n
		Sets the selected or deselected state of this animatable in the
		specified selection set.
		\param which Indicates the Track %View selection set to modify -- this should be
		<b>\>=0</b> and <b>\<MAX_TRACKVIEW_SELSETS</b>\n\n
		\param inOut TRUE for in; FALSE for out. */
		CoreExport void SetTrackViewSelSet(int which, BOOL inOut);

		// The tracks time range:
		/*! \remarks Implemented by the System.\n\n
		Returns an interval representing the tracks time range, based on the
		flags passed.
		\param flags One or more of the following values:\n\n
		<b>TIMERANGE_SELONLY</b>\n The bounding interval of selected keys only.\n\n
		<b>TIMERANGE_ALL</b>\n Whatever the channel's time range is - usually the bounding interval of
		all keys.\n\n
		<b>TIMERANGE_CHILDNODES</b>\n The node's time range should include its child nodes.\n\n
		<b>TIMERANGE_CHILDANIMS</b>\n A animatable's child anim ranges should be included.
		\return  An interval representing the tracks time range. */
		CoreExport virtual Interval GetTimeRange(DWORD flags);
		/*! \remarks This method is called to change the range of the anim (usually a
		controller) to the given range. This is the range that is used to
		compute the Out of Range Types. For example, this method may be called
		when the user is working in Position Range mode in the Track %View.\n\n
		Keyframe controllers generally support this method. Other controllers
		may or may not support this method. For example, a procedural
		controller may want to maintain a range upon which the animation is
		based. The user may then move the range bar around to move the
		procedural animation around.\n\n
		The range passed is the range used to compute the Out of Range Types.
		This may be used for example with the Loop ORT to extend the range,
		either past the last key or before the first key, so there is some time
		to loop back to the start.\n\n
		The 3ds Max keyframe controllers maintain an interval that is their
		range. It is normally defined to be the first key to the last key. If
		the user goes into Position Range mode and moves the range around, this
		method is called. The keyframe controllers set a flag to indicate that
		the range is no longer linked to the first key or the last key. Then
		the range is stored in the interval, and this is considered the 'in
		range' portion of the controller. If time is evaluated outside of this
		range it applies the ORTs to determine the value.
		\param range The new range for the anim.\n\n
		\param flags
		<b>EDITRANGE_LINKTOKEYS</b>\n
		If this flag is set, the controller should re-establish the link
		between the start and end keys and its range. This is passed if the
		user presses the link to keys button in Track %View. Thus, if one of
		the ends of the interval is at a key, link it to the key so that if the
		key moves, the interval moves.
		\par Default Implementation:
		<b>{}</b> */
		virtual void EditTimeRange(Interval range, DWORD flags) {};
		
		// Operations to a selected block of time		
		/*! \remarks This method is called to delete the specified interval of time (or the
		keys within the interval).
		\param iv The interval of time to delete.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.\n\n
		<b>TIME_NOSLIDE</b>\n Delete any keys in the interval but don't actually remove the block of time.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteTime(Interval iv, DWORD flags) {}
		/*! \remarks This method is called to reverse the data within the specified
		interval. For example, if the interval passed is from frame 10 to 20,
		and there is a key at frame 12, the key should be moved to frame 18.
		Considered another way, if all the times were normalized, and there was
		a value <b>n</b> between 0 and 1, <b>n</b> should be changed to
		<b>1-n</b>.
		\param iv The interval of time over which to reverse the data.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::ReverseTime( Interval iv, DWORD flags )
		{
			Interval test = TestInterval(iv,flags);
			int n = keys.Count();
			HoldTrack();
			for ( int i = 0; i < n; i++ ) {
				if (keys[i].TimeLocked()) continue;
				if ( test.InInterval(keys[i].time) ) {
					TimeValue delta = keys[i].time - iv.Start();
					keys[i].time = iv.End()-delta;
				}
			}
			keys.KeysChanged();
			keys.CheckForDups();
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		\endcode */
		virtual void ReverseTime(Interval iv, DWORD flags) {}
		/*! \remarks This method is called to scale an interval of time by the specified
		scale factor.
		\param iv The interval of time to scale. The origin of the scale is at <b>iv.Start()</b>.
		\param s The scale factor for the time.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::ScaleTime( Interval iv, float s)
		{
			int n = keys.Count();
			TimeValue delta = int(s*float(iv.End()-iv.Start())) + iv.Start() - iv.End();
			HoldTrack();
			for ( int i = 0; i < n; i++ ) {
				if (keys[i].TimeLocked()) continue;
				if ( iv.InInterval(keys[i].time) ) {
					keys[i].time =
						int(s*float(keys[i].time - iv.Start())) + iv.Start();
				} else
				if (keys[i].time > iv.End()) {
					keys[i].time += delta;
				}
			}
			keys.KeysChanged();
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		\endcode */
		virtual void ScaleTime(Interval iv, float s) {}
		/*! \remarks This method is called to insert the specified amount of time at the
		specified insertion point.
		\param ins The time to begin the insertion.
		\param amount The amount of time to insert.
		\par Default Implementation:
		<b>{}</b> */
		virtual void InsertTime(TimeValue ins, TimeValue amount) {}
		
		// If an anim supports the above time operations it should return TRUE from 
		// this method. Returning TRUE enables time selection on the track view for 
		// the track.
		/*! \remarks If an anim supports time operations in the track view (cut, copy,
		paste, etc.), it should implement this method to return TRUE. When it
		is FALSE the user cannot select blocks of time in the anim's track.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL SupportTimeOperations() { return FALSE; }

		// Operations to keys
		/*! \remarks The method is called to update the keys specified by the flags, using
		the TimeMap passed. The plug-in should go through the specified keys
		and change their <b>time</b> to <b>TimeMap::map(time)</b>. See the
		sample code below for how this is done.
		\param map Point to instance of Class TimeMap.\n\n
		\param flags The flags indicate the keys to operate on. One or more of the following
		values:\n\n
		<b>TRACK_DOSEL</b>\n Selected keys only.\n\n
		<b>TRACK_DOALL</b>\n All the keys, ignore their selection state.\n\n
		<b>TRACK_SLIDEUNSEL</b>\n Slide unselected keys to the right. Keys are slid by the amount the
		last key was transformed.\n\n
		<b>TRACK_RIGHTTOLEFT</b>\n Enumerate right to left. If <b>TRACK_SLIDEUNSEL</b> is set, keys will
		slide to the left.\n\n
		<b>TRACK_DOSUBANIMS</b>\n Sub-Animatables keys as well.\n\n
		<b>TRACK_DOCHILDNODES</b>\n Child Nodes keys as well\n\n
		<b>TRACK_MAPRANGE</b>\n The range, if not locked to first and last key, should be mapped as well.
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::MapKeys(TimeMap *map,DWORD flags )
		{
			int n = keys.Count();
			BOOL changed = FALSE;
			if (!n) goto doneMapKeys;
			HoldTrack();
			if (flags&TRACK_DOALL) {
				for (int i = 0; i < n; i++) {
					if (keys[i].TimeLocked()) continue;
					keys[i].time = map->map(keys[i].time);
					changed = TRUE;
				}
			} else if (flags&TRACK_DOSEL) {
				BOOL slide = flags&TRACK_SLIDEUNSEL;
				TimeValue delta = 0, prev;
				int start, end, inc;
				if (flags&TRACK_RIGHTTOLEFT) {
					start = n-1;
					end = -1;
					inc = -1;
				}
				else {
					start = 0;
					end = n;
					inc = 1;
				}
				for (int i = start; i != end; i += inc) {
					if (keys[i].TimeLocked()) continue;
					if (keys[i].TestKFlag(KEY_SELECTED)) {
						prev = keys[i].time;
						keys[i].time =
							map->map(keys[i].time);
						delta = keys[i].time - prev;
						changed = TRUE;
					}
					else if (slide) {
						keys[i].time += delta;
					}
				}
			}
			if (flags&TRACK_MAPRANGE && keys.TestTFlag(RANGE_UNLOCKED)) {
				TimeValue t0 = map->map(keys.range.Start());
				TimeValue t1 = map->map(keys.range.End());
				keys.range.Set(t0,t1);
			}
			if (changed) {
				keys.KeysChanged();
				ivalid.SetEmpty();
				NotifyDependents(FOREVER, PART_ALL,
					REFMSG_CHANGE);
			}
			doneMapKeys:
			Animatable::MapKeys(map,flags);
		}
		\endcode  */
		CoreExport virtual void MapKeys(TimeMap *map, DWORD flags);
		/*! \remarks This method is called to delete keys, as specified by the flags passed.
		\param flags One or more of the following values:\n\n
		<b>TRACK_DOSEL</b>\n Delete selected keys only.\n\n
		<b>TRACK_DOALL</b>\n Delete all keys (ignore selection state).\n\n
		<b>TRACK_SLIDEUNSEL</b>\n Slide unselected keys to the right.\n\n
		<b>TRACK_RIGHTTOLEFT</b>\n Enumerate right to left. If <b>TRACK_SLIDEUNSEL</b> is set, keys will
		slide to the left.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteKeys(DWORD flags) {}
		/*! \remarks Deletes the key specified by the index passed.
		\param index The index of the key to delete.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteKeyByIndex(int index) {}
		/*! \remarks This method is called to select or deselect a set of keys identified by
		the <b>TrackHitTab</b> and the specified flags.
		\param sel The table of track hit records. See Class TrackHitRecord and
		Class Tab. Note the following: <b>typedef Tab\<TrackHitRecord\> TrackHitTab;</b>\n\n
		\param flags Either <b>SELKEYS_SELECT</b>, <b>SELKEYS_DESELECT</b>, or a combination
		of <b>SELKEYS_CLEARKEYS</b> and <b>SELKEYS_CLEARCURVE</b> will be specified.\n\n
		One or more of the following values:\n\n
		<b>SELKEYS_SELECT</b>\n The keys should be selected.\n\n
		<b>SELKEYS_DESELECT</b>\n The keys should be deselected.\n\n
		<b>SELKEYS_CLEARKEYS</b>\n All keys should be deselected.\n\n
		<b>SELKEYS_CLEARCURVE</b>\n All keys on the function curve should be deselected.\n\n
		<b>SELKEYS_FCURVE</b>\n Indicates that we are operating on the keys of a function curve, and
		not of a track.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SelectKeys(TrackHitTab& sel, DWORD flags) {}
		// this is called on the client when the client takes over control of an anims fcurve
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method is called on the client when the client takes over control
		of an anims function curves. It's called to select or deselect a set of
		keys identified by the <b>TrackHitTab</b> and the specified flags.
		\param subNum The index of the sub-anim to select or deselect\n
		\param sel The table of track hit records. See Class TrackHitRecord and
		Class Tab. Note the following:
		<b>typedef Tab\<TrackHitRecord\> TrackHitTab;</b>\n\n
		\param flags Either <b>SELKEYS_SELECT</b>, <b>SELKEYS_DESELECT</b>, or a combination
		of <b>SELKEYS_CLEARKEYS</b> and <b>SELKEYS_CLEARCURVE</b> will be
		specified.\n\n
		One or more of the following values:\n\n
		<b>SELKEYS_SELECT</b>\n The keys should be selected.\n\n
		<b>SELKEYS_DESELECT</b>\n The keys should be deselected.\n\n
		<b>SELKEYS_CLEARKEYS</b>\n All keys should be deselected.\n\n
		<b>SELKEYS_CLEARCURVE</b>\n All keys on the function curve should be deselected.\n\n
		<b>SELKEYS_FCURVE</b>\n Indicates that we are operating on the keys of a function curve, and
		not of a track.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SelectSubKeys(int subNum, TrackHitTab& sel, DWORD flags) {} 
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method is called to set the selected state of the sub-curve whose
		index is passed.
		\param subNum The index of the sub-anim to select or deselect
		\param sel TRUE to select the curve; FALSE to deselect it.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SelectSubCurve(int subNum, BOOL sel) {}
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method is called to set the selected state of the key whose index
		is passed.
		\param i The key to select or deselect.
		\param sel TRUE to select the key; FALSE to deselect it.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SelectKeyByIndex(int i, BOOL sel) {}
		/*! \remarks Returns TRUE if the key specified by the index is selected; otherwise
		FALSE.
		\param i The index of the key to test.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL IsKeySelected(int i) { return FALSE; }
		/*! \remarks This method is called to have the plug-in flag or mark a specific key
		identified by the TrackHitRecord.\n\n
		As an example, when the user goes to move a selection set of keys in
		the Track %View, a yellow marker is drawn. To move the group of keys,
		the user clicks on a single one. The system needs to track this one key
		as it is moved, and needs a way to identify it. This method is called
		so the developer can flag this key as the one that was selected. This
		is needed because the Track %View doesn't know anything about a
		specific controllers ordering of keys and thus cannot refer to it by
		index.\n\n
		The system will call <b>GetFlagKeyIndex()</b> (described below) to
		retrieve the index of the key that was flagged.
		\param hit The hit record that the controller gave the Track %View in the first place to identify
		the hit. Thus this is enough information to identify the key. See Class TrackHitRecord.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::FlagKey(TrackHitRecord hit)
		{
			int n = keys.Count();
			for ( int i = 0; i < n; i++ ) {
				keys[i].ClearKFlag(KEY_FLAGGED);
			}
			assert(hit.hit>=0&&hit.hit<(DWORD)n);
			keys[hit.hit].SetKFlag(KEY_FLAGGED);
		}
		\endcode */
		virtual void FlagKey(TrackHitRecord hit) {}
		/*! \remarks Returns the index of the key that is flagged, or -1 if no keys are
		flagged. See the method above.
		\par Default Implementation:
		<b>{return -1;}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		int InterpControl<INTERP_CONT_PARAMS>::GetFlagKeyIndex()
		{
			int n = keys.Count();
			for ( int i = 0; i < n; i++ ) {
				if (keys[i].TestKFlag(KEY_FLAGGED)) {
					return i;
				}
			}
			return -1;
		}
		\endcode  */
		virtual int GetFlagKeyIndex() { return -1; } 
		/*! \remarks Returns the number of selected keys.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int NumSelKeys() { return 0; }
		// When offset is TRUE, set the new key time to be centered between the 
		// original key and the next key
		/*! \remarks This method is called to make a copy of the selected keys.
		\param offset If TRUE, set the new key time to be centered between the original key
		and the next key. */
		virtual void CloneSelectedKeys(BOOL offset = FALSE) {}   
		/*! \remarks This method is called to add a new key at the specified time. The value
		of the key is set to the value of the previous key, or interpolated
		between keys, based on the flags passed.
		\param t The time to add the key.
		\param flags One or more of the following values:\n\n
		<b>ADDKEY_SELECT</b>\n Select the new key and deselect any other selected keys.\n\n
		<b>ADDKEY_INTERP</b>\n If TRUE then initialize the new key to the interpolated value at that
		time. If FALSE, initialize the key to the value of the previous key.
		\par Default Implementation:
		<b>{}</b> */
		virtual void AddNewKey(TimeValue t, DWORD flags) {}
		// move selected keys vertically in the function curve editor
		/*! \remarks This method is called to move selected keys vertically in the function
		curve editor. This moves the key values but does not alter the key
		times. The developer adds the delta to the selected key values, after
		converting them using the dimension <b>*dim</b> passed. See the sample
		code below for how this may be done.
		\param dim Used to scale the parameter's values into and out of
		units used in the user interface. For example, if the parameter was an
		angle, it would be shown in degrees, but stored in radians. Methods of
		this class allow the value to be converted back and forth. This is
		needed because the delta passed is in user interface units. Thus the
		selected key values need to be converted before the delta is applied.
		See Class ParamDimensBase.
		\param delta The amount to move the keys (move the values - not the times). This is
		in the units of the user interface. For example, if an angle has a
		value in the function curve editor of 100 degrees, 100 would be passed
		as the delta.
		\param flags Not currently used.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::MoveKeys(ParamDimensionBase *dim,float delta,DWORD flags)
		{
			int n = keys.Count();
			if (!n) return;
			float m = 1.0f;
			Interval valid;
			BOOL changed = FALSE;
			HoldTrack();
			for (int i = 0; i < n; i++ ) {
				for (int j=0;j<ELS;j++) {
					if (keys[i].AnyElemSelected()) {
						m = GetMultVal(keys[i].time,valid);
					}
					if (keys[i].ElemSelected(j)) {
						keys[i][j] = dim->UnConvert(dim->Convert(keys[i][j]*m)+delta)/m;
						changed = TRUE;
					}
				}
			}
			if (changed) {
		// FALSE indicates that key times didn't
		// change so sorting isn't necessary.
				keys.KeysChanged(FALSE);
				ivalid.SetEmpty();
				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		}
		\endcode  */
		virtual void MoveKeys(ParamDimensionBase *dim, float delta, DWORD flags) {}  
		/*! \remarks This method is called to scale selected keys values. This scales the
		key values but does not alter the key times. The developer scales the
		selected key values about the specified origin, after converting them
		using the dimension <b>*dim</b> passed.\n\n
		Note the following macro available for scaling about an origin:\n\n
		<b>#define ScaleAboutOrigin(val,origin,scale)</b>\n\n
		<b> ((((val)-(origin))*(scale))+(origin))</b>
		\param dim Used to scale the parameter's values into and out of
		units used in the user interface. For example, if the parameter was an
		angle, it would be shown in degrees, but stored in radians. Methods of
		this class allow the value to be converted back and forth. See
		Class ParamDimensBase.
		\param origin The origin about which the keys are scaled.
		\param scale The scale factor to apply to the key values.
		\param flags Not currently used.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::ScaleKeyValues(ParamDimensionBase *dim,float origin,float scale,DWORD flags)
		{
			int n = keys.Count();
			if (!n) return;
			BOOL changed = FALSE;
			HoldTrack();
			for (int i = 0; i < n; i++ ) {
				for (int j=0;j<ELS;j++) {
					if (keys[i].ElemSelected(j)) {
						keys[i][j] = dim->UnConvert(ScaleAboutOrigin(dim->Convert(keys[i][j]),origin,scale));
						changed = TRUE;
					}
				}
			}
			if (changed) {
				keys.KeysChanged(FALSE);
				ivalid.SetEmpty();
				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		}
		\endcode  */
		virtual void ScaleKeyValues(
			ParamDimensionBase *dim, 
			float origin, 
			float scale, 
			DWORD flags) {}
		/*! \remarks The plug-in keeps track of whether its function curve is selected or
		not. This method is called to have the plug-in select or deselect its
		function curve.
		\param sel TRUE if the curve should be selected; FALSE if it should be deselected.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SelectCurve(BOOL sel) {}
		/*! \brief Returns TRUE if the function curve is selected; otherwise returns
		FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL IsCurveSelected() { return FALSE; }		

		//! \brief Returns the selected state of the sub-curve whose index is passed.
		/*! \param subNum The index of the sub-anim
			\return TRUE if subNum is seletected. */
		virtual BOOL IsSubCurveSelected(int subNum) { return FALSE; }
		/*! \remarks This method is used to determine the commonality of the selected keys
		for display in the time/value type in fields of Track %View. It is also
		used to retrieve the value and/or time of the selected keys (if there
		is only one selected, or they are common to the selected keys). The
		flags parameter specified which values to retrieve. The return value
		indicates if nothing, or several keys were selected. It also indicates
		if the selected keys shared a common time and/or common value.
		\param t The time of the selected keys is returned here (if appropriate).
		\param val The value of the selected keys is returned here (if appropriate).
		\param flags One of the following values:\n\n
		<b>KEYCOORDS_TIMEONLY</b>\n Only the time <b>t</b> needs to be updated.\n\n
		<b>KEYCOORDS_VALUEONLY</b>\n Only the value <b>val</b> needs to be updated.
		\return  This indicates what was selected, and what these keys had in
		common. One or more of the following values should be set:\n\n
		<b>KEYS_NONESELECTED</b>\n
		This indicates that no keys are selected.\n\n
		<b>KEYS_MULTISELECTED</b>\n
		This indicates that multiple keys are selected. Both of these last two bits could be set.\n\n
		<b>KEYS_COMMONTIME</b>\n
		If the selected keys share the same time then this flag should be set.
		In this case it is appropriate to update <b>t</b> if required.\n\n
		<b>KEYS_COMMONVALUE</b>\n
		If the selected keys share the same value then this flag should be set.
		In this case it is appropriate to update <b>val</b> if required.
		\par Default Implementation:
		<b>{return KEYS_NONESELECTED;}</b> */
		virtual int GetSelKeyCoords(TimeValue &t, float &val, DWORD flags) { 
			return KEYS_NONESELECTED; 
		}
		/*! \remarks This method is called to update the time and/or value of the selected
		keys as specified by the flags. This is called if the user uses the
		time/value type in fields of Track %View.
		\param t The time to set for the selected keys (if the flags indicate this is needed).
		\param val The value to set for the selected keys (if the flags indicate this is needed).
		\param flags One of the following values:\n\n
		<b>KEYCOORDS_TIMEONLY</b>\n
		Only the time needs to be updated.\n\n
		<b>KEYCOORDS_VALUEONLY</b>\n
		Only the value needs to be updated.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetSelKeyCoords(TimeValue t, float val,DWORD flags) {}
		/*! \remarks		This method is similar to <b>SetSelKeyCoords()</b> above. In that case
		you're given a time and a value and are to update the selected keys
		with these values (based on the flags passed). In this case, you are
		instead passed time and value <b>expressions</b> (as strings). The
		ideas is that these strings are evaulated as expressions and the
		resulting values are used to updated the selected keys. For instance,
		the user could select a bunch of keys and then type in <b>n+45</b>.
		This would add 45 to all the values of the keys.\n\n
		Developers can use the 3ds Max expression parser (see
		Class Expr) to evaluate the strings.
		Debug SDK users can see <b>/MAXSDKDB/SDKSRC/CTRLTEMP.H</b> for an
		example (or see the sample code below)). If a plug-in doesn't support
		this feature it can return FALSE from this method and the old
		<b>SetSelKeyCoords()</b> method will be called. Note that the variable
		names are defined as <b>KEYCOORDS_TIMEVAR</b> and
		<b>KEYCOORDS_VALVAR</b>.
		\param dim This is used to convert the parameter value once you get it.
		\param timeExpr A string containing the time expression.
		\param valExpr A string containing the value expression.
		\param flags One of the following values:\n\n
		<b>KEYCOORDS_TIMEONLY</b>\n
		Only the time <b>t</b> needs to be updated.\n\n
		<b>KEYCOORDS_VALUEONLY</b>\n
		Only the value <b>val</b> needs to be updated.
		\return  This indicates what was selected, and what these keys had in
		common. One or more of the following values should be set:\n\n
		<b>KEYCOORDS_EXPR_UNSUPPORTED</b>\n
		Don't implement this method\n\n
		<b>KEYCOORDS_EXPR_ERROR</b>\n
		Error in expression\n\n
		<b>KEYCOORDS_EXPR_OK</b>\n
		Expression evaluated
		\par Default Implementation:
		<b>{return KEYCOORDS_EXPR_UNSUPPORTED;}</b>\n\n
		 
		\par Sample Code:
		\code
		INTERP_CONT_TEMPLATE
		int InterpControl<INTERP_CONT_PARAMS>::SetSelKeyCoordsExpr( ParamDimension *dim, MCHAR *timeExpr, MCHAR *valExpr, DWORD flags)
		{
			Expr texpr, vexpr;
			float vin, vout=0.0f, tfin, tfout=0.0f;
		
			if (timeExpr) {
				texpr.defVar(SCALAR_VAR,KEYCOORDS_TIMEVAR);
				if (texpr.load(timeExpr)!=EXPR_NORMAL) return KEYCOORDS_EXPR_ERROR;
			}
			if (valExpr) {
				vexpr.defVar(SCALAR_VAR,KEYCOORDS_VALVAR);
				if (vexpr.load(valExpr)!=EXPR_NORMAL) return KEYCOORDS_EXPR_ERROR;
			}
		
			int n = keys.Count();
			if (!n) return KEYCOORDS_EXPR_OK;
			HoldTrack();
			for (int i = 0; i < n; i++ ) {
				if (!(flags&KEYCOORDS_VALUEONLY)) {
					if (keys[i].TimeLocked()) continue;
					if (keys[i].TestKFlag(KEY_SELECTED)) {
						tfin = float(keys[i].time)/float(GetTicksPerFrame());
						texpr.eval(&tfout, 1, &tfin);
						keys[i].time = int(tfout*GetTicksPerFrame());
					}
				}
				if (!(flags&KEYCOORDS_TIMEONLY)) {
					for (int j=0;j<ELS;j++) {
						if (keys[i].ElemSelected(j)) {
							vin = dim->Convert(keys[i][j]);
							vexpr.eval(&vout, 1, &vin);
							keys[i][j] = dim->UnConvert(vout);
						}
					}
				}
			}
			keys.KeysChanged();
			keys.CheckForDups();
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			return KEYCOORDS_EXPR_OK;
		\endcode      */
		virtual int SetSelKeyCoordsExpr(
			ParamDimension *dim,
			MCHAR *timeExpr, 
			MCHAR *valExpr, 
			DWORD flags) {
				return KEYCOORDS_EXPR_UNSUPPORTED;
		}
		/*! \remarks If a plug-in has tangent handles, this method is called if the user
		adjusts them. If a plug-in doesn't have tangent handles, this method
		may be ignored. This method is called if the user selects one of the
		handles and moves the mouse. This method is passed the dx, and dy of
		the mouse motion.\n\n
		The plug-in may have any types of handles it wishes, and it is
		responsible for processing whatever needs to be done when the user
		adjusts them. The method is passed information about the screen space,
		such as the overall rectangle, and time and value scroll and zoom
		factors. See \ref timeToScreen for macros to convert in and out of
		screen space.
		\param hit This identifies the handle that was selected.
		\param dim The parameter dimension. See Class ParamDimensionBase.
		\param rcGraph This is the rectangle of the graph viewport.
		\param tzoom This is the time zoom factor.
		\param tscroll This is the time scroll factor.
		\param vzoom This is the value zoom factor.
		\param vscroll This is the value scroll factor.
		\param dx The mouse movement in screen coordinates in the x direction.
		\param dy The mouse movement in screen coordinates in the y direction.
		\param flags One of the following values:\n\n
		<b>ADJTAN_LOCK</b>\n
		Indicates the tangents are locked.\n\n
		<b>ADJTAN_BREAK</b>\n
		Indicates the tangents have been broken.
		\par Default Implementation:
		<b>{}</b> */
		virtual void AdjustTangents(
			TrackHitRecord hit,
			ParamDimensionBase *dim,
			Rect& rcGraph,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			int dx,int dy,
			DWORD flags) {};

		
		// Set-key mode related methods		

		//! \brief returns true if the sub-anim has a "Set Key" buffer present
		/*! \sa SetKeyBufferPresent
		\param subNum - The index of the SubAnim to test.
		\return TRUE if the SubAnim has a SetKey buffer present 
		\par Default Implementation:
		\code
		{
			if (SubAnim(SubNum)) return SubAnim(subNum)->SetKeyBufferPresent();
			return FALSE;
		}
		\endcode */
		CoreExport virtual BOOL SubAnimSetKeyBufferPresent(int subNum); 

		//! \brief returns true if there is a "Set Key" buffer present
		/*! This method should be implemented for a keyframeable controller to support the
		"Set Key" animation mode.  When in the "Set Key" mode, if the user modifies a keyframe controller
		the controller should create a temporary buffer to hold the new value until the user either
		commits or cancels the change.\n\n
		\note It is the plugins responsibility to ensure that the correct value (either the temp buffer or
		the permanent keyframed value) is returned from GetValue.
		For an example implementation of this method, look at
		samples/controllers/boolctrl.cpp
		\return True if a buffer is present */
		virtual BOOL SetKeyBufferPresent() { return FALSE; }

		//! \brief Commit any "Set Key" buffers on the given sub-anim
		/*! \sa CommitSetKeyBuffer
		\param subNum - The index of the SubAnim to commit.
		\par Default Implementation:
		\code
		{
			Animatable* anim = SubAnim(subNum);
			if ( anim ) anim->CommitSetKeyBuffer(t);
		}
		\endcode */
		CoreExport virtual void SubAnimCommitSetKeyBuffer(TimeValue t, int subNum); 

		//! \brief Commit any "Set Key" buffers
		/*! This function will be called whenever the user clicks the the large "Set Keys"
		button.  If this animatable is keyable, it should commit any temporary 
		"Set Key" buffers to its permanent keyframe storage.
		For an example implementation of this method, look at
		samples/controllers/boolctrl.cpp */
		virtual void CommitSetKeyBuffer(TimeValue t) {}

		//! \brief Revert any "Set Key" buffers on the given sub-anim
		/*! \sa RevertSetKeyBuffer
		\param subNum - The index of the SubAnim to revert.
		\par Default Implementation:
		\code
		{
			Animatable* anim = SubAnim(subNum);
			if ( anim ) anim->RevertSetKeyBuffer(t);
		}
		\endcode */
		CoreExport virtual void SubAnimRevertSetKeyBuffer(int subNum); 

		//! \brief Revert any "Set Key" buffers
		/*! If this animatable currently has a "Set Key" buffer, revert the value 
		and remove the buffer.
		For an example implementation of this method, look at
		samples/controllers/boolctrl.cpp */
		virtual void RevertSetKeyBuffer() {}

		/*! \remarks Returns TRUE if this animatable actually has animation; otherwise
		FALSE. This method is recursive, so for example, if you call
		<b>node-\>IsAnimated()</b> it will return TRUE if any aspect of the
		node is animated; otherwise it will return FALSE.
		\par Default Implementation:
		The default implementation returns TRUE if a child anim has animation.
		*/
		CoreExport virtual BOOL IsAnimated(); 

		// Clipboard methods:
		/*! \remarks Returns TRUE if this item can copy its data over the specified range;
		otherwise returns FALSE.
		\param iv The interval of time that would be copied.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL CanCopyTrack(Interval iv, DWORD flags) { return FALSE; }
		/*! \remarks Returns TRUE if this item can paste its data over the specified range;
		otherwise returns FALSE.
		\param cobj The clipboard object that would be pasted. The item should look at the
		SuperClassID and Class_ID of the creator of the clip object to determine if it is a suitable
		object to paste. See Class TrackClipObject.
		\param iv The interval of time that would be pasted.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL CanPasteTrack(TrackClipObject *cobj, Interval iv, DWORD flags) { return FALSE; }
		/*! \remarks This method is called to copy the item's track data over the specified
		interval.
		\param iv The interval of time over which to copy the track data. 
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\return  The item should return an instance of a class derived from
		TrackClipObject that contains the data for the item. See
		Class TrackClipObject.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual TrackClipObject *CopyTrack(Interval iv, DWORD flags) { return NULL; }
		/*! \remarks This method is called to paste the specified clip object to this track.
		This method will not be called unless <b>CanPasteTrack()</b> returned
		TRUE.
		\param cobj The data to paste.
		\param iv The interval of time to paste.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint. */
		virtual void PasteTrack(TrackClipObject *cobj, Interval iv, DWORD flags) {}

		// Plug-ins can implement copying and pasting for cases where their subanims
		// don't implement it. These aren't called on the client unless the sub-anim
		// doesn't implement the above versions.
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				If <b>CanCopyTrack()</b> returns FALSE then this method is called on
		the sub-anim (passing the sub number).\n\n
		This is used in particular for Parameter Blocks. In that case, if there
		is no controller plugged into the track, the copying and pasting of
		controllers can't be done (since there is no controller). However, this
		method allows the Parameter Block to handle it.
		\param subNum Specifies the sub-anim to check.
		\param iv The interval of time over which to copy the track data.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\return  TRUE if the specified item can copy its data over the
		specified range; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL CanCopySubTrack(int subNum, Interval iv, DWORD flags) { return FALSE; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				Returns TRUE if the specified item can paste its data over the
		specified range; otherwise returns FALSE.\n\n
		Plug-ins can implement pasting for cases where their sub-anims don't
		implement it. An example of this is the Parameter Block class. It
		implements this method to allow pasting parameters that don't have
		controllers assigned to them. These aren't called on the client unless
		the sub-anim doesn't implement <b>CanPasteTrack()</b>.
		\param subNum Specifies the sub-anim to check.
		\param cobj The data to paste.
		\param iv The interval of time to paste.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\return  TRUE if the specified item can paste its data over the
		specified range; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL CanPasteSubTrack(int subNum,TrackClipObject *cobj, Interval iv, DWORD flags) { return FALSE; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method is called to copy the specified sub anim's track data over
		the specified interval.
		\param subNum The number of the sub-anim to copy.
		\param iv The interval of time over which to copy the track data.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\return  The item should return an instance of a class derived from
		TrackClipObject that contains the data for the item. See
		Class TrackClipObject.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual TrackClipObject *CopySubTrack(int subNum, Interval iv, DWORD flags) { return NULL; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method is called to paste the specified clip object to the
		specified sub-anim track.
		\param subNum The number of the sub-anim to paste.
		\param cobj The data to paste.
		\param iv The interval of time to paste.
		\param flags One or more of the following values:\n\n
		<b>TIME_INCLEFT</b>\n Include the left endpoint.\n\n
		<b>TIME_INCRIGHT</b>\n Include the right endpoint.
		\par Default Implementation:
		<b>{}</b> */
		virtual void PasteSubTrack(int subNum, TrackClipObject *cobj, Interval iv, DWORD flags) {}

		// Drawing and hit testing tracks
		/*! \remarks Returns the vertical space occupied by the track in units of one line.
		\param lineHeight The height of a single line in pixels.
		\par Default Implementation:
		<b>{ return 1; }</b> */
		virtual int GetTrackVSpace( int lineHeight ) { return 1; }
		/*! \remarks This method is called to determine which keys lie within the
		<b>rcHit</b> rectangle. Keys that are hit are added to the <b>hits</b>
		table.
		\param hits The table of <b>TrackHitRecords</b> to update. Each key that lies
		within the hit rectangle (is hit) should be added to this table. It is
		up to the plug-in to define a scheme that allows it to identify its
		hits using the data members of Class TrackHitRecord. Also see Class Tab for methods to add to
		the table.
		\param rcHit This is the region that was selected for hit testing. This may be a
		small rectangle about the mouse pick point, or a larger rectangle if
		the user selected by window.
		\param rcTrack This is the entire rectangular region of the track.
		\param zoom The is the time zoom factor.
		\param scroll This is the time scroll factor.
		\param flags One or more of the following value:\n\n
		<b>HITTRACK_SELONLY</b>\n Selected only.\n\n
		<b>HITTRACK_UNSELONLY</b>\n Unselected only.\n\n
		<b>HITTRACK_ABORTONHIT</b>\n Abort hit testing on first hit.\n\n
		<b>HITCURVE_TESTTANGENTS</b>\n Hit test curve tangents.
		\return  One of the following values:\n\n
		<b>TRACK_DONE</b>\n 
		This indicates the track was hit tested.\n\n
		<b>TRACK_DORANGE</b>\n 
		This indicates that the system will handle hit testing to the range bar
		for the item. For example a node returns this value because it does not
		have any keys. Therefore it just lets the user hit test the range bar.
		In general, anything that is not a leaf controller will not implement
		this method and return the default. The system will then simply hit
		test the range bar.\n\n
		<b>TRACK_ASKCLIENT</b>\n 
		If a plug-in returns this value then the anim's client will be given a
		chance to paint the track in Track %View. If a client returns this
		value then the method <b>PaintSubTrack()</b> will be called.
		\par Default Implementation:
		<b>{ return TRACK_DORANGE; }</b> */
		virtual int HitTestTrack(			
			TrackHitTab& hits,
			Rect& rcHit,
			Rect& rcTrack,			
			float zoom,
			int scroll,
			DWORD flags ) { return TRACK_DORANGE; }
		/*! \remarks This method is called to display the item in the track view. If an item
		needs to draw itself in a special fashion, it implements this method to
		do so. For example, a sound plug-in may draw its waveform using this
		method. If an item does not need to draw itself, the default
		implementation may be used. This draws the range bar for the item.\n\n
		Note: When drawing something to appear in Track %View, a developer
		should not do any clipping of their own. 3ds Max will take care of all
		clipping itself.
		\param dim The dimension for the parameter of this track.
		\param hdc The handle of the device context.
		\param rcTrack The entire rectangle of the inside of the track.
		\param rcPaint This is the rectangular region that needs to be repainted - 
		the invalid region.
		\param zoom The time zoom factor.
		\param scroll The time scroll factor.
		\param flags One or more of the following values which are filters for controllers
		with more than one curve:\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n\n
		\note RGB controllers interpret X as red, Y as green, and Z as blue.
		<b>DISPLAY_ZCURVE</b>
		\return  One of the following values:\n\n
		<b>TRACK_DONE</b>\n
		Indicates the track was painted.\n\n
		<b>TRACK_DORANGE</b>\n
		Indicates the system should draw the range bars for the item.\n\n
		<b>TRACK_ASKCLIENT</b>\n
		Indicates the anim's client will be given a chance to paint the track
		in Track %View. See Animatable::PaintSubTrack() which will be
		called to do this.
		\par Default Implementation:
		<b>{ return TRACK_DORANGE; }</b> */
		virtual int PaintTrack(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags ) { return TRACK_DORANGE; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				This method will be called if PaintTrack returns
		<b>TRACK_ASKCLIENT</b>. This gives the anim's client a chance to paint
		the tracks in Track %View.
		\param subNum Specifies the sub-anim to paint.
		\param dim The dimension for the parameter of this track.
		\param hdc The handle of the device context.
		\param rcTrack The entire rectangle of the inside of the track.
		\param rcPaint This is the rectangular region that needs to be repainted - the invalid
		region.
		\param zoom The time zoom factor.
		\param scroll The time scroll factor.
		\param flags One or more of the following values which are filters for controllers
		with more than one curve:\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>
		\note RGB controllers interpret X as red, Y as green, and Z as blue.
		\return  One of the following values:\n\n
		<b>TRACK_DONE</b>\n
		Indicates the track was painted.\n\n
		<b>TRACK_DORANGE</b>\n
		Indicates the system should draw the range bars for the item.
		\par Default Implementation:
		<b>{return TRACK_DORANGE;}</b> */
		virtual int PaintSubTrack(			
			int subNum,
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags) {return TRACK_DORANGE;}

		// Drawing and hit testing function curves
		/*! \remarks This method is called to draw the function curve of the anim.
		\param dim The parameter dimension. See Class ParamDimensionBase.
		\param hdc The handle of the device context.
		\param rcGraph The entire rectangle of the inside of the graph region.
		\param rcPaint This is the rectangular region that needs to be repainted - the invalid
		region.
		\param tzoom The time zoom factor.
		\param tscroll The time scroll factor.
		\param vzoom The value zoom factor.
		\param vscroll The value scroll factor.
		\param flags One or more of the following values which are filters for controllers 
		with more than one curve:\n\n
		<b>PAINTCURVE_SHOWTANGENTS</b>\n
		Show the curve tangent handles.\n\n
		<b>PAINTCURVE_FROZEN</b>\n
		Show the curve in a frozen state.
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>\n
		<b>PAINTCURVE_GENCOLOR</b>\n
		Draw the curve in its standard color.\n\n
		The following options are passed to float controllers indicating a
		sugested color for drawing:\n\n
		<b>PAINTCURVE_XCOLOR</b>\n
		Draw the curve in red.\n\n
		<b>PAINTCURVE_YCOLOR</b>\n
		Draw the curve in green.\n\n
		<b>PAINTCURVE_ZCOLOR</b>\n
		Draw the curve in blue.
		\note RGB controllers interpret X as red, Y as green and Z as blue.
		\return  A plug-in should always return 0.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int PaintFCurves(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags ) { return 0; }
		/*! \remarks This method is called to hit test the item's function curves. It is
		called to determine which keys on the curve lie within the <b>rcHit</b>
		rectangle. Keys that are hit are added to the <b>hits</b> table.
		\param dim The parameter dimension. See Class ParamDimensionBase.
		\param hits The table of <b>TrackHitRecords</b> to update. Each key that lies
		within the hit rectangle (is hit) should be added to this table. It is
		up to the plug-in to define a scheme that allows it to identify its
		hits using the data members of Class TrackHitRecord. Also
		see Class Tab for methods to add to the table.
		\param rcHit This is the region that was selected for hit testing. This may be a
		small rectangle about the mouse pick point, or a larger rectangle if
		the user selected by window.
		\param rcGraph This is the entire rectangle of the graph region.
		\param tzoom This is the time zoom factor.
		\param tscroll This is the time scroll factor.
		\param vzoom This is the time zoom factor.
		\param vscroll This is the time scroll factor.
		\param flags One or more of the following values:\n\n
		<b>HITTRACK_SELONLY</b>\n
		Selected only.\n\n
		<b>HITTRACK_UNSELONLY</b>\n
		Unselected only.\n\n
		<b>HITTRACK_ABORTONHIT</b>\n
		Abort hit testing on first hit.\n\n
		<b>HITCURVE_TESTTANGENTS</b>\n
		Hit Test curve tangent handles.\n\n
		The following are filters for controllers with more than one curve.\n\n
		<b>DISPLAY_XCURVE</b>\n\n
		<b>DISPLAY_YCURVE</b>\n\n
		<b>DISPLAY_ZCURVE</b>
		\note RGB controllers interpret X as red, Y as green and Z as blue.
		\return One of the following values to indicate what was hit:\n\n
		<b>HITCURVE_KEY</b>\n
		Hit one or more keys.\n\n
		<b>HITCURVE_WHOLE</b>\n
		Hit the curve (anywhere).\n\n
		<b>HITCURVE_TANGENT</b>\n
		Hit a tangent handle.\n\n
		<b>HITCURVE_NONE</b>\n
		Nothing was hit.\n\n
		<b>HITCURVE_ASKCLIENT</b>\n
		Ask the client to hit test the function curve. See Animatable::HitTestSubFCurves().
		\par Default Implementation:
		<b>{ return HITCURVE_NONE; }</b> */
		virtual int HitTestFCurves(			
			ParamDimensionBase *dim,
			TrackHitTab& hits,
			Rect& rcHit,
			Rect& rcGraph,			
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags ) { return HITCURVE_NONE; }
		
		// Versions that allow clients to paint and hit test their subanims curves
		/*! \remarks This method is called to draw the specified sub-anim function curve.
		This allows the client to paint its function curve.
		\param subNum The sub-anim number to paint.
		\param dim The parameter dimension. See Class ParamDimensionBase.
		\param hdc The handle of the device context.
		\param rcGraph The entire rectangle of the inside of the graph region.
		\param rcPaint This is the rectangular region that needs to be repainted - the invalid region.
		\param tzoom The time zoom factor.
		\param tscroll The time scroll factor.
		\param vzoom The value zoom factor.
		\param vscroll The value scroll factor.
		\param flags One or more of the following values:\n\n
		<b>PAINTCURVE_SHOWTANGENTS</b>\n
		Show the curve tangent handles.\n\n
		<b>PAINTCURVE_FROZEN</b>\n
		Show the curve in a frozen state.\n\n
		The following values are filters for controllers with more than one curve.\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>\n\n
		<b>PAINTCURVE_GENCOLOR</b>\n
		Draw the curve in its standard color.\n\n
		The following options are passed to float controllers indicating a
		sugested color for drawing:\n\n
		<b>PAINTCURVE_XCOLOR</b>\n
		Draw the curve in red.\n\n
		<b>PAINTCURVE_YCOLOR</b>\n
		Draw the curve in green.\n\n
		<b>PAINTCURVE_ZCOLOR</b>\n
		Draw the curve in blue.
		\note RGB controllers interpret X as red, Y as green and Z as blue.
		\return  A plug-in should always return 0.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int PaintSubFCurves(			
			int subNum,
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags ) { return 0; }
		/*! \remarks This method is called if <b>HitTestFCurves()</b> returns
		<b>HITCURVE_ASKCLIENT</b>. It allows the client to hit test its
		sub-anim curves.
		\param subNum The sub-anim number to hit test.
		\param dim The parameter dimension. See Class ParamDimensionBase.
		\param hits The table of TrackHitRecord instances to update. Each key that lies
		within the hit rectangle (is hit) should be added to this table. It is
		up to the plug-in to define a scheme that allows it to identify its
		hits using the data members of Class TrackHitRecord. Also
		see Class Tab for methods to add to the table.
		\param rcHit This is the region that was selected for hit testing. This may be a
		small rectangle about the mouse pick point, or a larger rectangle if
		the user selected by window.
		\param rcGraph This is the entire rectangle of the graph region.
		\param tzoom This is the time zoom factor.
		\param tscroll This is the time scroll factor.
		\param vzoom This is the time zoom factor.
		\param vscroll This is the time scroll factor.
		\param flags One or more of the following values:\n\n
		<b>HITTRACK_SELONLY</b>\n
		Selected only.\n\n
		<b>HITTRACK_UNSELONLY</b>\n
		Unselected only.\n\n
		<b>HITTRACK_ABORTONHIT</b>\n
		Abort hit testing on first hit.\n\n
		<b>HITCURVE_TESTTANGENTS</b>\n
		Hit Test curve tangent handles.\n\n
		The follow are filters for controllers with more than one curve. (RGB
		controllers interpret X as red, Y as green and Z as blue.)\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>
		\return  One of the following values to indicate what was hit:\n\n
		<b>HITCURVE_KEY</b>\n
		Hit one or more keys.\n\n
		<b>HITCURVE_WHOLE</b>\n
		Hit the curve (anywhere).\n\n
		<b>HITCURVE_TANGENT</b>\n
		Hit a tangent handle.\n\n
		<b>HITCURVE_NONE</b>\n
		Nothing was hit.
		\par Default Implementation:
		<b>{ return HITCURVE_NONE; }</b> */
		virtual int HitTestSubFCurves(
			int subNum,
			ParamDimensionBase *dim,
			TrackHitTab& hits,
			Rect& rcHit,
			Rect& rcGraph,			
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags ) { return HITCURVE_NONE; }

		/*! \remarks This method is called for the plug-in to put up a modal dialog and let the user
		edit the tracks parameters for the selected keys. This function should not
		return until the user has completed editing at which time any windows that were
		created should be destroyed. Unlike <b>BeginEditParams()</b> and
		<b>EndEditParams()</b> this interface is modal.
		\param t This time represents the horizontal position of where the user right clicked to
		bring up the modal edit track parameters dialog. See the flags below for when
		this parameter is valid.
		\param dim The parameter dimension. See Class ParamDimensionBase</a>.
		\param pname The name of the parameter as given by the client.
		\param hParent This is the parent window that should be used to create any dialogs.
		\param ip An interface pointer available for calling functions in 3ds Max.
		\param flags One or more of the following values:\n\n
		<b>EDITTRACK_FCURVE</b>\n
		The user is in the function curve editor.\n\n
		<b>EDITTRACK_TRACK</b>\n
		The user is in one of the track views.\n\n
		<b>EDITTRACK_SCENE</b>\n
		The user is editing a path in the scene.\n\n
		<b>EDITTRACK_BUTTON</b>\n
		The user invoked by choosing the properties button. In this case the time
		parameter is NOT valid.\n\n
		<b>EDITTRACK_MOUSE</b>\n
		The user invoked by right clicking with the mouse. In this case the time
		parameter is valid.
		\par Default Implementation:
		<b>{}</b> */
		virtual void EditTrackParams(
			TimeValue t,	// The horizontal position of where the user right clicked.
			ParamDimensionBase *dim,
			MCHAR *pname, // The name of the parameter as given by the client
			HWND hParent,
			IObjParam *ip,
			DWORD flags) {}

		// Returns a value indicating how track parameters are
		// are invoked. See description above by
		// TRACKPARAMS_NONE, TRACKPARAMS_KEY, TRACKPARAMS_WHOLE
		/*! \remarks This method returns a value that indicates how the track parameter
		editing is invoked.
		\return  One of the following values:\n\n
		<b>TRACKPARAMS_NONE</b>\n\n
		Has no track parameters. If this is returned then
		<b>EditTrackParams()</b> will not be called.\n\n
		<b>TRACKPARAMS_KEY</b>\n\n
		Entered by right clicking on a selected key. This should be used if the
		dialog provides parameters for the entire controller (for example as
		the Noise controller's dialog does).\n\n
		<b>TRACKPARAMS_WHOLE</b>\n\n
		Entered by right clicking anywhere in the track. This should be used if
		the dialog will represent the selection of keys (as a key info type
		dialog does).
		\par Default Implementation:
		<b>{return TRACKPARAMS_NONE;}</b> */
		virtual int TrackParamsType() {return TRACKPARAMS_NONE;}

		// Calculate the largest and smallest values.
		// If this is processed, return non-zero.
		//! \brief This method is called to calculate the largest and smallest values of the anim.
		/*! The values max and min should be initialized before calling this function.  A plugin
		implementing this function should not reset the values passed - in this way if max is passed
		with a larger value than the curve extents calculated for the current anim, the value of 
		max will be unchanged.
		\param dim - The dimension of the anim.
		\param[out] min - The smallest value. These are in the units given by the dimension. For
		example, if it was an angle parameter that was displayed in degrees,
		the units returned through <b>min</b> should be in degrees as well.
		A class should implement this method to not reset this value
		\param[out] max - The largest value. These are in the units given by the dimension. For
		example, if it was an angle parameter that was displayed in degrees,
		the units returned through <b>max</b> should be in degrees as well.
		\param flags - One or more of the following values which are filters for 
		controllers with more than one curve:\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>
		\note RGB controllers interpret X as red, Y as green and Z as blue. 
		\return  If this method is processed, return nonzero; otherwise zero.*/
		virtual int GetFCurveExtents(
			ParamDimensionBase *dim,
			float &min, 
			float &max, 
			DWORD flags) { return 0; }

		//! \brief This method is called to calculate the largest and smallest values of the specified subanim.
		/*! The values max and min should be initialized before calling this function.  A plugin
		implementing this function should not reset the values passed - in this way if max is passed
		with a larger value than the curve extents calculated for the current anim, the value of 
		max will be unchanged.
		\param subNum - The index of the SubAnim to query for curve extents
		\param dim - The dimension of the anim.
		\param[out] min - The smallest value. These are in the units given by the dimension. For
		example, if it was an angle parameter that was displayed in degrees,
		the units returned through <b>min</b> should be in degrees as well.
		A class should implement this method to not reset this value
		\param[out] max - The largest value. These are in the units given by the dimension. For
		example, if it was an angle parameter that was displayed in degrees,
		the units returned through <b>max</b> should be in degrees as well.
		\param flags - One or more of the following values which are filters for 
		controllers with more than one curve:\n\n
		<b>DISPLAY_XCURVE</b>\n
		<b>DISPLAY_YCURVE</b>\n
		<b>DISPLAY_ZCURVE</b>
		\note RGB controllers interpret X as red, Y as green and Z as blue. 
		\return  If this method is processed, return nonzero; otherwise zero.*/
		virtual int GetSubFCurveExtents(
			int subNum,
			ParamDimensionBase *dim,
			float &min, 
			float &max, 
			DWORD flags) { return 0; }

		// Describes the type of dimension of the ith sub-anim
		/*! \remarks Returns the type of dimension of the 'i-th' sub-anim. A dimension
		describes the type and order of magnitude of a sub-anim.
		\param i Specifies the sub-anim (parameter) to return the dimension of.
		\return  The dimension of the 'i-th' sub-anim (parameter).
		\par Default Implementation:
		<b>{return defaultDim;}</b> */
		virtual ParamDimension* GetParamDimension(int i) { return defaultDim; }

		// This is not used anymore.
		//! \brief This function is obsolete
		virtual LRESULT CALLBACK TrackViewWinProc( 
			HWND hwnd,  
			UINT message, 
		  WPARAM wParam, 
			LPARAM lParam ) { return 0;}

		// Called when the user clicks on the icon of a subAnim in the track view.
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		When a user is in Track %View in Edit Keys mode and clicks on the green
		triangle of a controller then this method will be called on the client
		with the appropriate sub number that corresponds to it. For instance,
		the Editable Mesh object implements this to allow the user to select
		vertices that are animated from the Track %View.
		\param subNum The index of the sub-anim that was clicked on.
		\return  TRUE if implemented; otherwise FALSE. (Track %View will call
		<b>RedrawViewports()</b> if something returns TRUE from this method).
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL SelectSubAnim(int subNum) { return FALSE; }

		/*! \remarks Implemented by the System.\n\n
		This method adds the specified note track.
		\param note The note track to add. The Note Tracks provided by 3ds Max are derived
		from Class DefNoteTrack (which is derived from <b>NoteTrack</b>). See that class for the methods and
		data members used to access Note Tracks. */
		CoreExport void AddNoteTrack(NoteTrack *note);
		/*! \remarks Implemented by the System.\n\n
		This method deletes the specified note track.
		\param note The note track to delete. The Note Tracks provided by 3ds Max are derived from
		Class DefNoteTrack (which is derived from NoteTrack). 
		See that class for the methods and data members used to access Note Tracks. 
		\param delNote If delNote is FALSE the note track will be removed from the anim but not deleted.*/
		CoreExport void DeleteNoteTrack(NoteTrack *note, BOOL delNote = TRUE); 
		/*! \remarks Implemented by the System.\n\n
		This method returns TRUE if the track has note tracks; otherwise FALSE.
		*/
		CoreExport BOOL HasNoteTracks();
		/*! \remarks Implemented by the System.\n\n
		This method returns the number of note tracks. */
		CoreExport int NumNoteTracks();
		/*! \remarks Implemented by the System.\n\n
		This method retrieves the 'i-th' note track.
		\param i Specifies the note track to retrieve.
		\return  A pointer to a Note Track. The Note Tracks provided by 3ds Max
		are derived from Class DefNoteTrack (which is derived from
		<b>NoteTrack</b>). See that
		class for the methods and data members used to access Note Tracks. */
		CoreExport NoteTrack *GetNoteTrack(int i);

		// Enumerate auxiliary files -- see ref.h 
		// this implementation calls EnumAuxFiles on the CustomAttributeContainer. If
		// you override this method, call this method also.
		// This method sets A_WORK1
		/*! \remarks This method is used to enumerate any 'auxiliary' files maintained by
		the item and record the filename with the callback. Entities which
		actually need to load auxiliary files (for instance texture maps) must
		implement this method, possibly calling
		<b>ReferenceMaker::EnumAuxFiles()</b> also. The <b>ReferenceMaker</b>
		implementation simply calls itself on all its references (see
		below).\n\n
		Class <b>Interface</b> has a method that may be used to call this on
		the entire system. This includes the renderer, materials editor,
		atmospheric effects, background, video post, lights, etc. -- everything
		that may have auxiliary files associated with it. After specifying the
		appropriate flags a list of filenames is created that matched the
		enumeration conditions as specified by the flags. This is used for
		instance by the Archive function in 3ds Max to grab a list of bitmap
		files used by the system.
		\param assetEnum The callback object that may be used to record the asset. See
		Class AssetEnumCallback. 
		\param flags See \ref EnumAuxFiles
		\par Sample Code:
		This is the default implementation provided by <b>ReferenceMaker</b>.\n\n
		\code
		void ReferenceMaker::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags)
		{
			   if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return;

			   // no local aux files...
		             
			   // enumerate my children. Derived classes don't need to do this since they should
			   // call EnumAuxFiles on their base class, and so will eventually end up here
			   if (flags&FILE_ENUM_DONT_RECURSE) return;
			   for (int i=0; i<NumRefs(); i++) {
					  ReferenceMaker *srm = GetReference(i);
					  if (srm) {
							 if ((flags&FILE_ENUM_CHECK_AWORK1)&&srm->TestAFlag(A_WORK1)) continue;
							 srm->EnumAuxFiles(nameEnum,flags);
							 srm->SetAFlag(A_WORK1);
					  }
			   }

			   // pick up base class...
			   Animatable::EnumAuxFiles(nameEnum,flags); 
		}

		\endcode  */
		CoreExport virtual void EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD flags = FILE_ENUM_ALL);

		// Free all bitmaps in the Animatable: don't recurse
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		This method frees all bitmaps in this <b>Animatable</b> but doesn't
		recurse. This is used for freeing all the scene bitmaps after a render.
		\par Default Implementation:
		<b>{}</b> */
		virtual void FreeAllBitmaps() {}

		// A master controller should implement this method to give the 
		// MAX a list of nodes that are part of the system. 
		/*! \remarks The master controller of a system plug-in should implement this method
		to give 3ds Max a list of nodes that are part of the system. The master
		controller should fill in the given table with the INode pointers of
		the nodes that are part of the system. This will ensure that operations
		like cloning and deleting affect the whole system.\n\n
		Said another way, <b>GetSystemNodes()</b> should be implemented for the
		master controller of a system, and should return a list of pointers to
		all nodes that are part of the system. <b>GetInterface()</b> should be
		implemented for the slave TM controllers of the system and return a
		pointer to the master controller.\n\n
		3ds Max will use <b>GetInterface()</b> in the TM controller of each
		selected node to retrieve the master controller and then call
		<b>GetSystemNodes()</b> on the master controller to get the list of
		nodes.
		\param nodes The table of nodes that are part of the system.
		\param Context This parameter is available in release 4.0 and later only.
		This parameter can be used to specify the context under which the
		"system nodes" are used. These are; <b>kSNCClone</b>,
		<b>kSNCDelete</b>, <b>kSNCFileMerge</b>, and <b>kSNCFileSave</b>.
		\par Default Implementation:
		<b>{}</b> */
		virtual void GetSystemNodes(INodeTab &nodes, SysNodeContext Context) {}

		//! \brief returns true if the animatable has sub-classed off the given class
		/*! If an object is a sub-class of a particular class, it will have a
		different <b>ClassID()</b> because it is a different class. This method
		allows an object to indicate that it is a sub-class of a particular
		class and therefore can be treated as one. For example, a class could
		be derived from <b>TriObject</b>. This derived class would have a
		different <b>ClassID()</b> then the <b>TriObject's</b> class ID however
		it still can be treated (cast) as a <b>TriObject</b> because it is
		derived from <b>TriObject</b>. Note the default implelementation: a
		class is considered to also be a subclass of itself.
		\param classID The Class_ID of the item that this class is a sub-class of.
		\return  TRUE if this class is a sub-class of <b>classID</b>; otherwise
		FALSE.
		\par Default Implementation:
		<b>{return classID==ClassID();}</b> */
		virtual BOOL IsSubClassOf(Class_ID classID) {
			return (classID == ClassID());
		}

		//! \brief Tells whether it is a ReferenceMaker
		/*! This function differentiates things sub classed from Animatable 
		from subclasses of ReferenceMaker. The implementation of this method 
		(in Animatable) returns FALSE and its implementation in ReferenceMaker 
		returns TRUE.
		\return Default of FALSE.
		*/
		virtual BOOL IsRefMaker() { return FALSE; }

		// Access app data chunks
		//! \brief Adds application/plugin specific (custom) data to an Animatable. 
		/*! This method is used to add an AppDataChunk to this Animatable. The chunk is 
		identified using the Class_ID, and SuperClassID of the owner, and an ID for sub-chunks.
		\note Developers who want to add appdata to the scene should see the method
		ReferenceTarget *Interface::GetScenePointer().
		\param cid - The Class_ID of the owner of the chunk.
		\param sid - The SuperClassID of the owner of the chunk.
		\param sbid - An extra ID that lets the owner identify its sub-chunks.
		\param len - The length of the data in bytes.
		\param data - Pointer to the actual data. The data should be allocated 
		on the heap by client code using MAX_malloc(). This will allow the system to 
		free it safely (using MAX_free()). MAX_malloc() and MAX_free() are memory 
		management routines implemented by the system. 
		\note Not allocating on the heap the data passed to this method may 
		lead to unexpected behaviour of the application. 
		\note Client code does not need to free the data that has been passed 
		to this method. The system will free it when the Animatable is deleted or when
		client code explicitely removes the custom data chunk from the Animatable by
		calling Animatable::RemoveAppDataChunk.
		*/
		CoreExport void AddAppDataChunk(Class_ID cid, SClass_ID sid, DWORD sbid, DWORD len, void *data);
		//! \brief  Retrieves the application/plugin specific (custom) data stored with an Animatable.
		/*! This method is used to retrieve a pointer to an AppDataChunk. The chunk is 
		identified using the Class_ID, SuperClassID and sub-chunk ID of the owner.
		\param cid - The Class_ID of the owner of the chunk.
		\param sid - The SuperClassID of the owner of the chunk.
		\param sbid - An extra ID that lets the owner identify its sub-chunks.
		\return A pointer to the previously stored AppDataChunk, or NULL if it could 
		not be found. 
		\see class AppDataChunk.
		*/
		CoreExport AppDataChunk *GetAppDataChunk(Class_ID cid, SClass_ID sid, DWORD sbid);
		//! \brief Deletes the application/plugin specific (custom) data stored with an Animatable.
		/*! This method is used to delete an AppDataChunk. The chunk is identified using the 
		Class_ID, SuperClassID and sub-chunk ID of the owner. 
		\param cid - The Class_ID of the owner of the chunk.
		\param sid - The SuperClassID of the owner of the chunk.
		\param sbid - An extra ID that lets the owner identify its sub-chunks.
		\return TRUE if the data was deleted, FALSE if it could not be found.
		*/
		CoreExport BOOL RemoveAppDataChunk(Class_ID cid, SClass_ID sid, DWORD sbid);		
		//! \brief Deletes all application/plugin specific (custom) data stored with an Animatable.
		/*! Calling this method will remove all the AppData associated with this Animatable. 
		\note Plugins that call this method will erase all appdata chunks, not just their own. 
		Therefore, it is usually more appropriate to call RemoveAppDataChunk() to remove 
		custom application data associated with a specific Class_ID.
		*/
		CoreExport void ClearAllAppData();
		
		/*! \remarks This method is available in release 2.0 and later only
		(previously in Class Control in 1.x).\n\n
				This method is called on whatever controller the user is modifying with
		the mouse -- when the mouse button is released. For example when the
		user selects a node in the viewports, then drags, then releases the
		mouse button, this method is called. This method will also be called
		when the user clicks on a key in the track view and lets up. If a
		controller performs extensive calculation in its evaluation this method
		is handy. The controller could perhaps perform a simplified calculation
		during interactive adjustment of a node. Then when the user releases
		the mouse button this method is called and the extensive calculations
		are performed.\n\n
		The default implementation of this method is recursive so it gets
		called on all sub-anims affected by a range bar operation.
		\param t The time the mouse was released. */
		CoreExport virtual void MouseCycleCompleted(TimeValue t);
		/*! \remarks				This method is called on whatever controller the user is modifying with
		the mouse -- when the mouse button is pressed.\n\n
		The default implementation of this method is recursive so it gets
		called on all sub-anims affected by a range bar operation.
		\param t The time the mouse was first pressed. */
		CoreExport virtual void MouseCycleStarted(TimeValue t);

		// JBW: direct ParamBlock2 access added
		// return number of ParamBlocks in this instance
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
				This method returns the number of ParamBlock2s in this instance.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int	NumParamBlocks() { return 0; }			
		// return i'th ParamBlock
		/*! \remarks				This method return 'i-th' ParamBlock2 in this instance (or NULL if not
		available).
		\param i The zero based index of the ParamBlock2 to return.
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		virtual IParamBlock2* GetParamBlock(int i) { return NULL; } 
		// return ParamBlock given ID
		/*! \remarks				This method returns a pointer to the ParamBlock2 as specified by the ID
		passed (or NULL if not available).
		\param id The BlockID of the ParamBlock2 instance.
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		virtual IParamBlock2* GetParamBlockByID(short id) { return NULL; } 


		// Save and load functions for schematic view data.
		// For classes derived from ReferenceMaker, there is
		// no need to call these.  However, IF you have stuff
		// derived from Animatable AND it appears in the
		// schematic view AND you want to save schematic view
		// properties for the object (node position, selection
		// state, etc.) THEN you have to call these guys in
		// your Save and Load functions... 
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Implemented by the System.\n\n
		This is the save method for schematic view data. For classes derived
		from ReferenceMaker, there is no need to call these. However, <b>if</b>
		you have a class derived from Animatable <b>and</b> it appears in the
		schematic view <b>and</b> you want to save schematic view properties
		for the object (node position, selection state, etc.) <b>then</b> you
		have to call this in your <b>Save()</b> method.
		\param isave An interface for saving data. See Class ISave.
		\param id The Chunk id (choosen by the developer).
		\return  Returns true if saved okay; otherwise false. */
		CoreExport bool SvSaveData(ISave* isave, USHORT id);
		/*! \remarks		Implemented by the System.\n\n
		This is the load method for schematic view data. For classes derived
		from ReferenceMaker, there is no need to call these. However, <b>if</b>
		you have a class derived from Animatable <b>and</b> it appears in the
		schematic view <b>and</b> you want to load schematic view properties
		for the object (node position, selection state, etc.) <b>then</b> you
		have to call this in your <b>Load()</b> method.
		\param iLoad An interface for loading data. See Class ILoad.
		\return  Returns true if loaded okay; otherwise false. */
		CoreExport bool SvLoadData(ILoad* iLoad);

		// Used internally by the schematic view.  There should
		// be no reason for plug-ins to ever call these...
		/*! \remarks		This method is for internal use only. */
		CoreExport DWORD SvGetRefIndex();
		/*! \remarks		This method is for internal use only. */
		CoreExport void SvSetRefIndex(DWORD i);
		/*! \remarks		This method is for internal use only.
		\par Operators:
		*/
		CoreExport bool SvDeleteRefIndex();

		// Traverses the graph of objects in the MAX scene,
		// adding desired objects to the schematic view.
		// Developers can specialize this behaviour by overriding
		// this method and adding whatever objects are interesting
		// to the schematic view...
		// Objects are added to the schematic view by calling
		// IGraphObjectManager::AddAnimatable(...)
		// Reference lines are added to the schematic view by
		// calling IGraphObjectManager::AddReference(...)
		// Implementers of SvTraverseAnimGraph(...) should
		// call SvTraverseAnimGraph(...) recursively to
		// process other objects in the scene.
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method traverses the graph of objects in the 3ds Max scene, adding
		desired objects to the schematic view. Developers can specialize this
		behaviour by overriding this method and adding whatever objects are
		interesting to the schematic view. Objects are added to the schematic
		view by calling <b>IGraphObjectManager::AddAnimatable(...)</b>.
		Reference lines are added to the schematic view by calling
		<b>IGraphObjectManager::AddReference(...)</b>. Implementers of this
		method should call it recursively to process other objects in the
		scene.\n\n
		See Class IGraphObjectManager.
		\param gom Points to the schematic view window manager.
		\param owner The owning animatable.
		\param id This is usually the sub-anim number (but can actually be any value the
		developer chooses).
		\param flags See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_sch_view_addanimflags.html">List of
		Schematic %View AddAnimatable Flags</a>.
		\return  A SvGraphNodeReference object. */
		CoreExport virtual SvGraphNodeReference SvTraverseAnimGraph(
			IGraphObjectManager *gom, 
			Animatable *owner, 
			int id, 
			DWORD flags);

		// A default graph traversal function which can be
		// called from SvTraverseAnimGraph(...) to handle
		// graph traversal in simple cases.  Follows sub-anim
		// and child references...
		/*! \remarks		This method is a default graph traversal function which can be called from
		<b>SvTraverseAnimGraph(...)</b> above to handle graph traversal in simple
		cases. This traversal follows the sub-anim and child references. See the code
		below.
		\param gom Points to the schematic view window manager.
		\param owner The owning animatable.
		\param id This is usually the sub-anim number (but can actually be any value the
		developer chooses).
		\param flags See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_sch_view_addanimflags.html">List of Schematic %View
		AddAnimatable Flags</a>.
		\return  A SvGraphNodeReference object.
		\par Default Implementation:
		\code
		// A default graph traversal function which can be
		// called from SvTraverseAnimGraph(...) to handle
		// graph traversal in simple cases. Follows sub-anim
		// and child references...
		SvGraphNodeReference Animatable::SvStdTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
		{
			int i;
			SvGraphNodeReference nodeRef;
			SvGraphNodeReference childNodeRef;
			gom->PushLevel(this);
			nodeRef = gom->AddAnimatable(this, owner, id, flags);
			if (nodeRef.stat == SVT_PROCEED) {
				for (i = 0; i < NumSubs(); i++) {
					if (SubAnim(i)) {
						childNodeRef = SubAnim(i)->SvTraverseAnimGraph(gom, this, i, flags);
						if (childNodeRef.stat != SVT_DO_NOT_PROCEED)
							gom->AddReference(nodeRef.gNode, childNodeRef.gNode, REFTYPE_SUBANIM);
					}
				}
			}
			gom->PopLevel();
			return nodeRef;
		}
		\endcode */
		CoreExport SvGraphNodeReference SvStdTraverseAnimGraph(
			IGraphObjectManager *gom, 
			Animatable *owner, 
			int id, 
			DWORD flags);

		// Animatable returns true if it can be the initiator of
		// a link operation in the schematic view...
		/*! \remarks		Returns true if this animatable can be the initiator of a link
		operation in the schematic view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view. */
		CoreExport virtual bool SvCanInitiateLink(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Animatable returns true if it can be the receiver
		// (parent) of a link operation in the schematic view...
		/*! \remarks		Returns true if this animatable can be the receiver (parent) of a link
		operation in the schematic view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\param gNodeChild Points to the child node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvCanConcludeLink(
			IGraphObjectManager *gom, 
			IGraphNode *gNode, 
			IGraphNode *gNodeChild);

		// Returns the name of the object as it appears in the
		// schematic view...
		/*! \remarks		Returns the name of the object as it appears in the schematic view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\param isBeingEdited TRUE if the item is being edited; FALSE if not.
		\par Default Implementation:
		\code
		{
			Animatable *owner;
			int subNum;
			MSTR name;
			owner = gNode->GetOwner();
			subNum = gNode->GetID();
			name = owner->SubAnimName(subNum);
			return name;
		}
		\endcode  */
		CoreExport virtual MSTR SvGetName(
			IGraphObjectManager *gom, 
			IGraphNode *gNode, 
			bool isBeingEdited);

		// Return true to permit the object's name to be
		// edited in the schematic view...
		/*! \remarks		Return true to permit the object's name to be edited in the schematic
		view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvCanSetName(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Called when the user changes the name of the object
		// in the schematic view...
		/*! \remarks		Called when the user changes the name of the object in the schematic
		view.
		\param gom< Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\param name The new name to set.
		\return  TRUE if the name was changed; FALSE if not. */
		CoreExport virtual bool SvSetName(
			IGraphObjectManager *gom, 
			IGraphNode *gNode, 
			MSTR &name);

		// Return true if this object can be removed in the schematic view...
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Return true if this object can be removed in the schematic view; false if not.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvCanRemoveThis(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		//! \brief Called when the user deletes this object in the schematic view...
		/*! \param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\return  true if deleted; false if not. */
		CoreExport virtual bool SvRemoveThis(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		//! \brief Returns true if the object is selected in its schematic view
		CoreExport virtual bool SvIsSelected(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Returns true if the object is to be highlighted in
		// the schematic view...
		/*! \remarks		Returns true if the object is to be highlighted in the schematic view;
		otherwise false.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvIsHighlighted(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Returns the highlight color for this node.  The
		// highlight color is used to outline nodes in the
		// schematic view when SvIsHighlighted(...) returns true...
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the highlight color for this node. The highlight color is used
		to outline nodes in the schematic view when <b>SvIsHighlighted(...)</b>
		returns true.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\return  See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF-DWORD format</a>.
		\par Default Implementation:
		<b>{ return gom-\>SvGetUIColor(SV_UICLR_PLUGIN_HIGHLIGHT); }</b> */
		CoreExport virtual COLORREF SvHighlightColor(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Returns a color which is used to paint the triangular
		// color swatch that appears in the upper-right hand
		// corner of the node in the schematic view.  Can
		// return SV_NO_SWATCH to indicate that no swatch is to be drawn...
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a color which is used to paint the triangular color swatch that
		appears in the upper-right hand corner of the node in the schematic
		view. One can return <b>SV_NO_SWATCH</b> to indicate that no swatch is
		to be drawn.
		\param gom Points to the schematic view window manager. 
		\param gNode Points to this node in the schematic view.
		\return  See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF-DWORD format</a>.
		\par Default Implementation:
		<b>{ return SV_NO_SWATCH; }</b> */
		CoreExport virtual COLORREF SvGetSwatchColor(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		// Returns true if this object is inactive.  The schematic
		// view draws inactive nodes in a grayed-out state.
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns true if this object is inactive; false is active. The schematic
		view draws inactive nodes in a grayed-out state.
		\param gom Points to the schematic view window manager.
		\param gNode Points to this node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvIsInactive(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks		This method is called to link this object to the <b>gNodeChild</b> passed.
		\param gom Points to the schematic view window manager.
		\param gNodeThis Points to this node in the schematic view.
		\param gNodeChild Points to the child node in the schematic view.
		\return  true if linked; false if not.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvLinkChild(
			IGraphObjectManager *gom, 
			IGraphNode *gNodeThis, 
			IGraphNode *gNodeChild);

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is called when this node is double-clicked in the schematic
		view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\return  true is handled; false if not interested in the event.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvHandleDoubleClick(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks This method is called before a multiple select/deselect
		operation in the schematic view. Returns a callback used to perform the
		(de)selection. May return NULL if this object cannot be selected in
		some principle editor outside the schematic view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\return  A pointer to the callback object. See
		Class MultiSelectCallback.
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		CoreExport virtual MultiSelectCallback* SvGetMultiSelectCallback(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);		

		/*! \remarks Returns true if this object can be selected in some
		editor (viewport, material editor, plug-in specific editor, etc.).
		Selection is actually accomplished by via the
		<b>SvGetMultiSelectCallback(...)</b> mechanism described above.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvCanSelect(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks		This method is reserved for future use.
		\par Default Implementation:
		<b>{ return false; }</b> */
		CoreExport virtual bool SvEditProperties(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks		Returns a string to be displayed in the tip window for this object in
		the schematic view.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\par Default Implementation:
		<b>{ return SvGetName(gom, gNode, false); }</b> */
		CoreExport virtual MSTR SvGetTip(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks Returns a string to be displayed in the tip window in the
		schematic view for a reference from "gNodeMaker" to this.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\param gNodeMaker Points to the Maker node in the schematic view.
		\par Sample Code:
		\code
		{
			return gNodeMaker->GetAnim()->SvGetName(gom, gNodeMaker, false) + " -> " + SvGetName(gom, gNode, false);
		}
		\endcode  */
		CoreExport virtual MSTR SvGetRefTip(
			IGraphObjectManager *gom, 
			IGraphNode *gNode, 
			IGraphNode *gNodeMaker);

		/*! \remarks Returns true is this object can respond to the <b>SvDetach(...)</b>
		method; false if not.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\par Default Implementation:
		<b>{ return false;}</b> */
		CoreExport virtual bool SvCanDetach(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \remarks		This method is called to detach this object from its owner.
		\param gom Points to the schematic view window manager.
		\param gNode Points to the node in the schematic view.
		\return  Returns true if detached; otherwise false.
		\par Default Implementation:
		<b>{ return false;}</b> */
		CoreExport virtual bool SvDetach(
			IGraphObjectManager *gom, 
			IGraphNode *gNode);

		/*! \brief Returns a string to be displayed in the tip window 
		in the schematic view for a relationship from "gNodeMaker" to "gNodeTarget"...
		\param gom Points to the schematic view window manager.
		\param gNodeTarget the IGraphNode that represents the target of the relationship.
		\param id The value passed as the IGraphObjectManager::AddRelationship ID parameter.  Usually the subanim number.
		\param gNodeMaker The IGraphNode that represents the maker of the relationship (the animatable being called)
		\return A string to be displayed in the tooltip
		\par Sample Code
		\code
		{
			return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarget->GetAnim()->SvGetName(gom, gNodeTarget, false);
		}
		\endcode*/
		CoreExport virtual MSTR SvGetRelTip(
			IGraphObjectManager *gom, 
			IGraphNode *gNodeTarget, 
			int id, 
			IGraphNode *gNodeMaker);

		//! \brief Returns true if this object can respond to the SvDetachRel(...) method...
		/*! \param gom Points to the schematic view window manager.
		\param gNodeTarget the IGraphNode that represents the target of the relationship.
		\param id The value passed as the IGraphObjectManager::AddRelationship ID parameter.  Usually the subanim number.
		\param gNodeMaker The IGraphNode that represents the maker of the relationship (the animatable being called)
		\return true if this animatable can process a call to SvDetachRel
		\par Default Implementation:
		\code
		{
			return false;
		}
		\endcode*/
		CoreExport virtual bool SvCanDetachRel(
			IGraphObjectManager *gom, 
			IGraphNode *gNodeTarget, 
			int id, 
			IGraphNode *gNodeMaker);

		//! \brief  Detach this relationship.
		/*! gNodeMaker is called to detach gNodeTarget
		\param gom Points to the schematic view window manager.
		\param gNodeTarget the IGraphNode that represents the target of the relationship.
		\param id The value passed as the IGraphObjectManager::AddRelationship ID parameter.  Usually the subanim number.
		\param gNodeMaker The IGraphNode that represents the maker of the relationship (the animatable being called)
		\return true if the relationship was detached
		\par Default Implementation:
		\code
		{
			return false;
		}
		\endcode*/
		CoreExport virtual bool SvDetachRel(
			IGraphObjectManager *gom, 
			IGraphNode *gNodeTarget, 
			int id, 
			IGraphNode *gNodeMaker);

		//! \brief  Called when this relationship is double-clicked in the schematic view...
		/*! \param gom Points to the schematic view window manager.
		\param gNodeTarget the IGraphNode that represents the target of the relationship.
		\param id The value passed as the IGraphObjectManager::AddRelationship ID parameter.  Usually the subanim number.
		\param gNodeMaker The IGraphNode that represents the maker of the relationship (the animatable being called)
		\return true if the double click was handled
		\par Default Implementation:
		\code
		{
			return false;
		}
		\endcode*/
		CoreExport virtual bool SvHandleRelDoubleClick(
			IGraphObjectManager *gom, 
			IGraphNode *gNodeTarget, 
			int id, 
			IGraphNode *gNodeMaker);

		/*! \brief		This method returns a pointer to the custom attributes container interface class.
		See Class ICustAttribContainer for more information.
		*/
		CoreExport ICustAttribContainer *GetCustAttribContainer();						
		/*! \brief		This method allocates space for a custom attributes container. */
		CoreExport void AllocCustAttribContainer();
		/*! \brief This method deletes space used by a custom attributes container. */
		CoreExport void DeleteCustAttribContainer();   // JBW 6.26.00


		//! \brief  Returns true if the passed description is being used
		/*! When plugins use a dynamic Paramblock2 system, and build a ParamBlockDesc2 on 
		demand, the list maintained by the ClassDesc may not reflect the active 
		configuration in the plugin.  This method lets the system filter the list 
		maintained by asking the plugin  (such as the DxMaterial) whether the current ParamBlockDesc2 is 
		currently used by the plugin. Anybody querying the ClassDesc for the ParamBlockDesc2 needs 
		to make sure that the descriptor is currently being used by the plugin.\n
		This mostly goes for maxscript.
		\param desc The description to test
		\return true if the plugin is currently using a parameter block built from desc.
		\par Default Implementation:
		\code
		{
			return false;
		}  
		\endcode*/
		virtual bool IsParamBlockDesc2Used(ParamBlockDesc2 * desc) { return true; }
};
#pragma warning(pop)
