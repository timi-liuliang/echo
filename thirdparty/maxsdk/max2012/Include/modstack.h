/**********************************************************************
 *<
	FILE: modstack.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig

	HISTORY: created January 20, 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "coreexp.h"
#include "maxtypes.h"
#include "object.h"

// These are the class IDs for object space derived objects and
// world space derived objects
extern CoreExport Class_ID derivObjClassID;
extern CoreExport Class_ID WSMDerivObjClassID;


/*! \sa  Class Object, Class ModContext, Class Modifier, Class INode, <a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry Pipeline System</a>.\n\n
\par Description:
This class provides an interface into derived objects. Methods of this class
are provided so developers can access the modifier stack, add and delete
modifiers, etc. All methods of this class are implemented by the system.\n\n
To use this interface you must #include the following file:\n\n
<b>#include "modstack.h"</b>\n\n
To get an <b>IDerivedObject</b> pointer from the pipeline of a node in the
scene first retrieve the object reference using <b>INode::GetOjbectRef()</b>.
Given this <b>Object</b> pointer check its SuperClassID to see if it is
<b>GEN_DERIVOB_CLASS_ID</b>. If it is, you can cast it to an
<b>IDerivedObject</b>.\n\n
Note: The following functions are not part of class <b>IDerivedObject</b> but
are available for use in conjunction with its methods. */
class IDerivedObject : public Object
{
	public:
		// Adds a modifier to the derived object.
		// before = 0				:Place modifier at the end of the pipeline (top of stack)
		// before = NumModifiers()	:Place modifier at the start of the pipeline (bottom of stack)
		/*! \remarks Adds a modifier to this derived object.
		\par Parameters:
		<b>Modifier *mod</b>\n\n
		The modifier to add.\n\n
		<b>ModContext *mc=NULL</b>\n\n
		The mod context for the modifier.\n\n
		<b>int before=0</b>\n\n
		If this value is set to 0 then the modifier will be placed at the end
		of the pipeline (top of stack). If this value is set to
		<b>NumModifiers()</b> then the modifier will be placed at the start of
		the pipeline (bottom of stack). */
		virtual void AddModifier(Modifier *mod, ModContext *mc=NULL, int before=0)=0;				
		/*! \remarks Deletes the specified modifier from the stack.
		\par Parameters:
		<b>int index=0</b>\n\n
		The index of the modifier to delete. */
		virtual void DeleteModifier(int index=0)=0;
		/*! \remarks Returns the number of modifiers this derived object has.
		*/
		virtual int NumModifiers()=0;		

		// Searches down the pipeline for the base object (an object that is not a
		// derived object). May step into other derived objects. 
		// This function has been moved up to Object, with a default implementation
		// that just returns "this".  It is still implemented by derived objects and
		// WSM's to search down the pipeline.  This allows you to just call it on
		// a Nodes ObjectRef without checking for type.
//		virtual Object *FindBaseObject()=0;
		
		// Get and set the object that this derived object reference.
		// This is the next object down in the stack and may be the base object.
		/*! \remarks Gets the object that this derived object references. This
		is the next object down in the stack and may be the base object.
		\return  The object that this derived object references. */
		virtual Object *GetObjRef()=0;
		/*! \remarks Sets the object that this derived object references. This
		is the next object down in the stack and may be the base object.
		\par Parameters:
		<b>Object *pob</b>\n\n
		The object that this derived object should reference.
		\return  One of the following values:\n\n
		<b>REF_SUCCEED</b>\n\n
		<b>REF_FAIL</b> */
		virtual RefResult ReferenceObject(Object *pob)=0;

		// Access the ith modifier.
		/*! \remarks Returns the modifier specified by the index.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the modifier to return. */
		virtual Modifier *GetModifier(int index)=0;

		// Replaces the ith modifier in the stack
		/*! \remarks This method replaces the modifier in the stack whose
		index is passed.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the modifier in the stack.\n\n
		<b>Modifier *mod</b>\n\n
		The modifier that will replace it. */
		virtual void SetModifier(int index, Modifier *mod)=0;

		// Access the mod context for the ith modifier
		/*! \remarks Returns the ModContext of the specified modifier.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the modifier in the stack. */
		virtual ModContext* GetModContext(int index)=0;
		
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to evaluate the pipeline starting with a
		specific modifier index. Prior to version 4.0 you had to turn all the
		modApps off, evaluate and then turn them on again. Now this can be
		easily done by specifying the modifier index.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		Specifies the time to evaluate the object.\n\n
		<b>int modIndex = 0</b>\n\n
		The index of the modifier.
		\return  The result of evaluating the object as an ObjectState. */
		virtual ObjectState Eval(TimeValue t, int modIndex = 0)=0;

