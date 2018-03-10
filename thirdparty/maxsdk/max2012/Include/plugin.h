#pragma once

// Copyright 2010 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.

#include "maxheap.h"
#include "strclass.h"
#include "plugapi.h"
#include "Path.h"
#include "Noncopyable.h"
#include "BuildWarnings.h"
#include "DataClassDesc.h"

/** \defgroup RequiredPluginDllFunction Required Plug-in DLL Functions
Plug-in DLLs must implement and export a number of functions in order to load
and work properly in 3ds Max. The following types represent pointers to these functions.*/
//@{ 
/** Pointer to a function with the following signature: ULONG LibVersion().
	LibVersion() must return the VERSION_3DSMAX pre-defined constant which encodes 
	the version of 3ds Max the plug-in is compatible with. This allows 3ds Max to 
	identify obsolete plug-in DLLs. */
typedef const ULONG (*LibVersionProcPtr)();

/** Pointer to a function with the following signature: MCHAR* LibDescription().
	Returns a string that describes the plug-in DLL. */
typedef const MCHAR* (*LibDescriptionProcPtr)();

/** Pointer to a function with the following signature: int LibNumberClasses().
	Returns the number of plug-in classes (number of ClassDesc derived objects) 
	this plug-in Dll exposes.*/
typedef int (*LibNumberClassesProcPtr)();

/** Pointer to a function with the following signature: ClassDesc* LibClassDesc(int)
	Returns the i-th class descriptor (ClassDesc derived object). 
	See class ClassDesc and ClassDesc2 for more information. */
typedef ClassDesc* (*LibClassDescProcPtr)(int i);
//@}

/** \defgroup OptionalPluginDllFunction Optional Plug-in DLL Functions
Plug-in DLLs can optionally take advantage of some 3ds Max features by implementing 
and exporting a number of functions. The following types represent pointers to these functions. */
//@{ 
/** Pointer to a function with the following signature: int LibInitialize()
	Allows to perform one-time plug-in data initialization. Returns TRUE if the 
	plug-in initialization was successful, FALSE otherwise. If the function returns 
	FALSE, the system will unload the plug-in dll via a call to FreeLibrary(). 
	This function may be called more then once. */
typedef int (*LibInitializeProcPtr)();

/** Pointer to a function with the following signature: int LibShutdown()
	Allows to perform one-time plug-in data un-initialization. It's called once, 
	just before the plug-in is unloaded. This function may be called more then once. */
typedef int (*LibShutdownProcPtr)();

/** Pointer to a function with the following signature: BOOL CanAutoDefer() 
	Plug-ins that do not wish or cannot be defer loaded should implement this method 
	to return FALSE. If the method returns true or is not implemented, 3ds Max will 
	attempt to defer load the plug-in. For more information see the topic called 
	"Deferred Loading of Plug-ins" in the 3ds Max SDK Programmer's Guide.*/
typedef ULONG (*CanAutoDeferProcPtr)();
//@}

/** This class models a plug-in DLL. 
	3ds Max creates an instance of this class for each plugin dlls that it loads.
	This class allows for querying a number of plug-in DLL properties and the plug-in classes that expose.
	
	Plugins must be built with support for RTTI, and must export all of the following 
	functions in order for them to be loaded into 3ds Max.
	\li ULONG LibVersion(void) - See ::LibVersionProcPtr for details
	\li const MCHAR* LibDescription(void) - See ::LibDescriptionProcPtr
	\li int LibNumberClasses(void) - See ::LibNumberClassesProcPtr
	\li ClassDesc* LibClassDesc(int i) - See ::LibClassDescProcPtr
	
	Plugins may implement the following functions for added flexibility:
	\li int LibInitialize(void) - See ::LibInitializeProcPtr
	\li int LibShutdown(void) - See ::LibShutdownProcPtr
	\li ULONG CanAutoDefer(void) - See ::CanAutoDeferProcPtr
	
	Class DllDir represents the plug-in Dll "registry" and offers various way to get to 
	DllDesc instances.
	\sa Class DllDir, Class ClassDesc, Class Interface, 
	\sa <a href="ms-its:3dsmaxsdk.chm::/start_dll_functions.html">Required DLL Functions and Class Descriptors</a>
	\sa <a href="ms-its:3dsMaxSDK.chm::/deferred_loading.html">Deferred Loading of Plugins</a>
*/
class DllDesc: public MaxHeapOperators
{
public:
	/** Returns true if the corresponding plug-in has been successfully loaded and 
	initialized, false otherwise or if the plug-in has been defer-loaded.*/
	CoreExport bool IsLoaded() const;

