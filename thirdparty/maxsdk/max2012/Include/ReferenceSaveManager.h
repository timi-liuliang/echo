/*******************************************************************************
Copyright 2010 Autodesk, Inc.  All rights reserved. 
This computer source code and related instructions and comments are the unpublished 
confidential and proprietary information of Autodesk, Inc. and are protected under 
applicable copyright and trade secret law.  They may not be disclosed to, copied 
or used by any third party without the prior written consent of Autodesk, Inc.
*******************************************************************************/

#pragma once
#include "ref.h"

//! \brief Used to specify and enumerate the save reference hierarchy, if different from the normal reference hierarchy.
/*! A ReferenceMaker can add, remove, or replace the contents of its "reference slots" in the context of
scene files saves. The primary use of this ability is to support Save To Previous, particularly when
converting a plugin from ParamBlock to ParamBlock2. When performing a Save to Previous, instead of storing
the ParamBlock2 instance, the plugin needs to store a ParamBlock instance. At the beginning of a scene file
save, the virtual ReferenceMaker::SpecifySaveReferences() method is called. If a plugin needs to store an
alternative reference hierarchy, the plugin would specify the changes from the normal reference hierarchy 
using the supplied ReferenceSaveManager. See ReferenceMaker::SpecifySaveReferences for an example implementation.

If an Add or Replace reference slot operation is performed, and a non-NULL ReferenceTarget is supplied, the 
ReferenceSaveManager will hold a reference to the ReferenceTarget. This reference will be dropped at the end
of the scene file save.

The Add/Remove/Replace reference slot operation calls register actions with the ReferenceSaveManager, and these
actions are processed in the ReferenceMaker::SpecifySaveReferences() implementation. Invalid reference slot indices
will not be detected until the ReferenceMaker::SpecifySaveReferences() implementation is executed, and will result in 
return value of false from that implementation.

If a plugin needs to access the save reference hierarchy, it would do so through the ReferenceSaveManager. 
The most common case of this would be in a SaveEnum(). For example:

\code
	void Scene::SaveEnum(SaveEnumProc& sep, BOOL isNodeCall) {
		if (sep.terminate(this))
			return;

		if(GetCustAttribContainer())
			GetCustAttribContainer()->SaveEnum(sep);

		ReferenceSaveManager& referenceSaveManager = GetReferenceSaveManager();
		DbgAssert(referenceSaveManager.ReadyForSave());
		int numSaveRefs = referenceSaveManager.NumRefs();
		for (int i=0; i < numSaveRefs; i++) 
		{	
			ReferenceTarget* refTarg = referenceSaveManager.GetReference(i);
			if (refTarg)
				refTarg->SaveEnum(sep, (i == SCENE_REF_ROOTNODE) ? TRUE : FALSE);
		}

		int numSaveIndirectRefs = referenceSaveManager->NumIndirectRefs();
		for (int j=0; j < numSaveIndirectRefs; j++)
		{
			ReferenceTarget* refTarg = referenceSaveManager->GetIndirectReference(j);
			if (refTarg)
				refTarg->SaveEnum(sep);
		}

		sep.proc(this); // process bottom up in ref hierarchy
	}
\endcode

If the ReferenceSaveManager is used to enumerate the save reference hierarchy before it has been prepared by a call to 
ReferenceMaker::SpecifySaveReferences(), the normal reference hierarchy will be exposed. That is, 
ReferenceSaveManager::NumRefs() will be routed to ReferenceMaker::NumRefs(), 
ReferenceSaveManager::GetReference() will be routed to ReferenceMaker::GetReference(), etc.

This interface is implemented by 3ds Max. Plug-ins do not need to implement it.

\see Interface::EnumAuxFiles, ReferenceMaker::SpecifySaveReferences, ReferenceMaker::EnumAuxFiles
*/
class ReferenceSaveManager : public MaxSDK::Util::Noncopyable
{
public:
	//! \brief Used to prepare a save reference hierarchy.
	/*! Before accessing the save reference hierarchy for a ReferenceMaker, the ReferenceMaker may need to prepare
	the save reference hierarchy. This is done by calling SpecifySaveReferences on the ReferenceMaker and all of its 
	save references. An instance of the PreSave class passed to the SaveEnum method will perform this preparation. 
	Plugins will not normally need to perform these actions to prepare the save reference hierarchy for a ReferenceMaker
	as the system will have already done so at the beginning of the save process. The only time a plugin would need 
	to do this is if it is initiating a call to SaveEnum on the reference maker (i.e., the SaveEnum call is not in a 
	SaveEnum implementation). For example:

	/code
		// this code is not within a SaveEnum implemention, so the save ref hierarchy has not already been created
		// initialize: build save ref hierarchy
		ReferenceSaveManager::PreSave preSaveEnumProc;
		maker->SaveEnum(preSaveEnumProc, FALSE);

		SimpleSaveEnumerator saver;
		maker->SaveEnum(saver);

		// finalize: clear save ref hierarchy
		ReferenceSaveManager::PostSave postSaveEnumProc;
		maker->SaveEnum(preSaveEnumProc, FALSE);
	/endcode

	As seen in the code example, after completing the SaveEnum calls, you will also need to perform an additional 
	SaveEnum passing a PostSave SaveEnumProc. This proc tears down the save reference hierarchy.
	*/
	class PreSave: public SaveEnumProc {
	public:
		CoreExport void proc(ReferenceMaker *rmaker); /// implemented by system
		CoreExport int terminate(ReferenceMaker *rmaker);  /// implemented by system
	};
	//! \brief Used to tear down a save reference hierarchy.
	/*! Before accessing the save reference hierarchy for a ReferenceMaker, the ReferenceMaker may need to prepare
	the save reference hierarchy. This is done by calling SpecifySaveReferences on the ReferenceMaker and all of its save
	references. An instance of the PreSave class passed to the SaveEnum method will perform this preparation. After
	the actions ReferenceMaker are complete, an instance of the PostSave class are passed to the SaveEnum method
	to tear down the save reference hierarchy. An example of this can be seen in the code example for PreSave.
	*/
	class PostSave: public SaveEnumProc {
	public:
		CoreExport void proc(ReferenceMaker *rmaker); /// implemented by system
		CoreExport int terminate(ReferenceMaker *rmaker);  /// implemented by system
	};
	//! \brief Returns the number of direct references in the save reference hierarchy.
	/*! The number of direct references in the save reference hierarchy is based on the number of references held 
	by the ReferenceMaker and the Add/Remove reference slot operations. If no Add/Remove reference slot operations
	were performed, or if the save reference hierarchy has not been prepared, the value returned is the result 
	from ReferenceMaker::NumRefs().
	\return     Returns the number of direct references in the save reference hierarchy.
	*/
	virtual int NumRefs() = 0;
	//! \brief Returns the i'th direct reference in the save reference hierarchy.
	/*! Returns the i'th direct reference in the save reference hierarchy. If no Add/Remove/Replace reference 
	slot operations was performed on the slot, or if the save reference hierarchy has not been prepared, the value 
	returned is the result from ReferenceMaker::GetReference using the appropriate index value. The index value 
	may be different than 'i' if Add/Remove reference slot operations were performed.
	\param  i Which direct reference in the save reference hierarchy to return
	\param  usePersistenceTests If true, a NULL value will be returned if the reference does not
	pass the persistence tests. A reference is persistent if ReferenceMaker::IsRealDependency or
	ReferenceMaker::ShouldPersistWeakRef returns true for the reference, or if the reference was specified
	using AddReferenceSlot or ReplaceReferenceSlot.
	Note that even if a reference is not persistent for a given ReferenceMaker, it may be persistent on another
	ReferenceMaker, and thus would be saved to the scene file. The behavior of the 3ds Max scene file load is that 
	if the ReferenceTarget is loaded, it is set as a reference to all ReferenceMakers that held a 
	reference to it, regardless of whether it was considered persistent on that ReferenceMakers or not.
	In most cases, and particularly in SaveEnum and EnumAuxFiles implementations, usePersistenceTests would be 
	true - you only want to enumerate references that are guaranteed to be saved. If for some reason you want to 
	look at all references, including those not guaranteed to be saved, then usePersistenceTests would be false.
	You can perform additional tests on the references using IsRealDependency and ShouldPersistWeakReference
	to see whether the reference would be persistent.
	\return Returns the i'th direct reference in the save reference hierarchy.
	*/
	virtual ReferenceTarget* GetReference(int i, bool usePersistenceTests = true) = 0;
	//! \brief Returns whether this is a "real" (strong) dependency or not.
	/*! Returns true if the reference is considered a strong dependency. If the reference was specified
	by a Add/Replace operation, it is considered a strong dependency. Otherwise, the call is passed to
	ReferenceMaker::IsRealDependency, and its value is returned.
	\param  theRef A pointer to the reference target.
	\return true if the reference dependency is "real". Otherwise it returns false.
	*/
	virtual bool IsRealDependency(ReferenceTarget *theRef) = 0;
	//! \brief Returns whether a weak reference is to be persisted on a partial load or save.
	/*! Returns true if the reference should be persisted if it is not a strong dependency. The call is 
	passed to ReferenceMaker::ShouldPersistWeakRef, and its value is returned.
	\param  theRef A pointer to the reference target.
	\return Whether to force the load/save of the weak reference if this reference maker is saved.
	*/
	virtual bool ShouldPersistWeakReference(ReferenceTarget *theRef) = 0;
	//! \brief Returns the number of indirect references in the save reference hierarchy.
	/*! The number of indirect references in the save reference hierarchy is based on the number of indirect references 
	held by the ReferenceMaker and the Add/Remove indirect reference slot operations. If no Add/Remove indirect reference 
	slot operations were performed, or if the save reference hierarchy has not been prepared, the value returned 
	is the result from IIndirectReferenceMaker::NumIndirectRefs().
	\return Returns the number of direct references in the save reference hierarchy.
	*/
	virtual int NumIndirectRefs() = 0;
	//! \brief Returns the i'th indirect reference in the save reference hierarchy.
	/*! Returns the i'th indirect reference in the save reference hierarchy. If no Add/Remove/Replace indirect reference 
	slot operations was performed on the slot, or if the save reference hierarchy has not been prepared, the value 
	returned is the result from IIndirectReferenceMaker::GetIndirectReference using the appropriate index value. 
	The index value may be different than 'i' if Add/Remove indirect reference slot operations were performed.
	\param  i Which indirect reference in the save reference hierarchy to return
	\param  usePersistenceTests If true, a NULL value will be returned if the reference does not
	pass the persistence tests. A reference is persistent if IIndirectReferenceMaker::ShouldPersistIndirectRef
	returns true for the reference, or if the reference was specified using AddIndirectReferenceSlot 
	or RemoveIndirectReferenceSlot.
	Note that even if a reference is not persistent for a given ReferenceMaker, it may be persistent on another
	ReferenceMaker, and thus would be saved to the scene file. The behavior of the 3ds Max scene file load is that 
	if the ReferenceTarget is loaded, it is set as a reference to all ReferenceMakers that held a 
	reference to it, regardless of whether it was considered persistent on that ReferenceMakers or not.
	In most cases, and particularly in SaveEnum and EnumAuxFiles implementations, usePersistenceTests would be 
	true - you only want to enumerate references that are guaranteed to be saved. If for some reason you want to 
	look at all references, including those not guaranteed to be saved, then usePersistenceTests would be false.
	You can perform additional tests on the references using ShouldPersistIndirectReference
	to see whether the reference would be persistent.
	\return Returns the i'th indirect reference in the save reference hierarchy.
	*/
	virtual ReferenceTarget* GetIndirectReference(int i, bool usePersistenceTests = true) = 0;
	//! \brief Returns whether the indirect reference is to be persisted on a partial load or save.
	/*! Returns true if the indirect reference is to be persisted on a partial load or save. If the indirect reference 
	was specified by a Add/Replace operation, it should be persisted and the method will return true. Otherwise, the call 
	is passed to IIndirectReferenceMaker::ShouldPersistIndirectRef, and it's value is returned.
	\param  theRef - A pointer to the reference target.
	\return Whether to force the load/save of the indirect reference if this reference maker is saved.
	*/
	virtual bool ShouldPersistIndirectReference(ReferenceTarget *theRef) = 0;
	//! \brief Adds a reference slot to the save reference hierarchy.
	/*! Inserts a reference slot at the specified index value. A GetReference call on that slot will
	return the specified ReferenceTarget. The ReferenceSaveManager will hold a reference to the specified
	ReferenceTarget until the end of the save operation.
	\param  insertAt The index to insert a reference slot at.
	\param  theRef The reference to store in the reference slot.
	*/
	virtual void AddReferenceSlot(int insertAt, ReferenceTarget* theRef) = 0;
	//! \brief Replaces the contents of a reference slot in the save reference hierarchy.
	/*! Specifies the ReferenceTarget value to return for a GetReference call on that slot. The 
	ReferenceSaveManager will hold a reference to the specified ReferenceTarget until the end of the save 
	operation.
	\param  which The index of the reference slot.
	\param  theRef The reference to store in the reference slot.
	*/
	virtual void ReplaceReferenceSlot(int which, ReferenceTarget* theRef) = 0;
	//! \brief Removes a reference slot from the save reference hierarchy.
	/*! Deletes a reference slot at the specified index value.
	\param  which The index of the reference slot to delete
	*/
	virtual void RemoveReferenceSlot(int which) = 0;
	//! \brief Adds an indirect reference slot to the save reference hierarchy.
	/*! Inserts an indirect reference slot at the specified index value. A GetIndirectReference call 
	on that slot will return the specified ReferenceTarget. The ReferenceSaveManager will hold a reference to 
	the specified ReferenceTarget until the end of the save operation.
	\param  insertAt The index to insert a indirect reference slot at.
	\param  theRef The reference to store in the indirect reference slot.
	*/
	virtual void AddIndirectReferenceSlot(int insertAt, ReferenceTarget* theRef) = 0;
	//! \brief Replaces the contents of an indirect reference slot in the save reference hierarchy.
	/*! Specifies the ReferenceTarget value to return for a GetIndirectReference call on that slot. The 
	ReferenceSaveManager will hold a reference to the specified ReferenceTarget until the end of the save 
	operation.
	\param  which The index of the indirect reference slot.
	\param  theRef The reference to store in the indirect reference slot.
	*/
	virtual void ReplaceIndirectReferenceSlot(int which, ReferenceTarget* theRef) = 0;
	//! \brief Removes an indirect reference slot from the save reference hierarchy.
	/*! Deletes an indirect reference slot at the specified index value.
	\param  which The index of the indirect reference slot to delete
	*/
	virtual void RemoveIndirectReferenceSlot(int which) = 0;
	//! \brief Specifies a ReferenceTarget that is to be saved instead of the ReferenceTarget represented by this instance of ReferenceSaveManager.
	/*! In some cases, instead of storing the current ReferenceTarget, you want another ReferenceTarget to be stored. An example of 
	this is when doing a Save To Previous, and the previous version of 3ds Max expects a ReferenceTarget of a different class. In
	this case, the ReferenceTarget represented by this instance of ReferenceSaveManager would implement the SpecifySaveReferences method, 
	and in that implementation create an instance of the old class, configure that instance as needed, and then specify to save that
	instance using this method. The ReferenceSaveManager will hold a reference to the specified ReferenceTarget until the end of 
	the save operation.
	The replacement ReferenceTarget specified by the most-derived class is used as the replacement ReferenceTarget. A reference will be held 
	to all replacement ReferenceTargets specified, and those references will be dropped at the end of the file save. 
	If this method is called, the ReferenceTarget represented by this instance of ReferenceSaveManager will not be saved. While calls to
	the various direct/indirect reference methods in this class are still valid, they have no affect on what is saved since they operate
	on the ReferenceTarget represented by this instance of ReferenceSaveManager.
	\param[in]  theRef The replacement reference to store to the scene file. A NULL value is a valid replacement reference pointer.

	The following is an example usage, where all Marble instance are replaced with Checker instances
	/code
		bool Marble::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
		{
			ReferenceTarget* newRef = (ReferenceTarget*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, Class_ID(CHECKER_CLASS_ID,0));
			referenceSaveManager.SetReplacementReferenceTarget(newRef);
			return Tex3D::SpecifySaveReferences(referenceSaveManager);
		}
	/endcode
	*/ 
	virtual void SetReplacementReferenceTarget(ReferenceTarget* theRef) = 0;
	//! \brief Returns whether a replacement ReferenceTarget was specified.
	//! \return Returns true if SetReplacementReferenceTarget() was called.
	virtual bool ReplacementReferenceTargetSpecified() const = 0;
	//! \brief Returns the ReferenceTarget that is to be saved instead of the ReferenceTarget represented by this instance of the ReferenceSaveManager.
	/*! This method should not be called unless ReplacementReferenceTargetSpecified() returns true since this method will return NULL if
	SetReplacementReferenceTarget has not been called.
	\return Returns the replacement ReferenceTarget.
	*/
	virtual ReferenceTarget* GetReplacementReferenceTarget() = 0;
	//! \brief Detect whether SpecifySaveReferences() was called on the ReferenceMaker.
	/*! Before the save reference hierarchy can be accessed, SpecifySaveReferences() needs to be called on
	the ReferenceMaker to prepare the save reference hierarchy.
	\return Returns true if SpecifySaveReferences() was called on the ReferenceMaker.
	*/
	virtual bool ReadyForSave() const = 0;
	//! \brief Clears the save reference hierarchy.
	/*! Clears the save reference hierarchy, dropping references to any ReferenceTargets
	specified by Add/Replace direct/indirect reference slot operations.
	*/
	virtual void Clear() = 0;
	//! \brief Required virtual dtor.
	virtual ~ReferenceSaveManager() {}
};

