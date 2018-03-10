	 /**********************************************************************
 
	FILE: IAssembly.h

	DESCRIPTION:  Public interface for setting and getting assembly flags

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created April 03, 2001

 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "maxtypes.h"

// This type is not being used currently 
typedef int AssemblyCode;

// class IAssembly
//
// This interface allows for setting and retrieving assembly membership
// information to\from nodes. All methods of the interface are implemented 
// by the system (Max).
// Client code can query an INode for this interface:
// INode* n;
// IAssembly* a = GetAssemblyInterface(n);
/*! This interface class allows for setting and retrieving assembly membership
information to or from nodes. All methods are implemented by the system (Max).
Client code can query an INode for this interface:\n\n
INode* n;\n\n
IAssembly* a = GetAssemblyInterface(n);\n\n
  */
class IAssembly : public FPMixinInterface 
{
	public:
		//
		// -- Methods for setting assembly flags
		//

		// NOTE: nodes can be both assembly members and heads in the same time.
		
		// If b=TRUE, sets a node as an assembly member.
		// If b=FALSE sets a closed or open assembly member as not an assembly member
		// If the member is open, it closes it first then removes the member flag
		// To close an assembly member, call SetAssemblyMemberOpen(FALSE)
		/*! \remarks Method for setting state of assembly member. To close an
		assembly member call SetAssemblyMemberOpen(FALSE), as documented
		below.\n\n

		\par Parameters:
		<b>BOOL b</b>\n\n
		Specifies a new state for an assembly member.\n\n
		If TRUE the node is set as an assembly member.\n\n
		If FALSE removes a closed or open assembly member from membership. An
		open member will be closed first and then have it's membership flag
		removed.\n\n
		  */
		virtual void SetAssemblyMember(BOOL b) = 0;
		
		// Should only be called on assembly members
		// If b=TRUE opens an assembly member
		// If b=FALSE closes an assembly member
		/*! \remarks Method for opening or closing an assembly member. It
		should only be called on members.\n\n

		\par Parameters:
		<b>BOOL b</b>\n\n
		Specifies the state of the assembly member.\n\n
		If TRUE the assembly member is opened.\n\n
		If FALSE the assembly member is closed.\n\n
		  */
		virtual void SetAssemblyMemberOpen(BOOL b) = 0;

		// If b=TRUE sets a node as an assembly head
		// If b=FALSE sets a closed or open assembly head as not an assembly head
		// If the head is open, it closes it first then removes the head flag
		// To close an assembly head, call SetAssemblyHeadOpen(FALSE)
		/*! \remarks Method to designate an assembly member as the assembly
		head.\n\n

		\par Parameters:
		<b>BOOL b</b>\n\n
		Specifies the head state of the member.\n\n
		If TRUE the node is set as the assembly head.\n\n
		If FALSE an open or closed assembly head becomes a non-head. If the
		head is open, it is first closed and then the head flag is removed. To
		close an assembly head call SetAssemblyHeadOpen(FALSE), as documented
		below.\n\n
		  */
		virtual void SetAssemblyHead(BOOL b) = 0;
		
		// Should only be called on assembly heads
		// If b=TRUE opens an assembly head
		// If b=FALSE closes an assembly head
		/*! \remarks Method for opening or closing an assembly head. It should
		only be called on an assembly head.\n\n

		\par Parameters:
		<b>BOOL b</b>\n\n
		Specifies the state of the assembly head.\n\n
		If TRUE the assembly head is opened.\n\n
		If FALSE the assembly head is closed.\n\n
		  */
		virtual void SetAssemblyHeadOpen(BOOL b) = 0;
		
		//
		// -- Methods for querying the assembly flags
		//
		
		// NOTE: to detect closed assembly members\heads, check for both 
		// the assembly member\head flag and open member\head flags:
		// IsAssemblyHead() && !IsAssemblyMemberOpen()

		// Returns TRUE for both closed and open assembly members\heads
		/*! \remarks Method to determine membership in an assembly. It will
		work with either open or closed members.\n\n

		\return  If TRUE, node is a member the assembly.\n\n
		If FALSE, node is not a member of the assembly.\n\n
		  */
		virtual BOOL IsAssemblyMember() const = 0;
		/*! \remarks Method to determine if a node is an assembly head. It
		works with either open or closed heads.\n\n

		\return  If TRUE, node is an assembly head.\n\n
		If FALSE, node is not an assembly head.\n\n
		  */
		virtual BOOL IsAssemblyHead() const = 0;
		
		// Returns TRUE for open assembly members\heads
		/*! \remarks Method to determine if an assembly member is open.\n\n

		\return  If TRUE, the assembly member is open.\n\n
		If FALSE, the assembly member is not open.\n\n
		  */
		virtual BOOL IsAssemblyMemberOpen() const = 0;
		/*! \remarks Method to determine if an assembly head is open.\n\n

		\return  If TRUE, the assembly head is open.\n\n
		If FALSE, the assembly head is not open.\n\n
		  */
		virtual BOOL IsAssemblyHeadOpen() const = 0;
		