	/** Returns the Description defined in the LibDescription function. */
	CoreExport const MCHAR* Description() const;

	/** Returns the number of classes implemented in the DLL. This is
	the value returned by the library function LibNumberClasses. */
	CoreExport int NumberOfClasses() const;

	/** Returns a pointer to the 'i-th' class descriptor.
	\param i - The zero based index of the ClassDesc to return.
	\return Pointer to a class descriptor. */
	CoreExport ClassDesc* operator[](int i) const;

	/** Returns a pointer to the 'i-th' class descriptor.
	This method is identical to DllDesc::operator[](int).
	\param i - The zero based index of the ClassDesc to return.
	\return Pointer to a class descriptor. */
	CoreExport ClassDesc* GetClassDesc(int i) const;

	/** Equality operator. Always returns zero. */
	CoreExport int operator==( const DllDesc& dd ) const;

	/** Checks if this plugins dll was built with run-time type information (RTTI).
	\pre NumberOfClasses must return a positive non-zero value for the check to be performed
	\return Returns true if the dll was built with RTTI, otherwise false. 
	In the later case, the plugin dll will not be loaded into 3ds Max and 
	DllDesc::IsLoaded() will return false. */
	CoreExport bool HasRttiSupport() const;

	/** Returns true if the plug-in DLL is binary compatible with the version of
	3ds Max where it's being loaded into, false otherwise. If it return false the 
	plugin dll will not be loaded into 3ds Max and DllDesc::IsLoaded() will return false. */
	CoreExport bool IsBinaryCompatible() const;

	/** Returns the value reported by the LibVersion() function implemented by the plug-in DLL.*/
	CoreExport ULONG GetVersion() const;

	/** Returns whether the corresponding plug-in DLL can be defer loaded or not. */
	CoreExport bool IsDeferrable() const;

	/** Returns whether the corresponding plug-in DLL implements the CanAutoDefer function or not. */
	CoreExport bool ImplementsCanAutoDefer() const;

	/** Verifies the validity of the specified plug-in DLL version.
	The supplied version number is valid if it identifies a 3ds Max SDK version that
	is binary compatible with the one used to compile the instance of 3ds Max that is
	being asked to validate this version number.
	\param pluginDllVersion The plug-in DLL version to validate. This is a value 
	returned by the plug-in DLL's LibVersion() function.
	\return True if the supplied plug-in DLL version number corresponds to a plug-in 
	DLL that is binary compatible with the instance of 3ds Max that is currently used. */
	CoreExport static bool IsPluginDllVersionValid(ULONG pluginDllVersion);

	/** Returns the full path of the plug-in DLL file this instance of DllDesc represents */
	CoreExport const MSTR& GetFullPath() const;

	/** Returns the file name and extension of the plug-in DLL this instance of DllDesc represents */
	CoreExport const MSTR& GetFileName() const;

	/** Returns the folder name with a trailing backslash of the plug-in DLL 
	this instance of DllDesc represents */
	CoreExport const MSTR& GetDirectoryName() const;

	/** Returns the module handle of the plug-in DLL represented by this DllDesc instance. 
	The module handle should not be used to load or unload the plug-in DLL.*/
	CoreExport const HINSTANCE GetHandle() const;

	/** Returns the size of the linear space that the plug-in DLL occupies in memory, 
	in bytes, or 0 if the plug-in DLL is not loaded. */
	CoreExport DWORD GetSize() const;