		// pass any notifies onto my ObjRef, such as node attaches/deletes - jbw 9.9.00
		void NotifyTarget(int msg, RefMakerHandle rm) { if (GetObjRef()) GetObjRef()->NotifyTarget(msg, rm); }	

		using Object::GetInterface;
		CoreExport virtual void* GetInterface(ULONG id);
	};

// Create a world space or object space derived object.
// If the given object pointer is non-NULL then the derived
// object will be set up to reference that object.
/*! \remarks This method creates a world space derived object.
\par Parameters:
<b>Object *pob=NULL</b>\n\n
If non-NULL then the WS derived object will be set up to reference this object.
\return  A pointer to the derived object.  */
CoreExport IDerivedObject *CreateWSDerivedObject(Object *pob=NULL);
/*! \remarks This method creates an object space derived object.
\par Parameters:
<b>Object *pob=NULL</b>\n\n
If non-NULL then the derived object will be set up to reference this object.
\return  A pointer to the derived object. */
CoreExport IDerivedObject *CreateDerivedObject(Object *pob=NULL);

enum PipeEnumResult {
	PIPE_ENUM_CONTINUE,
	PIPE_ENUM_STOP
};

// This is the callback procedure for pipeline enumeration. The ReferenceTarget 
// passed to the proc can be a Node, Modifier or Object. In case it is a Modifier
// derObj contains the DerivedObject and the index is the index of this modifier 
// in the DerivedObject. In all other cases derObj is NULL and index is 0;
// In case the flag includeEmptyDOs is declared as true, the proc will be called
// even for DerivedObjects, that don't contain any modifiers. In that case the
// object pointer will be NULL, the derObj pointer will contain the DerivedObject
// and the index will be -1.

/*! \sa  Class InterfaceServer, Class INode,  Class Object,  Class Modifier, Class IDerivedObject, Class ModContext, Class NotifyCollapseEnumProc.\n\n
\par Description:
This class and its associated global functions are available in release 4.0 and
later only.\n\n
This is the callback object for the global geometry pipeline enumeration
functions. The single <b>proc()</b> method of this class is called as the
enumeration takes place.\n\n
The following functions are not methods of this class but are available for use
with it to begin the enumeration:\n\n
These all start a pipeline enumeration down the pipeline towards the baseobject
and over the baseobjects' branches in case it is a compound object. A pipleine
enumeration can be started from a Node, an Object or from a Modifier. */
class GeomPipelineEnumProc : public InterfaceServer
{
public:	
	/*! \remarks This is the callback procedure for pipeline enumeration. The
	ReferenceTarget passed to the proc can be a Node, Modifier or Object. In
	case it is a Modifier the parameter <b>derObj</b> contains the
	DerivedObject and the index is the index of this modifier in the
	DerivedObject. In all other cases <b>derObj</b> is NULL and index is 0.
	\par Parameters:
	<b>ReferenceTarget *object</b>\n\n
	Points to the item in the geometry pipeline. This can be a Node, Modifier
	or Object.\n\n
	<b>IDerivedObject *derObj</b>\n\n
	If <b>object</b> above is a Modifier this points to the derived object.\n\n
	<b>int index</b>\n\n
	If <b>object</b> aive is a Modifier this is the index of this modifier in
	the DerivedObject.
	\return  One of the following values which determines how the enumeration
	proceeds:\n\n
	<b>PIPE_ENUM_CONTINUE</b>\n\n
	Specifies to continue the enumeration.\n\n
	<b>PIPE_ENUM_STOP</b>\n\n
	Specifies to halt the enumeration. */
	virtual PipeEnumResult proc(ReferenceTarget *object, IDerivedObject *derObj, int index)=0;
};

//---------------------------------------------------------------------------
// Pipeline Enumeration

// These methods start a pipeline enumeration down the pipeline towards the baseobject
// and over the baseobjects' branches in case it is a compound object.
// A pipleine enumeration can be started from a Node, an Object or from a Modifier. 
// In case it is started from a Modifier, the client has to provide the IDerviedObject the 
// Modifier is applied to and the index of the Modifier in the IDerivedObject. One can use 
// the method Modifier::GetIDerivedObject(); in order to get the IDerviedObject 
// and the index, given a modifier and a ModContext.