		// This method is used for detecting assemblies in assemblies. 
    // The method checks if this assembly node is a head node and whether it's a
    // member of the assembly head node passed in as parameter.
		/*! \remarks Method to detect assemblies within assemblies. It checks whether
		this assembly node is a head node and is also a member of the assembly headed
		by the node passed in as a parameter.\n\n

		\return  If TRUE, node is both a head node and is a member of another
		assembly.\n\n
		If FALSE, node is neither a head node nor a member of another assembly.\n\n*/
		virtual BOOL IsAssemblyHeadMemberOf(const IAssembly* const assemblyHead) const = 0;

		// Allow persistance of info kept in object implementing this interface
		/*! \remarks Write method for implementing persistence of the
		underlying object.\n\n

		\par Parameters:
		ISave* isave\n\n
		Pointer for write methods.\n\n

		\return  <b>IO_OK, the call succeeded.</b>\n\n
		<b>IO_ERROR, the call was unsuccessful.</b>\n\n
		  */
		virtual IOResult Save(ISave* isave) = 0;
		/*! \remarks Read method for implementing persistence of the
		underlying object.\n\n

		\par Parameters:
		ILoad* iload\n\n
		Pointer for read methods.\n\n

		\return  <b>IO_OK, the call succeeded.</b>\n\n
		<b>IO_ERROR, the call was unsuccessful.</b> */
		virtual IOResult Load(ILoad* iload) = 0;

		// -- IAssembly function publishing
		// Methods IDs
		enum 
		{ 
			E_SET_ASSEMBLY_MEMBER, 
			E_GET_ASSEMBLY_MEMBER, 
			E_SET_ASSEMBLY_HEAD, 
			E_GET_ASSEMBLY_HEAD, 
			E_SET_ASSEMBLY_MEMBER_OPEN, 
			E_GET_ASSEMBLY_MEMBER_OPEN, 
			E_SET_ASSEMBLY_HEAD_OPEN,
			E_GET_ASSEMBLY_HEAD_OPEN,
		}; 
	
}; 

// Assembly interface ID
#define ASSEMBLY_INTERFACE Interface_ID(0x2512714b, 0x4b456518)

inline IAssembly* GetAssemblyInterface(BaseInterface* baseIfc)	
{ DbgAssert( baseIfc != NULL); return static_cast<IAssembly*>(baseIfc->GetInterface(ASSEMBLY_INTERFACE)); }

// class IAssembly2
//
// This new version of the assembly interface extends IAssembly
// SDK programmers are encouraged to use this version of the assembly interface
// 
// All methods of the interface are implemented by the system (Max).
// Client code can query an INode for this interface:
// INode* n;
// IAssembly2* a = GetAssemblyInterface2(n);
/*! This new version of the assembly interface extends IAssembly. Developers
are encouraged to use this version of the assembly interface.\n\n
Client code can query an INode for this interface:\n\n
INode* n;\n\n
IAssembly2* a = GetAssemblyInterface2(n);\n\n
  <br>  These methods should be called on assembly heads only. Calling
them on members will not affect the display of the bounding box. The bounding
box is displayed in red (by default) around an open assembly. Turning it off
can reduce viewport clutter; it won't affect the functionality of the assembly
(the way the assembly works).\n\n
Calling these methods on an assembly head, affects the display of the bounding
box only on that assembly.\n\n
  */
class IAssembly2 : public IAssembly
{
	public:
		// Methods to control the display of assembly world 
		// bounding box of open assemblies, on a per assembly basis
		// These methods should be called on assembly head nodes only
		// If called on assembly member nodes, the display of the world 
		// bounding box won't get turned off.
		/*! \remarks Implemented by the System. Method to control the display
		of an assembly's world space bounding box.\n\n

		\par Parameters:
		<b>BOOL b</b>\n\n
		If TRUE, display the bounding box.\n\n
		If FALSE, do not display the bounding box.\n\n
		  */
		virtual void SetAssemblyBBoxDisplay(BOOL b) = 0;
		/*! \remarks Implemented by the system. Method to retrieve the value
		of the bounding box display flag.\n\n

		\return  <b>BOOL</b>\n\n
		If TRUE, the bounding box display is enabled.\n\n
		If FALSE, the bounding box display is disabled. */
		virtual BOOL GetAssemblyBBoxDisplay() = 0;

		// -- IAssembly2 function publishing
		// Methods IDs
		enum 
		{ 
			E_SET_ASSEMBLY_BBOX_DISPLAY = IAssembly::E_GET_ASSEMBLY_HEAD_OPEN + 1, 
			E_GET_ASSEMBLY_BBOX_DISPLAY, 
		};

};
#define ASSEMBLY_INTERFACE2 Interface_ID(0x6fd5515a, 0x353c6734)
inline IAssembly2* GetAssemblyInterface2(BaseInterface* baseIfc)	
{ DbgAssert( baseIfc != NULL); return static_cast<IAssembly2*>(baseIfc->GetInterface(ASSEMBLY_INTERFACE2)); }