	/** Returns the date and time the plug-in DLL was last modified. */
	CoreExport FILETIME GetTimeLastModified() const;

	/** \deprecated Deprecated in 3ds Max 2012 and there's no replacement for it. 
	DllDesc::Free() was effectively unloading the plug-in DLL which is currently 
	not supported.
	Decrements the reference count of the plug-in DLL. 
	When the reference count reaches zero, the module is unmapped from the address
	space of the calling process. */
	MAX_DEPRECATED void Free();

	/** \deprecated Deprecated in 3ds Max 2012 and there's no replacement for it. 
	A plug-in's LibInitialize method should be called only once by 3ds Max as part of
	the plug-in's loading process. */
	MAX_DEPRECATED BOOL Initialize();

	/** \deprecated  Deprecated in 3ds Max 2012 and there's no replacement for it. 
	A plug-in's LibShutdown method should be called only once by 3ds Max as part of
	the plug-in's unloading process.*/
	MAX_DEPRECATED BOOL Shutdown();

	// The following methods are supposed to be used only internally, therefore
	// they are not exported.

	// Constructor - Use it to create a DllDesc that represents a plug-in DLL that was 
	// loaded (had a valid HINSTANCE), as  opposed to being defer loaded.
	explicit DllDesc(
		HINSTANCE dllHandle, 
		const MCHAR* dllFullPath, 
		LibVersionProcPtr libVersionProcPtr,
		LibDescriptionProcPtr libDescriptionProcPtr,
		LibNumberClassesProcPtr libNumberClassesProcPtr,
		LibClassDescProcPtr libClassDescProcPtr,
		LibInitializeProcPtr libInitializeProcPtr,
		LibShutdownProcPtr libShutdownProcPtr,
		CanAutoDeferProcPtr canAutoDeferProcPtr,
		const FILETIME& timeLastWrite);

	// Constructor - Use it create a DllDesc that represents a defer loaded plug-in DLL
	explicit DllDesc(const MCHAR* dllFullPath, const MCHAR* dllDescription, const FILETIME& timeLastWrite);
	
	// Unloads the plug-in DLL that corresponds to this DllDesc instance
	void Unload();

	// Calls a plug-in's LibInitialize method
	BOOL CallInitialize();

	// Calls a plug-in's LibShutdown method 
	BOOL CallShutdown();

	// Copy constructor 
	DllDesc(const DllDesc& other);

	// Assignment operator
	DllDesc operator=(const DllDesc& other);

	// Destructor 
	~DllDesc(); 

private:
	class DllDescImpl;
	MaxSDK::AutoPtr<DllDescImpl> mImpl;

	// Disallowed method
	DllDesc();
};

/*-----------------------------------------------------------------
We will build one big DllDir on startup.
As we do this, we will build a set of lists, one for each SuperClass.
For a given super class, we want to 
(a) Enumerate all public subclasses 
(b) Enumerate all subclasses
(c) Find the subClass for a given subClass ID.

--------------------------------------------------------------*/

/** This class wraps a ClassDesc instance and maintains run-time related information about it.
	One class ClassEntry instance for each ClassDesc is created by 3ds Max and stored 
	in the ClassDirectory. The ClassEntry maintains information such as the usage count, 
	rollout state etc. All methods of this class are implemented by the system. 
	Client code can access ClassEntry instance via ClassDirectory::FindClassEntry() or 
	various methods of class SubClassList.
	\sa  Class DllDir, Class SubClassList, Class ClassDesc, Class Class_ID. */
#pragma warning(push)
#pragma warning(disable:4100)
class ClassEntry : public MaxHeapOperators
{
	public:
		CoreExport ClassEntry();
		CoreExport ClassEntry(const ClassEntry &ce);
		CoreExport ClassEntry(ClassDesc *cld, int dllN, int index, bool load);
		CoreExport ~ClassEntry();

		/** Returns the DllDir index of the plug-in dll that exposes the ClassDesc 
		wrapped by this ClassEntry instance. */
		CoreExport int DllNumber() const;
		