/*! \remarks The global function begins an enumeration of the geometry
pipeline using the specified node.
\par Parameters:
<b>GeomPipelineEnumProc *gpep</b>\n\n
Points to the callback object to process the enumeration.\n\n
<b>INode *start</b>\n\n
Points to the node to start the enumeration.\n\n
<b>bool includeEmptyDOs = false</b>\n\n
In case the flag includeEmptyDOs is declared as true, the proc will be called
even for DerivedObjects, that don't contain any modifiers. In that case the
object pointer will be NULL, the derObj pointer will contain the DerivedObject
and the index will be -1.
\return  One of the following values:\n\n
<b>PIPE_ENUM_CONTINUE</b>\n\n
<b>PIPE_ENUM_STOP</b> */
CoreExport int EnumGeomPipeline(GeomPipelineEnumProc *gpep, INode *start, bool includeEmptyDOs = false);
/*! \remarks The global function begins an enumeration of the geometry
pipeline using the specified object.
\par Parameters:
<b>GeomPipelineEnumProc *gpep</b>\n\n
Points to the callback object to process the enumeration.\n\n
<b>Object *start</b>\n\n
Points to the object to start the enumeration.\n\n
<b>bool includeEmptyDOs = false</b>\n\n
In case the flag includeEmptyDOs is declared as true, the proc will be called
even for DerivedObjects, that don't contain any modifiers. In that case the
object pointer will be NULL, the derObj pointer will contain the DerivedObject
and the index will be -1.
\return  One of the following values:\n\n
<b>PIPE_ENUM_CONTINUE</b>\n\n
<b>PIPE_ENUM_STOP</b> */
CoreExport int EnumGeomPipeline(GeomPipelineEnumProc *gpep, Object *start, bool includeEmptyDOs = false);
/*! \remarks The global function begins an enumeration of the geometry
pipeline using the specified derived object and modifier index. The caller of
this method has to provide the IDerviedObject the Modifier is applied to and
the index of the Modifier in the IDerivedObject. Developers can use the method
<b>Modifier::GetIDerivedObject()</b> in order to get the IDerviedObject and the
index, given a modifier and a ModContext.
\par Parameters:
<b>GeomPipelineEnumProc *gpep</b>\n\n
Points to the callback object to process the enumeration.\n\n
<b>IDerivedObject *start</b>\n\n
Points to the derived object to start the enumeration.\n\n
<b>int modIndex = 0</b>\n\n
The zero based index of the modifier in the derived object to start with.\n\n
<b>bool includeEmptyDOs = false</b>\n\n
In case the flag includeEmptyDOs is declared as true, the proc will be called
even for DerivedObjects, that don't contain any modifiers. In that case the
object pointer will be NULL, the derObj pointer will contain the DerivedObject
and the index will be -1.
\return  One of the following values:\n\n
<b>PIPE_ENUM_CONTINUE</b>\n\n
<b>PIPE_ENUM_STOP</b>  */
CoreExport int EnumGeomPipeline(GeomPipelineEnumProc *gpep, IDerivedObject *start, int modIndex = 0, bool includeEmptyDOs = false);

//---------------------------------------------------------------------------
// Collapse Notification 

// Whenever the modifier stack is collapsed the code has to notify all objects in the 
// stack with a Pre and a Post notification. In order to do this, the 
// NotifyCollapseEnumProc can be used in conjunction with the method 
// EnumGeomPipleine(). In the constructor one can specify, if it is a pre-, 
// or post-collapse notification. In case it is a postcollapse the object that 
// represents the result of the collapse has to be provided as well. The
// INode pointer to the beginning of the pipeline that was collapsed has to be 
// provided in both cases.

/*! \sa  Class GeomPipelineEnumProc,  Class BaseObject,  Class Object, Class IDerivedObject.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
Whenever the modifier stack is collapsed the code has to notify all objects in
the stack with a Pre and a Post notification. In order to do this, this class
can be used in conjunction with the method <b>EnumGeomPipleine()</b>. In the
constructor one can specify, if it is a pre- or post- collapse notification. In
case it is a post collapse the object that represents the result of the
collapse has to be provided as well. The INode pointer to the beginning of the
pipeline that was collapsed has to be provided in both cases.\n\n
All methods of this class are implemented by the System.  */
class NotifyCollapseEnumProc : public GeomPipelineEnumProc
{
bool bPreCollapse;
INode *node;
Object *collapsedObject;
public:	
	/*! \remarks Constructor. The private data members are initialized by the
	passed parameters.
	\par Parameters:
	<b>bool preCollapse</b>\n\n
	Indicates if this is a pre- collapse or a post- collapse. Pass true for pre and
	false for post.\n\n
	<b>INode *n</b>\n\n
	Points to the node at the beginning of the pipeline that was collapsed.\n\n
	<b>Object *collapsedObj = NULL</b>\n\n
	If this is a post- collapse then points to the object which is the result of
	the collapse.
	\par Sample Code:
	The following code fragment shows using this constructor and then the
	EnumGeomPipeline with this object:\n\n
	<b>NotifyCollapseEnumProc PreNCEP(true,node);</b>\n\n
	<b>EnumGeomPipeline(\&PreNCEP,node);</b> */
	NotifyCollapseEnumProc(bool preCollapse, INode *n, Object *collapsedObj = NULL) : bPreCollapse(preCollapse), node(n), collapsedObject(collapsedObj) {}
	/*! \remarks This is the implementation of the EnumGeomPipeline callback
	method proc() which calls BaseObject::NotifyPreCollapse or
	BaseObject::NotifyPostCollapse as required. */
	virtual PipeEnumResult proc(ReferenceTarget *object,IDerivedObject *derObj, int index) {
		if(object->ClassID() == Class_ID(BASENODE_CLASS_ID,0))
			return PIPE_ENUM_CONTINUE;

		if(bPreCollapse)
			((BaseObject *) object)->NotifyPreCollapse(node, derObj, index);
		else
			((BaseObject *) object)->NotifyPostCollapse(node, collapsedObject, derObj, index);

		return PIPE_ENUM_CONTINUE;
	}

};

