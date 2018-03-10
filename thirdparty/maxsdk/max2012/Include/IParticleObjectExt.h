/**********************************************************************
 *<
	FILE: IParticleObjectExt.h

	DESCRIPTION: API extension for a ParticleObject class
				the interface is used by particles systems and 
				particle groups. The time parameter does not
				used in property access since the particle system
				gives out property of the current state. You can
				change the state by using UpdateParticles method
				with the time specified.

	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-30-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "ifnpub.h"
#include "maxtypes.h"
#include "point3.h"
#include "quat.h"
// forward declarations
class View;

// interface ID
#define PARTICLEOBJECTEXT_INTERFACE Interface_ID(0x66c2429a, 0x38d60a0b) 

#define GetParticleObjectExtInterface(obj) ((IParticleObjectExt*)(obj)->GetInterface(PARTICLEOBJECTEXT_INTERFACE)) 
#pragma warning(push)
#pragma warning(disable:4100)
class IParticleObjectExt : public FPMixinInterface
{
public:

	// function IDs
	enum {	kUpdateParticles,
			kSetEveryStepUpdateScriptLine,
			kSetEveryStepUpdateScriptFile,
			kSetFinalStepUpdateScriptLine,
			kSetFinalStepUpdateScriptFile,
			kExecuteEveryStepUpdateScript,
			kExecuteFinalStepUpdateScript,
			kGetUpdateTime,
			kGetUpdateInterval,
			kNumParticles,
			kNumParticlesGenerated,
			kAddParticle,
			kAddParticles,
			kDeleteParticle,
			kDeleteParticles,
			kGetParticleBornIndex,
			kHasParticleBornIndex,
			kGetParticleGroup,
			kGetParticleIndex,
			kGetCurrentParticleIndex,
			kGetCurrentParticleBornIndex,
			kSetCurrentParticleIndex,
			kSetCurrentParticleBornIndex,
			kGetParticleAgeByIndex,
			kGetParticleAgeByBornIndex,
			kSetParticleAgeByIndex,
			kSetParticleAgeByBornIndex,
			kGetParticleAge,
			kSetParticleAge,
			kGetParticleLifeSpanByIndex,
			kGetParticleLifeSpanByBornIndex,
			kSetParticleLifeSpanByIndex,
			kSetParticleLifeSpanByBornIndex,
			kGetParticleLifeSpan,
			kSetParticleLifeSpan,
			kGetParticleGroupTimeByIndex,
			kGetParticleGroupTimeByBornIndex,
			kSetParticleGroupTimeByIndex,
			kSetParticleGroupTimeByBornIndex,
			kGetParticleGroupTime,
			kSetParticleGroupTime,
			kGetParticlePositionByIndex,
			kGetParticlePositionByBornIndex,
			kSetParticlePositionByIndex,
			kSetParticlePositionByBornIndex,
			kGetParticlePosition,
			kSetParticlePosition,
			kGetParticleSpeedByIndex,
			kGetParticleSpeedByBornIndex,
			kSetParticleSpeedByIndex,
			kSetParticleSpeedByBornIndex,
			kGetParticleSpeed,
			kSetParticleSpeed,
			kGetParticleOrientationByIndex,
			kGetParticleOrientationByBornIndex,
			kSetParticleOrientationByIndex,
			kSetParticleOrientationByBornIndex,
			kGetParticleOrientation,
			kSetParticleOrientation,
			kGetParticleSpinByIndex,
			kGetParticleSpinByBornIndex,
			kSetParticleSpinByIndex,
			kSetParticleSpinByBornIndex,
			kGetParticleSpin,
			kSetParticleSpin,
			kGetParticleScaleByIndex,
			kGetParticleScaleByBornIndex,
			kSetParticleScaleByIndex,
			kSetParticleScaleByBornIndex,
			kGetParticleScale,
			kSetParticleScale,
			kGetParticleScaleXYZByIndex,
			kGetParticleScaleXYZByBornIndex,
			kSetParticleScaleXYZByIndex,
			kSetParticleScaleXYZByBornIndex,
			kGetParticleScaleXYZ,
			kSetParticleScaleXYZ,
			kGetParticleTMByIndex,
			kGetParticleTMByBornIndex,
			kSetParticleTMByIndex,
			kSetParticleTMByBornIndex,
			kGetParticleTM,
			kSetParticleTM,
			kGetParticleSelectedByIndex,
			kGetParticleSelectedByBornIndex,
			kSetParticleSelectedByIndex,
			kSetParticleSelectedByBornIndex,
			kGetParticleSelected,
			kSetParticleSelected,
			kGetParticleShapeByIndex,
			kGetParticleShapeByBornIndex,
			kSetParticleShapeByIndex,
			kSetParticleShapeByBornIndex,
			kGetParticleShape,
			kSetParticleShape,
			kSetGlobalParticleShape,
	}; 

	// Function Map for Function Publish System 
	//***********************************

	BEGIN_FUNCTION_MAP

	VFN_2(kUpdateParticles,							UpdateParticles,				TYPE_INODE, TYPE_TIMEVALUE);
	VFN_1(kSetEveryStepUpdateScriptLine,			SetEveryStepUpdateScriptLine,	TYPE_STRING);
	VFN_1(kSetEveryStepUpdateScriptFile,			SetEveryStepUpdateScriptFile,	TYPE_FILENAME);
	VFN_1(kSetFinalStepUpdateScriptLine,			SetFinalStepUpdateScriptLine,	TYPE_STRING);
	VFN_1(kSetFinalStepUpdateScriptFile,			SetFinalStepUpdateScriptFile,	TYPE_FILENAME);
	VFN_0(kExecuteEveryStepUpdateScript,			ExecuteEveryStepUpdateScript	);
	VFN_0(kExecuteFinalStepUpdateScript,			ExecuteFinalStepUpdateScript	);
	FN_0(kGetUpdateTime,		TYPE_TIMEVALUE,		GetUpdateTime					);
	VFN_2(kGetUpdateInterval,						GetUpdateInterval,				TYPE_TIMEVALUE_BR, TYPE_TIMEVALUE_BR);

	RO_PROP_FN(kNumParticles,				NumParticles,			TYPE_INT);
	RO_PROP_FN(kNumParticlesGenerated,		NumParticlesGenerated,	TYPE_INT);

	FN_0(kAddParticle,			TYPE_bool,	AddParticle				);
	FN_1(kAddParticles,			TYPE_bool,	AddParticles,			TYPE_INT);
	FN_1(kDeleteParticle,		TYPE_bool,	DeleteParticle,			TYPE_INDEX);
	FN_2(kDeleteParticles,		TYPE_bool,	DeleteParticles,		TYPE_INDEX, TYPE_INT);

	FN_1(kGetParticleBornIndex,	TYPE_INDEX,	GetParticleBornIndex,	TYPE_INDEX);
	FN_2(kHasParticleBornIndex,	TYPE_bool,	HasParticleBornIndex,	TYPE_INDEX, TYPE_INDEX_BR);
	FN_1(kGetParticleGroup,		TYPE_INODE,	GetParticleGroup,		TYPE_INDEX);
	FN_1(kGetParticleIndex,		TYPE_INDEX,	GetParticleIndex,		TYPE_INDEX);

	PROP_FNS(kGetCurrentParticleIndex,		GetCurrentParticleIndex,		kSetCurrentParticleIndex,		SetCurrentParticleIndex,	TYPE_INDEX);
	PROP_FNS(kGetCurrentParticleBornIndex,	GetCurrentParticleBornIndex,	kSetCurrentParticleBornIndex,	SetCurrentParticleBornIndex,TYPE_INDEX);

	FN_1(kGetParticleAgeByIndex,		TYPE_TIMEVALUE,	GetParticleAgeByIndex,			TYPE_INDEX);
	FN_1(kGetParticleAgeByBornIndex,	TYPE_TIMEVALUE,	GetParticleAgeByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticleAgeByIndex,						SetParticleAgeByIndex,			TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleAgeByBornIndex,					SetParticleAgeByBornIndex,		TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleAge, GetParticleAge, kSetParticleAge, SetParticleAge,			TYPE_TIMEVALUE);

	FN_1(kGetParticleLifeSpanByIndex,		TYPE_TIMEVALUE,	GetParticleLifeSpanByIndex,			TYPE_INDEX);
	FN_1(kGetParticleLifeSpanByBornIndex,	TYPE_TIMEVALUE,	GetParticleLifeSpanByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticleLifeSpanByIndex,						SetParticleLifeSpanByIndex,			TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleLifeSpanByBornIndex,					SetParticleLifeSpanByBornIndex,		TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleLifeSpan, GetParticleLifeSpan, kSetParticleLifeSpan, SetParticleLifeSpan,			TYPE_TIMEVALUE);

	FN_1(kGetParticleGroupTimeByIndex,		TYPE_TIMEVALUE,	GetParticleGroupTimeByIndex,	TYPE_INDEX);
	FN_1(kGetParticleGroupTimeByBornIndex,	TYPE_TIMEVALUE,	GetParticleGroupTimeByBornIndex,TYPE_INDEX);
	VFN_2(kSetParticleGroupTimeByIndex,						SetParticleGroupTimeByIndex,	TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleGroupTimeByBornIndex,					SetParticleGroupTimeByBornIndex,TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleGroupTime, GetParticleGroupTime, kSetParticleGroupTime, SetParticleGroupTime,	TYPE_TIMEVALUE);

	FN_1(kGetParticlePositionByIndex,		TYPE_POINT3,	GetParticlePositionByIndex,			TYPE_INDEX);
	FN_1(kGetParticlePositionByBornIndex,	TYPE_POINT3,	GetParticlePositionByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticlePositionByIndex,						SetParticlePositionByIndex,			TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticlePositionByBornIndex,					SetParticlePositionByBornIndex,		TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticlePosition, GetParticlePosition, kSetParticlePosition, SetParticlePosition,	TYPE_POINT3);

	FN_1(kGetParticleSpeedByIndex,		TYPE_POINT3,	GetParticleSpeedByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSpeedByBornIndex,	TYPE_POINT3,	GetParticleSpeedByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSpeedByIndex,						SetParticleSpeedByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleSpeedByBornIndex,					SetParticleSpeedByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleSpeed, GetParticleSpeed, kSetParticleSpeed, SetParticleSpeed,	TYPE_POINT3);

	FN_1(kGetParticleOrientationByIndex,		TYPE_POINT3,	GetParticleOrientationByIndex,		TYPE_INDEX);
	FN_1(kGetParticleOrientationByBornIndex,	TYPE_POINT3,	GetParticleOrientationByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleOrientationByIndex,						SetParticleOrientationByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleOrientationByBornIndex,					SetParticleOrientationByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleOrientation, GetParticleOrientation, kSetParticleOrientation, SetParticleOrientation,	TYPE_POINT3);

	FN_1(kGetParticleSpinByIndex,		TYPE_ANGAXIS,	GetParticleSpinByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSpinByBornIndex,	TYPE_ANGAXIS,	GetParticleSpinByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSpinByIndex,						SetParticleSpinByIndex,		TYPE_INDEX, TYPE_ANGAXIS);
	VFN_2(kSetParticleSpinByBornIndex,					SetParticleSpinByBornIndex,	TYPE_INDEX, TYPE_ANGAXIS);
	PROP_FNS(kGetParticleSpin, GetParticleSpin, kSetParticleSpin, SetParticleSpin,	TYPE_ANGAXIS);

	FN_1(kGetParticleScaleByIndex,			TYPE_FLOAT,	GetParticleScaleByIndex,		TYPE_INDEX);
	FN_1(kGetParticleScaleByBornIndex,		TYPE_FLOAT,	GetParticleScaleByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleScaleByIndex,						SetParticleScaleByIndex,		TYPE_INDEX, TYPE_FLOAT);
	VFN_2(kSetParticleScaleByBornIndex,					SetParticleScaleByBornIndex,	TYPE_INDEX, TYPE_FLOAT);
	PROP_FNS(kGetParticleScale, GetParticleScale, kSetParticleScale, SetParticleScale,	TYPE_FLOAT);
	FN_1(kGetParticleScaleXYZByIndex,		TYPE_POINT3,GetParticleScaleXYZByIndex,		TYPE_INDEX);
	FN_1(kGetParticleScaleXYZByBornIndex,	TYPE_POINT3,GetParticleScaleXYZByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleScaleXYZByIndex,					SetParticleScaleXYZByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleScaleXYZByBornIndex,				SetParticleScaleXYZByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleScaleXYZ, GetParticleScaleXYZ, kSetParticleScaleXYZ, SetParticleScaleXYZ,	TYPE_POINT3);

	FN_1(kGetParticleTMByIndex,			TYPE_MATRIX3,	GetParticleTMByIndex,		TYPE_INDEX);
	FN_1(kGetParticleTMByBornIndex,		TYPE_MATRIX3,	GetParticleTMByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleTMByIndex,						SetParticleTMByIndex,		TYPE_INDEX, TYPE_MATRIX3);
	VFN_2(kSetParticleTMByBornIndex,					SetParticleTMByBornIndex,	TYPE_INDEX, TYPE_MATRIX3);
	PROP_FNS(kGetParticleTM, GetParticleTM, kSetParticleTM, SetParticleTM,			TYPE_MATRIX3);

	FN_1(kGetParticleSelectedByIndex,		TYPE_bool,	GetParticleSelectedByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSelectedByBornIndex,	TYPE_bool,	GetParticleSelectedByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSelectedByIndex,					SetParticleSelectedByIndex,		TYPE_INDEX, TYPE_bool);
	VFN_2(kSetParticleSelectedByBornIndex,				SetParticleSelectedByBornIndex,	TYPE_INDEX, TYPE_bool);
	PROP_FNS(kGetParticleSelected, GetParticleSelected, kSetParticleSelected, SetParticleSelected,	TYPE_bool);

	FN_1(kGetParticleShapeByIndex,		TYPE_MESH,	GetParticleShapeByIndex,		TYPE_INDEX);
	FN_1(kGetParticleShapeByBornIndex,	TYPE_MESH,	GetParticleShapeByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleShapeByIndex,					SetParticleShapeByIndex,		TYPE_INDEX, TYPE_MESH);
	VFN_2(kSetParticleShapeByBornIndex,				SetParticleShapeByBornIndex,	TYPE_INDEX, TYPE_MESH);
	VFN_1(kSetGlobalParticleShape,					SetGlobalParticleShape,			TYPE_MESH);
	PROP_FNS(kGetParticleShape, GetParticleShape, kSetParticleShape, SetParticleShape,			TYPE_MESH);

	END_FUNCTION_MAP

	// Implemented by the Plug-In.
	// Since particles may have different motion, the particle system should supply speed information
	// on per vertex basis for a motion blur effect to be able to generate effect.
	// the method is not exposed in maxscript
	// returns true if the object supports the method
	// Parameters:
	//		TimeValue t
	//			The time to get the mesh vertices speed.
	//		INode *inode
	//			The node in the scene
	//		View& view
	//			If the renderer calls this method it will pass the view information here.
	//		Tab<Point3>& speed
	//			speed per vertex in world coordinates
	virtual bool GetRenderMeshVertexSpeed(TimeValue t, INode *inode, View& view, Tab<Point3>& speed)  { return false; }

	// Implemented by the Plug-In.
	// Particle system may supply multiple render meshes. If this method returns a positive number, 
	// then GetMultipleRenderMesh and GetMultipleRenderMeshTM will be called for each mesh, 
	// instead of calling GetRenderMesh. The method has a current time parameter which is not
	// the case with the NumberOfRenderMeshes method of GeomObject class
	// the method is not exposed in maxscript
	// Parameters:
	//		TimeValue t
	//			Time for the number of render meshes request.
	//		INode *inode
	//			The node in the scene
	//		View& view
	//			If the renderer calls this method it will pass the view information here.
	virtual int NumberOfRenderMeshes(TimeValue t, INode *inode, View& view) { return 0; }
	
	// For multiple render meshes, if it supports vertex speed for motion blur, this method must be implemented
	// Since particles may have different motion, the particle system should supply speed information
	// on per vertex basis for a motion blur effect to be able to generate effect.
	// the method is not exposed in maxscript
	// returns true if the particular render mesh supports the method
	// Parameters:
	//		TimeValue t
	//			The time to get the mesh vertices speed.
	//		INode *inode
	//			The node in the scene
	//		View& view
	//			If the renderer calls this method it will pass the view information here.
	//		int meshNumber
	//			Specifies which of the multiple meshes is being asked for.
	//		Tab<Point3>& speed
	//			speed per vertex in world coordinates
    virtual	bool GetMultipleRenderMeshVertexSpeed(TimeValue t, INode *inode, View& view, int meshNumber, Tab<Point3>& speed) { return false; }

	// Implemented by the Plug-In.
	// This method is called so the particle system can update its state to reflect 
	// the current time passed.  This may involve generating new particle that are born, 
	// eliminating old particles that have expired, computing the impact of collisions or 
	// force field effects, and modify properties of the particles.
	// Parameters:
	//		TimeValue t
	//			The particles should be updated to reflect this time.
	//		INode *node
	//			This is the emitter node.
	// the method is not exposed in maxscript
	virtual void UpdateParticles(INode *node, TimeValue t) { ; }

	// Implemented by the Plug-in
	// This method defines a maxscript to be executed after every step of particle system
	// update integration. When a particle system updates itself from frame 0 to frame 10,
	// it goes through a series of integration steps: frame 1, frame 2, frame 3, ... or even
	// smaller steps up to the frame 10. The script will be called after each integration
	// step to update particle properties. Name of the file is stored by the particle system
	// Parameters:
	//		MCHAR *script
	//			The maxscript
	//		MCHAR *file
	//			The file name for the script file.
	virtual void SetEveryStepUpdateScriptLine(MCHAR* script) { ; }
	virtual void SetEveryStepUpdateScriptFile(MCHAR* file) { ; }

	// Implemented by the Plug-in
	// This method defines a maxscript to be executed after last step of particle system
	// update integration. When a particle system updates itself from frame 0 to frame 10,
	// it goes through a series of integration steps: frame 1, frame 2, frame 3, ... or even
	// smaller steps up to the frame 10. The script will be called after the last integration
	// step at frame 10. If you don't need to update particle properties in intermediate steps
	// then use this method, i.e. if you can define particle position given the current
	// frame without speed data.
	// You can use both SetUpdateScript and SetFinalUpdateScript methods at the same time.
	// Name of the file is stored by the particle system.
	// Parameters:
	//		MCHAR *script
	//			The maxscript
	//		MCHAR *file
	//			The file name for the script file.
	virtual void SetFinalStepUpdateScriptLine(MCHAR* script) { ; }
	virtual void SetFinalStepUpdateScriptFile(MCHAR* file) { ; }

	// Implemented by the Plug-in
	// This methods envokes execution of update scripts
	// -- for internal use only (may-21-2002)
	virtual void ExecuteEveryStepUpdateScript() { ; }
	virtual void ExecuteFinalStepUpdateScript() { ; }

	// Implemented by the Plug-in
	// Use this method to retrieve time of the current update step. The update time maybe unrelated to 
	// the current time of the scene.
	virtual TimeValue GetUpdateTime() { return 0; }
	// Implemented by the Plug-in
	// Use this method to retrieve time interval of the current update step. The update time maybe unrelated to 
	// the current time of the scene. The GetUpdateTime method above retrieves the finish time.
	virtual void GetUpdateInterval(TimeValue& start, TimeValue& finish) { ; }

	// Implemented by the Plug-In.
	// The method returns how many particles are currently in the particle system. 
	// Some of these particles may be dead or not born yet (indicated by GetAge(..) method =-1). 
	virtual int NumParticles() = 0;

	// Implemented by the Plug-In.
	// The method returns how many particles were born. Since particle systems have
	// a tendency of reusing indices for newly born particles, sometimes it's necessary 
	// to keep a track for particular particles. This method and the methods that deal with
	// particle IDs allow us to accomplish that.
	virtual int NumParticlesGenerated() { return NumParticles(); }

	// Implemented by the Plug-in
	// The following four methods modify amount of particles in the particle system
	// Returns true if the operation was completed successfully
	//		Add a single particle
	virtual bool AddParticle() { return false; }
	//		Add "num" particles into the particle system
	virtual bool AddParticles(int num) { return false; }
	//		Delete a single particle with the given index
	virtual bool DeleteParticle(int index) { return false; }
	//		List-type delete of "num" particles starting with "start"
	virtual bool DeleteParticles(int start, int num) { return false; }

	// Implemented by the Plug-In.
	// Each particle is given a unique ID (consecutive) upon its birth. The method 
	// allows us to distinguish physically different particles even if they are using 
	// the same particle index (because of the "index reusing").
	// Parameters:
	//		int i
	//			index of the particle in the range of [0, NumParticles-1]
	virtual int GetParticleBornIndex(int i) { return 0; }

	// Implemented by the Plug-In.
	// the methods verifies if a particle with a given particle id (born index) is present
	// in the particle system. The methods returns Particle Group node the particle belongs to,
	// and index in this group. If there is no such particle, the method returns false.
	// Parameters:
	//		int bornIndex
	//			particle born index
	//		INode*& groupNode
	//			particle group the particle belongs to
	//		int index
	//			particle index in the particle group or particle system
	virtual bool HasParticleBornIndex(int bornIndex, int& index) { return false; }
	virtual INode* GetParticleGroup(int index) { return NULL; }
	virtual int GetParticleIndex(int bornIndex) { return 0; }

	// Implemented by the Plug-In.
	// The following four methods define "current" index or bornIndex. This index is used
	// in the property methods below to get the property without specifying the index.
	virtual int GetCurrentParticleIndex() { return 0; }
	virtual int GetCurrentParticleBornIndex() { return 5; }
	virtual void SetCurrentParticleIndex(int index) { ; }
	virtual void SetCurrentParticleBornIndex(int bornIndex) { ; }

	// Implemented by the Plug-In.
	// The following six methods define age of the specified particle. Particle is specified by either its
	// index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		TimeValue age
	//			new age value to set for a particle
	virtual TimeValue GetParticleAgeByIndex(int index) { return 0; }
	virtual TimeValue GetParticleAgeByBornIndex(int id) { return 0; }
	virtual void SetParticleAgeByIndex(int index, TimeValue age) { ; }
	virtual void SetParticleAgeByBornIndex(int id, TimeValue age) { ; }
	virtual TimeValue GetParticleAge() { return 0; }
	virtual void SetParticleAge(TimeValue age) { ; }

	// Implemented by the Plug-In.
	// The following six methods define lifespan of the specified particle. Particle is specified by either its
	// index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		TimeValue lifespan
	//			new lifespan value to set for a particle
	virtual TimeValue GetParticleLifeSpanByIndex(int index) { return TIME_PosInfinity; }
	virtual TimeValue GetParticleLifeSpanByBornIndex(int id) { return TIME_PosInfinity; }
	virtual void SetParticleLifeSpanByIndex(int index, TimeValue LifeSpan) { ; }
	virtual void SetParticleLifeSpanByBornIndex(int id, TimeValue LifeSpan) { ; }
	virtual TimeValue GetParticleLifeSpan() { return TIME_PosInfinity; }
	virtual void SetParticleLifeSpan(TimeValue lifespan) { ; }

	// Implemented by the Plug-In.
	// The following six methods define for how long the specified particle was staying in the current
	// particle group. Particle is specified by either its
	// index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		TimeValue time
	//			how long particle was staying in the current particle group
	virtual TimeValue GetParticleGroupTimeByIndex(int index) { return 0; }
	virtual TimeValue GetParticleGroupTimeByBornIndex(int id) { return 0; }
	virtual void SetParticleGroupTimeByIndex(int index, TimeValue time) { ; }
	virtual void SetParticleGroupTimeByBornIndex(int id, TimeValue time) { ; }
	virtual TimeValue GetParticleGroupTime() { return 0; }
	virtual void SetParticleGroupTime(TimeValue time) { ; }
	
	// Implemented by the Plug-In.
	// The following six methods define position of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		Point3 pos
	//			position of the particle
	virtual Point3* GetParticlePositionByIndex(int index) { return NULL; }
	virtual Point3* GetParticlePositionByBornIndex(int id) { return NULL; }
	virtual void SetParticlePositionByIndex(int index, Point3 pos) { ; }
	virtual void SetParticlePositionByBornIndex(int id, Point3 pos) { ; }
	virtual Point3* GetParticlePosition() { return NULL; }
	virtual void SetParticlePosition(Point3 pos) { ; }

	// Implemented by the Plug-In.
	// The following six methods define speed of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		Point3 speed
	//			speed of the particle in units per frame
	virtual Point3* GetParticleSpeedByIndex(int index) { return NULL; }
	virtual Point3* GetParticleSpeedByBornIndex(int id) { return NULL; }
	virtual void SetParticleSpeedByIndex(int index, Point3 speed) { ; }
	virtual void SetParticleSpeedByBornIndex(int id, Point3 speed) { ; }
	virtual Point3* GetParticleSpeed() { return NULL; }
	virtual void SetParticleSpeed(Point3 speed) { ; }

	// Implemented by the Plug-In.
	// The following six methods define orientation of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		Point3 orient
	//			orientation of the particle. The orientation is defined by incremental rotations
	//			by world axes X, Y and Z. The rotation values are in degrees.
	virtual Point3* GetParticleOrientationByIndex(int index) { return NULL; }
	virtual Point3* GetParticleOrientationByBornIndex(int id) { return NULL; }
	virtual void SetParticleOrientationByIndex(int index, Point3 orient) { ; }
	virtual void SetParticleOrientationByBornIndex(int id, Point3 orient) { ; }
	virtual Point3* GetParticleOrientation() { return NULL; }
	virtual void SetParticleOrientation(Point3 orient) { ; }

	// Implemented by the Plug-In.
	// The following six methods define angular speed of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		AngAxis spin
	//			angular speed of the particle in rotation per frame
	//			axis defines rotation axis, angle defines rotation amount per frame
	virtual AngAxis* GetParticleSpinByIndex(int index) { return NULL; }
	virtual AngAxis* GetParticleSpinByBornIndex(int id) { return NULL; }
	virtual void SetParticleSpinByIndex(int index, AngAxis spin) { ; }
	virtual void SetParticleSpinByBornIndex(int id, AngAxis spin) { ; }
	virtual AngAxis* GetParticleSpin() { return NULL; }
	virtual void SetParticleSpin(AngAxis spin) { ; }

	// Implemented by the Plug-In.
	// The following twelve methods define scale factor of the specified particle in the current state.
	// The XYZ form is used for non-uniform scaling
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		float scale
	//			uniform scale factor
	//		Point3 scale
	//			scale factor for each local axis of the particle
	virtual float GetParticleScaleByIndex(int index) { return 1.0f; }
	virtual float GetParticleScaleByBornIndex(int id) { return 1.0f; }
	virtual void SetParticleScaleByIndex(int index, float scale) { ; }
	virtual void SetParticleScaleByBornIndex(int id, float scale) { ; }
	virtual float GetParticleScale() { return 1.0f; }
	virtual void SetParticleScale(float scale) { ; }
	virtual Point3* GetParticleScaleXYZByIndex(int index) { return NULL; }
	virtual Point3* GetParticleScaleXYZByBornIndex(int id) { return NULL; }
	virtual void SetParticleScaleXYZByIndex(int index, Point3 scale) { ; }
	virtual void SetParticleScaleXYZByBornIndex(int id, Point3 scale) { ; }
	virtual Point3* GetParticleScaleXYZ() { return NULL; }
	virtual void SetParticleScaleXYZ(Point3 scale) { ; }

	// Implemented by the Plug-In.
	// The following six methods define transformation matrix of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		Matrix3 tm
	//			transformation matrix of the particle
	virtual Matrix3* GetParticleTMByIndex(int index) { return NULL; }
	virtual Matrix3* GetParticleTMByBornIndex(int id) { return NULL; }
	virtual void SetParticleTMByIndex(int index, Matrix3 tm) { ; }
	virtual void SetParticleTMByBornIndex(int id, Matrix3 tm) { ; }
	virtual Matrix3* GetParticleTM() { return NULL; }
	virtual void SetParticleTM(Matrix3 tm) { ; }

	// Implemented by the Plug-In.
	// The following six methods define selection status of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		bool selected
	//			selection status of the particle
	virtual bool GetParticleSelectedByIndex(int index) { return true; }
	virtual bool GetParticleSelectedByBornIndex(int id) { return true; }
	virtual void SetParticleSelectedByIndex(int index, bool selected) { ; }
	virtual void SetParticleSelectedByBornIndex(int id, bool selected) { ; }
	virtual bool GetParticleSelected() { return true; }
	virtual void SetParticleSelected(bool selected) { ; }


	// Implemented by the Plug-In.
	// The following seven methods define shape of the specified particle in the current state.
	// Particle is specified by either its index in the particle group or particle system, or by its born index
	// if no index is specified then the "current" index is used
	// Parameters:
	//		int id
	//			particle born index
	//		int index
	//			particle index in the particle group
	//		Mesh* shape
	//			shape of the particle
	virtual Mesh* GetParticleShapeByIndex(int index) { return NULL; }
	virtual Mesh* GetParticleShapeByBornIndex(int id) { return NULL; }
	virtual void SetParticleShapeByIndex(int index, Mesh* shape) { ; }
	virtual void SetParticleShapeByBornIndex(int id, Mesh* shape) { ; }
	virtual Mesh* GetParticleShape() { return NULL; }
	virtual void SetParticleShape(Mesh* shape) { ; }
	// set the same shape for all particles
	virtual void SetGlobalParticleShape(Mesh* shape) { ; }

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLEOBJECTEXT_INTERFACE); }
};

#pragma warning(pop)