		/** Returns nonzero if the class is public; otherwise zero.
		Non-public classes are those that are meant for private use by other plug-ins. */
		CoreExport int IsPublic() const;
		
		/** Returns the Class_ID of the wrapped ClassDesc. */
		CoreExport Class_ID ClassID() const;
		
		/** Returns the class name of the wrapped ClassDesc. */
		CoreExport MSTR& ClassName() const;
		
		/** Returns the category for the wrapped ClassDesc. */
		CoreExport MSTR& Category() const;

		/** Returns the number of instance of this class used in 3ds Max. */
		CoreExport int UseCount() const;

		/** \internal This is used internally to increment the usage count for this entry. */
		CoreExport void IncUseCount();

		/** \internal This is used internally to set the usage count for this	entry. */
		CoreExport void SetUseCount(int i);

		/** Returns nonzero if this entry matches the specified access type; otherwise zero.
		\param accType - One of the following values: 
		ACC_PUBLIC - public classes, 
		ACC_PRIVATE - non-public classes */
		CoreExport int IsAccType(int accType);
		
		/** Returns true if the wrapped ClassDesc is loaded, false if it's defer loaded. */
		CoreExport bool IsLoaded() const;

		/** Returns the index of the wrapped class descriptor within the plug-in Dll */
		CoreExport int ClassNumber() const;

		/** Returns a pointer to the wrapped class descriptor.
		If the class descriptor corresponds to a defer loaded plug-in, 3ds Max will
		load that plug-in automatically as soon as it's needed. Developers to not
		need to load the plug-in themselves. */
		CoreExport ClassDesc* CD() const;

		/** Force the plug-in Dll that exposes the wrapped ClassDesc to load and returns 
		a pointer to the wrapped and loaded class descriptor.
		As of 3ds Max 2012, FullCD() doesn't have to be called to ensure that the wrapped
		ClassDesc is loaded. 3ds Max loads the class descriptor on demand as soon as 
		client code uses it.
		See <a href="ms-its:3dsmaxsdk.chm::/deferred_loading.html">Deferred 	Loading of Plug-Ins</a> for more details. */
		CoreExport ClassDesc* FullCD();

		/** Assignment operator. */
		CoreExport ClassEntry& operator=( const ClassEntry &ce ); 

		/** Equality operator. Always returns 0. */
		CoreExport int operator==( const ClassEntry &ce ) const;

		/** \name Internal methods	*/
		//@{
		/** \internal This method is used internally. */
		CoreExport void Set(ClassDesc *cld, int dllN, int index, bool load);

		/** \internal This method is used internally. */
		CoreExport int GetScroll() const;

		/** \internal This method is used internally. */
		CoreExport void SetScroll(int s); 

		/** \internal Tests if page state has been actually been set, default initializes to 0x7fffffff */
		CoreExport BOOL PageStateSet() const; 

		/** \internal This method is used internally. */
		CoreExport BOOL GetPageState(int i) const;

		/** \internal This method is used internally. */
		CoreExport void SetPageState(int i,BOOL state);

		/** \internal This method is used internally. */
		CoreExport DWORD GetPageState() const;
		//@}

	private:
		class ClassEntryImpl;
		MaxSDK::Util::AutoPtr<ClassEntryImpl> mImpl;
};


/** \name Plug-in access types. */
//@{
/** ClassDesc has public access - plug-in type is exposed to end users in the UI */
#define ACC_PUBLIC 1 
/** ClassDesc has private access - plug-in type is not exposed to end users in the UI, 
but 3ds Max uses it internally, for example when it needs to load plug-in data. */
#define ACC_PRIVATE 2 
/** Convenience access specifier, to allow easy retrieval of both public and private ClassDesc. */
#define ACC_ALL (ACC_PUBLIC|ACC_PRIVATE)
//@}

