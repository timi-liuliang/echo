#pragma once

#include "ifnpub.h"

#define PATCHDEFORMPW_CLASS_ID	Class_ID(0x22b7bd09, 0x673ac5be)
#define PATCHDEFORMPW_INTERFACE Interface_ID(0xDE21A34f, 0x8A43E3E3)

class IPatchDeformPWMod : public FPMixinInterface
{

public:

	enum 
	{ 
		patchdeformpw_params /// This is the parameter rollup fields
	};

	enum 
	{ 
		pb_patch,		/// this is the patch that is doing the deformation
		pb_autoupdate,	/// Not used anymore
		pb_samplerate,	/// this is how far down the system will sample to find hits on the patch
						/// the higher the value the more accurate it will be
		pb_patchlist	/// to be used later
	};

	FPInterfaceDesc* GetDesc(); 

	/// Resample()
	/// This forces the modifier to resample itself. This will force the system to resample the patch
	virtual void Resample()=0;

	/// int GetNumberOfPoints(INode *node)
	/// This returns the number of points that are deformed
	virtual int GetNumberOfPoints(INode *node)=0;

	/// int Point3 GetPointUVW(INode *node, int index)
	/// This returns the closest UVW point on the patch to this point
	///		INode *node this is the node that owns the modifier so we can get the right local data
	///		int index this is the index of the point you want to lookup
	virtual Point3 GetPointUVW(INode *node, int index)=0;

	/// int Point3 GetPointUVW(INode *node, int index)
	/// This returns the local space point of the deforming point before deformation
	///		INode *node this is the node that owns the modifier so we can get the right local data
	///		int index this is the index of the point you want to lookup
	virtual Point3 GetPointLocalSpace(INode *node, int index)=0;

	/// int Point3 GetPointPatchSpace(INode *node, int index)
	/// This returns the point in the space of the patch of the deforming point before deformation
	///		INode *node this is the node that owns the modifier so we can get the right local data
	///		int index this is the index of the point you want to lookup
	virtual Point3 GetPointPatchSpace(INode *node, int index)=0;

	/// int int GetPointPatchIndex(INode *node, int index)
	/// This returns closest patch to this point
	///		INode *node this is the node that owns the modifier so we can get the right local data
	///		int index this is the index of the point you want to lookup
	virtual int GetPointPatchIndex(INode *node, int index)=0;

};