//! \brief Class used by EnumGeomPipeline to notify the pipeline of a collapse as well as maintaing the Custom Attributes. (Obsolete)
/*! 
This class is used in conjunction with ICustAttribCollapseManager.  The calling code needs to make sure they call the correct
version of the enumeration.  
Note: this class is superceded by NotifyCollapseMaintainCustAttribEnumProc2 as it does not provide the capability for handling
cloning of the node's base object correctly.
*/
class NotifyCollapseMaintainCustAttribEnumProc : public GeomPipelineEnumProc
{
	bool bPreCollapse;
	INode *node;
	Object *collapsedObject;
	bool bCopied;
public:	
	
	//! \brief Constructor. The private data members are initialized by the passed parameters
	/*! \param [in] preCollapse Indicates if this is a pre- collapse or a post- collapse. Pass true for pre and false for post.
	\param [in] n Points to the node at the beginning of the pipeline that was collapsed.
	\param [in] collapsedObj If this is a post- collapse then points to the object which is the result of the collapse.
	*/
	CoreExport NotifyCollapseMaintainCustAttribEnumProc(bool preCollapse, INode *n, Object *collapsedObj = NULL);
	//! \brief This is the implementation of the EnumGeomPipeline callback method proc().
	/*! On a pre-collapse enumeration, collects the custom attributes on each object and its references (recursively), and
	calls NotifyPreCollapse on the object.
	On a post-collapse enumeration, applies the custom attributes previously collected to the collapsed object, and
	calls NotifyPostCollapse on the object.
	*/
	CoreExport virtual PipeEnumResult proc(ReferenceTarget *object,IDerivedObject *derObj, int index); 

};

//! \brief Class used by EnumGeomPipeline to notify the pipeline of a collapse as well as maintaing the Custom Attributes.
/*! 
This class is used in conjunction with ICustAttribCollapseManager.  The calling code needs to make sure they call the correct
version of the enumeration.  The following is a code example of its usage.

ICustAttribCollapseManager * iCM = ICustAttribCollapseManager::GetICustAttribCollapseManager();

if(iCM && iCM->GetCustAttribSurviveCollapseState())
{
NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false,oldObj,true,obj);	//use the CA safe version
EnumGeomPipeline(&PostNCEP,oldObj);
}
else
{
NotifyCollapseEnumProc PostNCEP(false,node,obj);	//use the original
EnumGeomPipeline(&PostNCEP,oldObj);
}
*/
class NotifyCollapseMaintainCustAttribEnumProc2 : public GeomPipelineEnumProc
{
	INode *node;
	Object *collapsedObject;
	Object *nodeBaseObject;
	bool bPreCollapse;
	bool bCopied;
	bool bIgnoreBaseObjectCAs;
public:	

	//! \brief Constructor. The private data members are initialized by the passed parameters
	/*! \param [in] preCollapse Indicates if this is a pre- collapse or a post- collapse. Pass true for pre and false for post.
	\param [in] node Points to the node at the beginning of the pipeline that was collapsed.
	\param [in] ignoreBaseObjectCAs If this is a post- collapse then specifies whether to apply CAs collected from the node's base object.
	\param [in] collapsedObj If this is a post- collapse then points to the object which is the result of the collapse.
	*/
	CoreExport NotifyCollapseMaintainCustAttribEnumProc2(bool preCollapse, INode *node, bool ignoreBaseObjectCAs = false, Object *collapsedObj = NULL);
	//! \brief This is the implementation of the EnumGeomPipeline callback method proc().
	/*! On a pre-collapse enumeration, collects the custom attributes on each object and its references (recursively), and
	calls NotifyPreCollapse on the object.
	On a post-collapse enumeration, applies the custom attributes previously collected to the collapsed object, and
	calls NotifyPostCollapse on the object. If ignoreBaseObjectCAs is true, CAs that were collected from the base object and its references
	are not applied. This is to account for cases of collapsing the stack where the base object is cloned. When the base object is cloned,
	it will create its own copy of the CAs. 
	*/
	CoreExport virtual PipeEnumResult proc(ReferenceTarget *object,IDerivedObject *derObj, int index); 

};