/*! \brief This class allows developers to provide some additional information on a superclass.
	This class is available in release 3.0 and later only. Currently this includes a color, and a method which draws a
	representative image in a Windows Device Context. DrawRepresentation(...) can return false to 
	indicate that no image was drawn. DrawRepresentation(...) should cache its image (if applicable) as the method is
	called repeatedly while drawing certain UI components (like the schematic view).  
	\sa  Class SubClassList, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.
*/
class SClassUIInfo: public MaxHeapOperators
	{
	public:
		virtual ~SClassUIInfo() {}
	// A color associated with the superclass.  This is currently used to draw nodes
	// in the schematic view at extreme zoom-outs where it is impossible to draw
	// legible node names.
	/*! \remarks Returns a color associated with the specified super class.
	This is currently used to draw nodes in the schematic view at extreme
	zoom-outs where it is impossible to draw legible node names.
	\par Parameters:
	SClass_ID superClassID
	The Super Class whose associated color to return. */
	CoreExport virtual COLORREF Color(SClass_ID superClassID) { return RGB(128, 128, 128); };

	// Draws an image which represents the superclass (usually an icon) in a rectangle
	// in a given Windows DC.  The implementation should attempt to draw the image as fast
	// as possible as this method is called repeatedly while drawing certain UI
	// components.  Return false if no image was drawn and a generic stand-in image will
	// be used.  The provided "bkColor" is the average background color of the surface on
	// which the image is being drawn.  It can be used, if desired, to antialias the image.
	/*! \remarks Draws an image which represents the superclass (usually an
	icon) in a rectangle in a given Windows DC. The implementation should
	attempt to draw the image as fast as possible as this method is called
	repeatedly while drawing certain UI components.
	\par Parameters:
	<b>SClass_ID superClassID</b>
	The super class to draw.
	<b>COLORREF bkColor</b>
	This is the average background color of the surface on which the image is
	being drawn. It can be used, if desired, to antialias the image.
	<b>HDC hDC</b>
	The handle to the device context.
	<b>Rect \&rect</b>
	The rectangle to draw in.
	\return  Return <b>false</b> if no image was drawn and a generic stand-in
	image will be used.
	\par Default Implementation:
	<b>{ return false; }</b> */
	CoreExport virtual bool DrawRepresentation(SClass_ID superClassID, COLORREF bkColor, HDC hDC, Rect &rect) { return false; }
};


/** A collection of ClassEntry objects that have the same super class id.
	Clients can get a SubClassList via ClassDirectory::GetClassList().
	\sa  Class DllDir, Class ClassDirectory, Class ClassEntry, Class ClassDesc, Class SClassUIInfo.
*/
class SubClassList : public MaxSDK::Util::Noncopyable
{
	public:
	/** Returns a reference to the 'i-th' ClassEntry for this super class.
	\param i - The index of the entry to return. Valid values begin at an index of 1. */
	CoreExport ClassEntry& operator[](int i)const;

	/** Returns the index in the list of sub-classes of the class whose Class_ID is passed.
	\param classID - 	Specifies which class to return the index of. */
	CoreExport int FindClass(Class_ID classID) const;
	
	/** Returns the index in the list of sub-classes of the class whose ClassName() is passed.
	\param name - 	Specifies which class to return the index of. */
	CoreExport int FindClass(const MCHAR *name) const;
	
	/** \internal This method is used internally. */
	CoreExport void AddClass(ClassDesc *cld, int dllNum, int index, bool load); 

	/** Returns the number of sub-classes that match the 	specified access type.
	\param accType - One of the following values:
	ACC_PUBLIC - public classes
	ACC_PRIVATE - non-public classes
	ACC_ALL - both of the above (ACC_PUBLIC|ACC_PRIVATE). */
	CoreExport int  Count(int accType) const;
	
	/** Returns the Super class ID corresponding to this sub-class list. */
	CoreExport SClass_ID SuperID() const;
	
	/** Returns the index of the first ClassDesc of the specified type in  the list of sub-classes.
	\param accType - One of the following values:
	ACC_PUBLIC - public classes
	ACC_PRIVATE - non-public classes */
	CoreExport int GetFirst(int accType) const;

	/** Returns the index of the next ClassDesc of the specified type (or -1 at the end).
	\param accType - One of the following values:
	ACC_PUBLIC - public classes
	ACC_PRIVATE - non-public classes */
	CoreExport int GetNext(int accType) const;

	/** Equality operator. */
	CoreExport int operator==( const SubClassList& lst ) const;
	/** Equality operator. */
	CoreExport int operator==(const SubClassList &sl);

	CoreExport int DeleteClass(ClassDesc *cld); 

	/** Allows developer to provide some additional information on a
	superclass. Currently this includes a color, and a method which draws a
	representative image in a Windows DC.
	\param uiInfo - Points to the information to set. */
	CoreExport void SetUIInfo(SClassUIInfo *uiInfo);

	/** This method returns additional user interface related information on a
	given superclass. Returns NULL if no superclass information was assigned. */
	CoreExport SClassUIInfo* GetUIInfo() const;

	/** \internal This method is for internal use only. */
	CoreExport void ReplaceClass(int idx, ClassDesc *cld, int dllNum, int index, bool load); 

	/** \internal Destructor */
	~SubClassList();
	/** \internal Constructor */
	SubClassList(SClass_ID sid);

private:
	class SubClassListImpl;
	MaxSDK::Util::AutoPtr<SubClassListImpl> mImpl;

private:
	SubClassList(); // disallowed
};
#pragma warning(pop)

/** This class represents the directory of all plug-in classes known to the system.
	The plug-in classes are grouped by super class id. The set of classes in each 
	superclass group can be accessed. For each plug-in class, its ClassDesc can be 
	then retrieved.
	All plug-in classes must have one of the super class ids pre-defined in the 3ds Max SDK.
	See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_super_class_ids.html">List of Super Class IDs</a>
	for a list of these super class ids.
	All methods of this class are implemented by the system. 
	Clients can retrive the sole instance of ClassDirectory via DllDir::ClassDir() or
	via ClassDirectory::GetInstance()
	\sa  Class DllDir, Class SubClassList, Class Class_ID, Class ClassDesc. 
*/
class ClassDirectory : public MaxSDK::Util::Noncopyable 
{
public:
	/** Retrieves the sole instance of class ClassDirectory. */
	CoreExport static ClassDirectory& GetInstance();

	/** Accesses a SubClassList instance that corresponds to all plug-in classes of a certain super class id.
		\param superClassID - The super class ID */
	CoreExport SubClassList* GetClassList(SClass_ID superClassID) const;

	/** Returns the number of SubClassList instances in the class directory. */
	CoreExport int Count() const;

	/** Accesses the i-th SubClassList.
		\param i - Specifies which sub class list to access. Must be in the range [0 and Count()-1]. */
	CoreExport SubClassList& operator[](int i) const;

	/** Returns a class descriptor that corresponds to a certain super and class id combination.
		\param superClassID - The super class id
		\param classID - The class id  */
	CoreExport ClassDesc* FindClass(SClass_ID superClassID, const Class_ID& classID) const;

	/** Returns a class entry that corresponds to a certain super and class id combination.
		\param superClassID - The super class id 
		\param classID - The class id */
	CoreExport ClassEntry* FindClassEntry(SClass_ID superClassID, const Class_ID& classID) const;

	/** \internal Registers a class descriptor with the class directory.
		\param ClassDesc* cdesc - Pointer to the class descriptor. 
		If cdesc replaces an existent class descriptor the system will broadcast a notification:
		\code
		BroadcastNotification(NOTIFY_CLASSDESC_REPLACED, &cdr);
		\endcode
		\param int dllNum - The index of the DllDesc hosting the class descriptor within DllDir
		\param int index - The index 
		\return 0 if class already exists, -1 if superclass was unknown and 1 if the class added successfully */
	CoreExport int AddClass(ClassDesc* cdesc, int dllNum, int index, bool load);

	/** \internal Deletes a class descriptor from the class directory.
		\return Returns 1 if successful, or -1 if not. */
	CoreExport int DeleteClass(ClassDesc* cdesc);

	/** Allows for providing additional UI related information pertaining to a superclass.
		Currently this includes a descriptive string, a color, and a method which
		draws a representative image in a Windows DC.
		\param superClassID - The super class id the info pertains to
		\param uiInfo - A pointer to a SClassUIInfo object that carries the information
		\return - Returns true if successful or false if the superclass was not found. */
	CoreExport bool SetUIInfoForSClass(SClass_ID superClassID, SClassUIInfo* uiInfo);

	/** Retrieves additional UI related information for a given superclass.
		\param superClassID - The super class id
		\return - Returns NULL if the superclass was not found or if no superclass 
		information was assigned. */
	CoreExport SClassUIInfo* GetUIInfoForSClass(SClass_ID superClassID) const;

	/** \internal Constructor */
	ClassDirectory();
	/** \internal Destructor */
	~ClassDirectory();

private:
	class ClassDirectoryImpl;
	MaxSDK::Util::AutoPtr<ClassDirectoryImpl> mImpl;

	/** \internal Adds a new Super Class ID to the list maintained by the class directory.*/
	void AddSuperClass(SClass_ID superClassID);
};

/** Represents the plug-in DLL "directory", which is a registry of every plug-in DLL loaded by 3ds Max.
	It also encapsulates the ClassDirectory which is a registry of all classes 
	implemented in the plug-in DLLs.
	The following diagram shows the relationship between the classes that make up
	the DllDir structure. The lines in the diagram indicate how the objects
	are accessed. Class DllDir provides access to a list of DllDesc objects. Each of
	these has a list of ClassDesc objects. Class ClassDirectory provides access to 
	the a table of SubClassList objects which are grouped by super class ID. 
	Each SubClassList has a series of ClassEntry objects. The ClassEntry object 
	provides information about the plug-in classes (some of the same information as 
	the class descriptor, usage counts, etc.). \n  
	\n \image html "dlldir_wmf.gif"
	DllDir is a singleton; its sole instance can be retrieved by DllDir::GetInstance() 
	or Interface::GetDllDir().
	All methods of this class are implemented by the system.  
	\sa  Class DllDesc, Class ClassDirectory, Class ClassDesc, Class ClassEntry, Class Interface.
*/
class DllDir : public MaxSDK::Util::Noncopyable
{
	public:
		/** Retrieves the sole instance of class DllDir. */
		CoreExport static DllDir& GetInstance();

		/* Destructor */
		virtual  ~DllDir() = 0;

		/** Returns the number of plug-in DLLs currently loaded. */
		CoreExport int Count() const;

		/** Returns a reference to the i-th DllDesc that represents a loaded plug-in dll
		 \param i - Zero based index of the loaded plug-in dll descriptors */
		CoreExport const DllDesc& operator[](int i) const;

		/** Returns a reference to the i-th DllDesc that represents a loaded plug-in dll
			\param i - Zero based index of the loaded plug-in dll descriptors */
		CoreExport const DllDesc& GetDllDescription(int i) const;

		/** \name Methods for loading plug-in dlls
			\brief The following methods load plug-in dlls. In most cases plug-ins 
			do not need to call these methods since 3ds Max will automatically attempt 
			to load all plug-in dlls found in the "stdplugs" folder and in all other 
			folders specified by the user in Configure System Paths > 3rd Party Plug-ins.
			Note that when 3ds Max attempts to load its plug-ins during it's startup, 
			it will defer as many plug-ins as possible. 
			One case in which a plug-in needs to call one of these methods is when it 
			has a run-time dependency on special functionality exposed by another plug-in 
			dll and it wants to make sure that the plug-in dll was not defer loaded by 3ds Max.
			Note that creating an instance of a plug-in object residing in a defer loaded 
			plug-in dll will result in 3ds Max loading that plug-in dll automatically.	
			The client of the plug-in instance does not need to explicitly load the plug-in dll.
			For more details on plug-in defer loading see the topic called 
			"Defer loading of plug-ins" in the 3ds Max SDK Programmer's Guide.
		*/
		//@{
		/** Loads the specified plug-in dlls from all the plug-in folders that 3ds Max knows about.
			This method loads the specified plug-ins from the "stdplugs" and all the other 
			folders specified in the Configure System Paths > 3rd Party Plug-ins list.
			\param pluginWildcard - String that specifies via wildcard characters (* and ?) 
			which plug-in dlls to load.
			\param forceLoadDeferrablePluginDlls - If true the plug-in dll is guaranteed not 
			to be defer loaded, otherwise the loading of the plug-in dll might be deferred to a later time.
			For more information see the topic called "Defer loading of plug-ins" in the 
			3ds Max SDK Programmer's Guide 
			\return  Nonzero if successful; zero on error. */
		CoreExport int LoadDlls(const MCHAR* pluginWildcard, bool forceLoadDeferrablepluginDlls = false);

		/** Loads plug-in DLLs from the specified directory.
			The default behaviour of this method is to defer the loading of a plug-in if possible.
			\param pluginPath - String that specifies a valid path to load plug-in DLLs from.
			\param pluginWildcard - String that specifies via wildcard characters (* and ?) which 
			plug-in dlls to load.
			\param forceLoadDeferrablePluginDlls - If true the plug-in dll is guaranteed not to be defer loaded,
			otherwise the loading of the plug-in dll might be deferred to a later time.
			For more information see the topic called "Defer loading of plug-ins" in the 
			3ds Max SDK Programmer's Guide
			\return  Nonzero if successful; zero on error. */
		CoreExport int LoadDllsFromDir(const MCHAR* pluginPath, const MCHAR* pluginWildcard, bool forceLoadDeferrablePluginDlls = false);

		/** Loads a given plug-in DLL.
			\param pluginDllFileName - The path and name of the plug-in dll to load
			\param forceLoadDeferrablePluginDlls - If true the plug-in dll is guaranteed 
			not to be defer loaded,
			otherwise the loading of the plug-in dll might be deferred to a later time.
			For more information see the topic called "Defer loading of plug-ins" in the 
			3ds Max SDK Programmer's Guide
			\return - true if the plug-in was loaded successfully */
		CoreExport bool LoadADll(const MCHAR* pluginDllFileName, bool forceLoadDeferrablePluginDlls);
	 //@}

		/** Returns a reference to the ClassDirectory. */
		CoreExport ClassDirectory& ClassDir() const;

		/** Returns the DllDesc that contains the definition of a class with the specified class and superclass IDs.
			\param superClassId The super class ID.
			\param classId The class ID.
			\return The DllDesc object that owns the ClassDesc associated with this ID pairing, or NULL if no such DllDesc exists. */
		CoreExport const DllDesc* GetDllDescriptionForClass(SClass_ID superClassId, const Class_ID& classId);

		/** Returns the DllDesc index within the DLL Directory (DllDir) for a given full path. 
			\param aFullPath A fully-qualified path object to the dll represented by the desired DllDesc object 
			\return the index to the DllDesc or -1 if not found */
		CoreExport int FindDllDescFromDllPath(const MaxSDK::Util::Path& aFullPath) const;

		/** Returns the DllDesc index within the DLL Directory (DllDir) for a given module handle.
			\param hInst The module handle used to identify a DllDesc
			\return the index to the DllDesc or -1 if not found */
		CoreExport int FindDllDescFromHInstance(const HINSTANCE hInst) const;

		/** \deprecated Deprecated as of 3ds Max 2012. Unloading plug-in DLLs is not 
		safe and therefore not supported. */
		MAX_DEPRECATED void UnloadAllDlls();

	protected:
		// Constructor
		DllDir();
		class DllDirImpl;
		MaxSDK::AutoPtr<DllDirImpl> mImpl;

	private:
		bool LoadADllProxy(const MaxSDK::Util::Path& filePath, const MCHAR* description, const FILETIME& timeLastWrite);

};

