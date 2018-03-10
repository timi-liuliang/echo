/*********************************************************************
 *<
	FILE: bipedapi.h

	DESCRIPTION: These are functions that are exported in biped.dlc

	CREATED BY:	Ravi Karra

	HISTORY: Created 21 June 1999
			 Modified July 2002 by Michael Zyracki

 *>	Copyright (c) 2002 All Rights Reserved.
 **********************************************************************/

#pragma once

#include "BipExp.h"
#include "..\maxheap.h"
#include "..\strbasic.h"
#include "..\tab.h"
#include "..\matrix3.h"
#include "..\interval.h"
#include "..\control.h"
#include "..\istdplug.h"

// forward declarations
class IBipMaster;
class IMoFlow;
class IMixer;
class MocapManager;
class MixerManager;
class MultFprintParams;
class Point3;
class INode;


// Interfaces -work similarly to the biped export interface
#define I_BIPMASTER		0x9165
#define I_BIPFOOTSTEP	0x9166
#define GetBipMasterInterface(anim) ((IBipMaster*)(anim)->GetInterface(I_BIPMASTER))
#define GetBipFSInterface(anim) ((IBipFootStep*)(anim)->GetInterface(I_BIPFOOTSTEP))

// Biped modes
#define BMODE_FIGURE		(1<<0)
#define BMODE_FOOTSTEP		(1<<1)
#define BMODE_MOTIONFLOW	(1<<2)
#define BMODE_BUFFER		(1<<3)
#define BMODE_BENDLINKS		(1<<4)
#define BMODE_RUBBERBAND	(1<<5)
#define BMODE_SCALESTRIDE	(1<<6)
#define BMODE_INPLACE		(1<<7)
#define BMODE_INPLACE_X		(1<<8)
#define BMODE_INPLACE_Y		(1<<9)
#define BMODE_MIXER			(1<<10)
#define BMODE_MOVEALL		(1<<11)

// Display settings
#define BDISP_BONES			(1<<0)
#define BDISP_OBJECTS		(1<<1)
#define BDISP_FOOTSTEPS		(1<<2)
#define BDISP_FOOTSTEPNUM	(1<<3)
#define BDISP_TRAJ			(1<<4)

// Biped Gait Flags
#define WALKGAIT    1
#define RUNGAIT     2
#define JUMPGAIT    3

//Copy/Paste types
#define COPY_POSTURE	0
#define COPY_POSE		1
#define COPY_TRACK		2

// Body types
#define BTYPE_SKELETON		0
#define BTYPE_MALE			1
#define BTYPE_FEMALE		2
#define BTYPE_CLASSIC		3

#define NUMPIVOTS	27 // Max number of pivot points that can exiss for an object



// Create a new biped with the given options
BIPExport IBipMaster* CreateNewBiped(float height, float angle, const Point3& wpos, 
				BOOL arms=TRUE, BOOL triPelvis=TRUE, int nnecklinks=1, int nspinelinks=4, 
				int nleglinks=3, int ntaillinks=0, int npony1links=0, int npony2links=0,   
				int numfingers=5, int nfinglinks=3, int numtoes=5, int ntoelinks=3, float ankleAttach=0.2,
				BOOL prop1exists = FALSE,BOOL prop2exists = FALSE, BOOL prop3exists = FALSE,
				int forearmTwistLinks = 0, int upperarmTwistLinks = 0, int thighTwistLinks = 0,
				int calfTwistLinks = 0, int horseTwistLinks = 0);

// The ticks per frame used by the biped (Currently same as GetTicksPerFrame()).
BIPExport int BipGetTicksPerFrame();


#define MB_FSJUMP		0
#define MB_FFMODE		1
#define MB_RCNTFIG		2

bool GetMsgBoxStatus(int which);
void SetMsgBoxStatus(int which, bool hide);

// Global object that contains the Mocap Interface 
extern BIPExport MocapManager TheMocapManager;

// Global object that contains the Mixer Interface 
extern BIPExport MixerManager TheMixerManager;

//! \brief Structure specifying which controllers to save in a BIP file
struct SaveSubAnimInfo: public MaxHeapOperators
{
	//! \brief The node whose controller will be saved  
	INode *node;
	//! \brief The type of controller to save: position = 0, rotation = 1, scale = 2
	int type;
};

//! \brief Structure specifying which controllers to load from a BIP file
struct LoadSubAnimInfo: public MaxHeapOperators
{
	//! \brief The name of the node whose controller will be loaded 
	MCHAR name[256];
	//! \brief The type of controller to load: position = 0, rotation = 1, scale = 2
	int type;
};


// Interface into the biped master controller
class IBipMaster: public MaxHeapOperators {
	public:
		virtual ~IBipMaster() {;}
		// Track selection, only work when the UI is showing up in command panel. The #defines are in tracks.h
		virtual void	SetTrackSelection(int track)=0;
		virtual int		GetTrackSelection()=0;

		// File I/O methods  
		//These functions pop-up the dialog for file selection.
		virtual void SaveBipFileDlg() = 0;
		virtual void LoadBipFileDlg() = 0;
		virtual int		SaveFigfile		(const MCHAR *fname)=0;
        virtual int		SaveStpfile		(const MCHAR *fname)=0;
		//These functions don't pop-up a dialog for I/O
		virtual int 	SaveBipfile(const MCHAR *fname, BOOL SaveListCntrls,BOOL SaveMaxObjects,Tab<SaveSubAnimInfo> *selectedControlers =NULL,
			Tab<INode *> *selectedNodes = NULL) = 0;
        virtual int 	SaveBipfileSegment(const MCHAR *filenamebuf,int StartSeg,int EndSeg,int SegKeyPerFrame,
			BOOL SaveListCntrls, BOOL SaveMaxObjects,Tab<SaveSubAnimInfo> *selectedControlers =NULL,
			Tab<INode *> *selectedNodes = NULL) = 0;
		virtual int		LoadFigfile		(const MCHAR *fname, BOOL redraw = false, BOOL msgs = false)=0; 
        virtual int		LoadBipStpfile(const MCHAR *fname, BOOL redraw, BOOL msgs, BOOL MatchFile = false, BOOL ZeroHgt = false, BOOL loadMaxObjects = false,
							  BOOL promptForDuplicates = false, BOOL retargetHeight = false, BOOL retargetLimbSizes = false, 
							  BOOL scaleIKObjectSize = false, BOOL loadSubAnimControllers = false,Tab<MCHAR *> *selectedNodes = NULL,
							  Tab<LoadSubAnimInfo> *selectedControllers = NULL) =0;
        virtual int		LoadMocapfile	(const MCHAR *fname, BOOL redraw = false, BOOL msgs = false, BOOL prompt = false)=0;

		// General+Modes 
		virtual BOOL	IsCreating()=0;  //will return TRUE if creating
		virtual void	BeginModes(DWORD modes, int redraw=TRUE)=0;
		virtual void	EndModes(DWORD modes, int redraw=TRUE)=0;
		virtual DWORD	GetActiveModes()=0;
		virtual	BOOL	CanSwitchMode(DWORD mode)=0; //returns TRUE if we can switch to that mode from our current mode
		virtual	void	ConvertToFreeForm(bool keyPerFrame=false)=0;
		virtual	void	ConvertToFootSteps(bool keyPerFrame=false, bool flattenToZ=true)=0;

		// Display properties
		virtual DWORD	GetDisplaySettings()=0;
		virtual void	SetDisplaySettings(DWORD disp)=0;
		virtual	BOOL	DoDisplayPrefDlg(HWND hParent)=0;
		
		// Body types
		virtual int	GetBodyType() =0;
		virtual void	SetBodyType(int bodytype) =0;

		// Anim properties 
		virtual	int		GetDynamicsType()=0;
		virtual	void	SetDynamicsType(int dyn)=0;
		virtual	float	GetGravAccel()=0;
		virtual void	SetGravAccel(float grav)=0;
		virtual const MCHAR*	GetRootName()=0;
		virtual void	SetRootName(const MCHAR *rootname, bool incAll=true)=0;
		virtual BOOL	GetAdaptLocks(int id)=0;
		virtual void	SetAdaptLocks(int id, BOOL onOff)=0;
		virtual BOOL	GetSeparateTracks(int id)=0;		
		virtual void	SeparateTracks(int id, BOOL separate)=0;
		virtual void SetBodySpaceNeckRotation(BOOL val)=0;
		virtual BOOL GetBodySpaceNeckRotation()=0;

		// Structure properties
		virtual BOOL	GetHasArms()=0;
		virtual void	SetHasArms(BOOL arms)=0;
		virtual int		GetNumLinks(int keytrack)=0;
		virtual void	SetNumLinks(int keytrack, int n)=0;
		virtual int		GetNumFingers()=0;
		virtual void	SetNumFingers(int n)=0;
		virtual int		GetNumToes()=0;
		virtual void	SetNumToes(int n)=0;
		virtual float	GetAnkleAttach()=0;
		virtual void	SetAnkleAttach(float aa)=0;
		virtual float	GetHeight()=0;
		virtual void	SetHeight(float h, BOOL KeepFeetOnGround = TRUE)=0;
		virtual BOOL	GetTrianglePelvis()=0;
		virtual void	SetTrianglePelvis(BOOL tri)=0;
		virtual BOOL	GetProp1Exists()=0;
		virtual void 	SetProp1Exists(BOOL prop)=0;
		virtual BOOL	GetProp2Exists()=0;
		virtual void 	SetProp2Exists(BOOL prop)=0;
		virtual BOOL	GetProp3Exists()=0;
		virtual void 	SetProp3Exists(BOOL prop)=0;
		
		// mocap params
		virtual BOOL	ConvertFromBuffer()=0;
		virtual BOOL	PasteFromBuffer()=0;
		virtual BOOL	GetDispBuffer()=0;
		virtual void	SetDispBuffer(BOOL onOff)=0;
		virtual BOOL	GetDispBufferTraj()=0;
		virtual void	SetDispBufferTraj(BOOL onOff)=0;
		virtual BOOL	GetTalentFigMode()=0;
		virtual void	SetTalentFigMode(BOOL onOff)=0;
		virtual void	AdjustTalentPose()=0;
		virtual void	SaveTalentFigFile(const MCHAR *fname)=0;
		virtual void	SaveTalentPoseFile(const MCHAR *fname)=0;

		// footstep creation/operations
		virtual BOOL	GetFSAppendState()=0;
		virtual void	SetFSAppendState(BOOL onOff)=0;
		virtual BOOL	GetFSInsertState()=0;
		virtual void	SetFSInsertState(BOOL onOff)=0;
		virtual	int		GetGaitMode()=0;
		virtual	void	SetGaitMode(int mode)=0;
		virtual	int		GetGroundDur()=0;
		virtual	void	SetGroundDur(int val)=0;
		virtual	int		GetAirDur()=0;
		virtual	void	SetAirDur(int val)=0;
		virtual	void	DoMultipleFSDlg()=0;
		virtual	int		AddFootprint(Point3 pos, float dir, Matrix3 mtx, int appendFS)=0;
		virtual	void	AddFootprints(MultFprintParams *Params)=0;
		virtual	void	NewFprintKeys()=0;
		virtual	void	BendFootprints(float angle)=0;
		virtual	void	ScaleFootprints(float scale)=0;

		// motion flow interface
		virtual	IMoFlow* GetMoFlow()=0;
        virtual void	UnifyMotion()=0;
		virtual const MCHAR*  GetClipAtTime(TimeValue t)=0; //returns the current clip

		// mixer  interface
		virtual	IMixer* GetMixer()=0;

		// IK objects
		//this set's the ik object for the current selected body part
		virtual void	SetAttachNode(INode *node)=0;
		virtual INode*	GetAttachNode()=0;
		//head target
		virtual void	SetHeadTarget(INode *node)=0;
		virtual INode * GetHeadTarget() =0;

		// Anim,controls,nodes....
		virtual	bool	IsNodeDeleted()=0; //test to see if the interface's node has been deleted.
		virtual Interval GetCurrentRange()=0;
		virtual	int		GetMaxNodes()=0;
		virtual	int		GetMaxLinks()=0;
		virtual	INode*	GetNode(int id, int link=0)=0;
		virtual BOOL GetIdLink(INode *node, int &id, int &link)=0;
		virtual Control * GetHorizontalControl()=0;
		virtual Control * GetVerticalControl()=0;
		virtual Control * GetTurnControl()=0;

		//get set keys and transforms. (in world space)
		virtual	void SetBipedKey(TimeValue t,INode *node = NULL, BOOL setHor = TRUE, BOOL setVer = TRUE,BOOL setTurn = TRUE)=0;
		virtual	void SetPlantedKey(TimeValue t,INode *node = NULL)=0;
		virtual	void SetSlidingKey(TimeValue t,INode *node = NULL)=0;
		virtual	void SetFreeKey(TimeValue t,INode *node = NULL)=0;
		virtual ScaleValue GetBipedScale(TimeValue t, INode *node)=0;
		virtual Point3	GetBipedPos(TimeValue t, INode *node)=0;
		virtual Quat	GetBipedRot(TimeValue t, INode *node,BOOL local = FALSE)=0;
		//note that this set's a relative scale!
		virtual void	SetBipedScale(BOOL relative,const ScaleValue &scale, TimeValue t, INode *node)=0;
        virtual void	SetBipedPos(const Point3 &p, TimeValue t, INode *node,BOOL setKey =TRUE)=0;		
        virtual void	SetBipedRot(const Quat &q, TimeValue t, INode *node,BOOL setKey = TRUE)=0;
        virtual void	SetMultipleKeys()=0;
		virtual void	DoSetMultipleKeysDlg()=0;				

		//reset's the limb ik pivots. useful when using subanim scale and there's some wierdness like sub-frame popping.
		virtual void ResetAllLimbKeys() = 0;

		//collapse the move all mode down
		virtual void CollapseMoveAllMode(BOOL msg) =0;

		//Biped Internal structures get/sets. These functions deal with internal biped structures and hierarchy.			
		virtual INode*	GetRotParentNode(int id,int link)=0; //the parent node where the rotation is inherited from
		virtual INode*	GetPosParentNode(int id,int link)=0; //the parent node where the position is inherited from
		virtual Quat	GetParentNodeRot(TimeValue t,int id,int link)=0; //rotation value of the parent
		virtual Point3	GetParentNodePos(TimeValue t,int id,int link)=0; //position value of the parent.
		virtual void GetClavicleVals(TimeValue t, int id, float &val1,float &val2)=0; //this is valid for KEY_RARM & KEY_LARM or
		virtual void GetHingeVal(TimeValue t,int id, float &val)=0; //this is valid for KEY_RARM & KEY_LARM & KEY_RLEG & KEY_LLEG. it get's the elbow/knee angle
		virtual void GetHorseAnkleVal(TimeValue, int id, float &val)=0;	//this is valid only if you have a horse leg and KEY_RLEG and  KEY_LLEG
		virtual void GetPelvisVal(TimeValue t, float &val) =0; //get's the pelvis angle
		virtual void GetFingerVal(TimeValue t,int id,int link, float &val) = 0;//get the finger rotation value for the finger segements with 1 DOF
		virtual BOOL GetIKActive(TimeValue t,int id) = 0;//Fuction to see if a biped limb is effect by ik at a particular time

		// Layers.
		virtual int		NumLayers()=0;
		virtual void	CreateLayer(int index, const MCHAR* name)=0;
		virtual void	DeleteLayer(int index)=0;
		virtual bool	CollapseAtLayer(int index)=0; //only works if all layers under this layer are active, returns true if successul 		
		virtual bool	GetLayerActive(int index)=0;
		virtual void	SetLayerActive(int index, bool onOff)=0;
		virtual const MCHAR*	GetLayerName(int index)=0;
		virtual void	SetLayerName(int index, const MCHAR* name)=0;
		virtual int		GetCurrentLayer()=0;
		virtual void	SetCurrentLayer(int index)=0;
		virtual void	UpdateLayers()=0; // need to call this after changes made to layers 
		virtual void	SetSnapKey(TimeValue t,INode *node = NULL)=0;
		//layer display info
		virtual int		GetVisibleBefore()=0;
		virtual void	SetVisibleBefore(int val)=0;
		virtual int		GetVisibleAfter()=0;
		virtual void	SetVisibleAfter(int val)=0;
		virtual bool	GetKeyHighlight()=0;
		virtual void	SetKeyHighlight(bool onOff)=0;

        // preferred clips for use with biped crowd.  
        virtual void	ClearPreferredClips()=0;
        virtual bool	AddPreferredClip(const MCHAR *clipname, int prob = 100)=0;
        virtual bool	DeletePreferredClip(const MCHAR *clipname)=0;
        virtual int 	IsPreferredClip(const MCHAR *clipname)=0;
        virtual const MCHAR*	GetCurrentClip()=0;
		virtual int		NumPreferredClips()=0;
		virtual const MCHAR*	GetPreferredClip(int i)=0;
		virtual int 	GetPreferredClipProbability(int i)=0;

		//Biped Subanims
		virtual bool	GetEnableSubAnims()=0;
		virtual void	SetEnableSubAnims(bool onOff)=0;
		virtual bool	GetManipSubAnims()=0; 
		virtual	void 	SetManipSubAnims(bool onOff)=0;

		//Trackbar stuff	//this is post CS4.2 
		virtual bool GetShowAllTracksInTV()=0;
		virtual void SetShowAllTracksInTV(bool onOff)=0;
		//doesn't workvirtual bool GetShowSubanimInTrackBar()=0;
		//virtual void SetShowSubanimInTrackBar(bool onOff)=0;
		virtual bool GetShowBipedInTrackBar()=0;
		virtual void SetShowBipedInTrackBar(bool onOff)=0;
		virtual void Mirror() =0;
		virtual void ClearAllAnimation() =0;
		virtual void ClearSelectedAnimation() =0;

		//These  functions will clone the 'controlToClone' control and put it in each appropiate BipedSubAnim::List,
		//and make that control the active control.  If 'checkIfOneExists' is TRUE it will first check to see 
		//if a controller of the same type already exists in the subanim list, in which case it will just
		//set that one as active,and not clone a new one.
		virtual void CreatePosSubAnims(Control *controlToClone, BOOL checkIfOneExists)=0;
		virtual void CreateRotSubAnims(Control *controlToClone, BOOL checkIfOneExists)=0;
		virtual void CreateScaleSubAnims(Control *controlToClone, BOOL checkIfOneExists)=0;

		//these functions Set a key for the appropiate active controller in the list controller for the
		//specified node.  If 'absolute' is true the value used to set the key is the total combined value
		//of the underlying biped value plus the subanim value. Thus the subanim value will be calculating
		//by subtracting out the biped value.  If 'absolute' is false the value is the exact subanim key value
		//that will be set.  Due to the limitation in defining a global 'biped scale', the scale type of this
		//function has no absolute parameter and always just sets the key with the specified value
		virtual void  SetPosSubAnim(const Point3 &p, TimeValue t, INode *node,BOOL absolute)=0;
        virtual void  SetRotSubAnim(const Quat &q, TimeValue t, INode *node,BOOL absolute) = 0;		
        virtual void  SetScaleSubAnim(const ScaleValue &s, TimeValue t, INode *node) = 0;		

		//these function calls collapse the specified subAnims..
		virtual void CollapseAllPosSubAnims(BOOL perFrame,BOOL keep) = 0;
		virtual void CollapseAllRotSubAnims(BOOL perFrame,BOOL keep) = 0;
		virtual void CollapseRotSubAnims(BOOL perFrame,BOOL keep,INode *node) = 0;
		virtual void CollapsePosSubAnims(BOOL perFrame,BOOL keep,INode *node) = 0;

		//Copy/Paste exposure
		virtual char * CopyPosture(int copyType,BOOL copyHor,BOOL copyVer,BOOL copyTurn) = 0;
        virtual BOOL PastePosture(int copyType,int opposite,char *name) = 0;
		virtual void DeleteAllCopies(int copyType, BOOL holdIt = true) = 0;
		virtual	int NumCopies(int copyType) = 0;
		virtual	void DeleteCopy(int copyType,char *name) = 0;
		virtual	const MCHAR *GetCopyName(int copyType,int index) = 0;
		virtual	void SetCopyName(int copyType,int index, char * newName) = 0;
		virtual BOOL SaveCopyPasteFile(char *fname) = 0;
		virtual	BOOL LoadCopyPasteFile(char *fname) = 0;
	};

//Interface to the biped footstep
class IBipFootStep: public MaxHeapOperators
{
	public:
		virtual ~IBipFootStep() {;}
		virtual void	SetFreeFormMode(BOOL mode)=0;
		virtual BOOL	GetFreeFormMode()=0;
		virtual	void	SetDispNumType(int type)=0;
		virtual	int		GetDispNumType()=0;
		virtual	void	SetDispAirDur(BOOL onOff)=0;
		virtual	BOOL	GetDispAirDur()=0;
		virtual	void	SetDispNoSupport(BOOL onOff)=0;
		virtual	BOOL	GetDispNoSupport()=0;
		virtual	void	UpdateEditTrackUI()=0;
		virtual	void	UpdateFootSteps(TimeValue t){ UNUSED_PARAM(t); }
};

// defines for mocap key reduction settings
#define KRS_ALL  0
#define KRS_HORZ 1
#define KRS_VERT 2
#define KRS_ROT  3
#define KRS_PLV  4
#define KRS_SPN  5
#define KRS_NCK  6
#define KRS_LARM 7
#define KRS_RARM 8
#define KRS_LLEG 9
#define KRS_RLEG 10
#define KRS_TAIL 11


// defines for mocap GetLimbOrientation/SetLimbOrientation
#define LIMB_KNEE   0
#define LIMB_ELBOW  1
#define LIMB_FOOT   2
#define LIMB_HAND   3

#define ANGLE_ALIGN 0
#define POINT_ALIGN 1
#define AUTO_ALIGN  2

//The mocap manager class.
class MocapManager: public MaxHeapOperators {
	public:
		// import dialog properties	
		BIPExport const MCHAR* GetTalentFigStrucFile();
		BIPExport void	SetTalentFigStrucFile(const MCHAR *fname);
		BIPExport BOOL	GetUseTalentFigStrucFile() const;
		BIPExport void	SetUseTalentFigStrucFile(BOOL onOff);

		BIPExport const MCHAR* GetTalentPoseAdjFile() const;
		BIPExport void	SetTalentPoseAdjFile(const MCHAR *fname);
		BIPExport BOOL	GetUseTalentPoseAdjFile() const;
		BIPExport void	SetUseTalentPoseAdjFile(BOOL onOff);

		BIPExport int	GetFSExtractionMode() const;
		BIPExport void	SetFSExtractionMode(int mode);

		BIPExport int	GetFSConversionMode() const;
		BIPExport void	SetFSConversionMode(int mode);
		
		// 0 - x, 1 - y, 2 - z
		BIPExport int	GetUpVector() const;
		BIPExport void	SetUpVector(int axis);

		BIPExport float	GetScaleFactor() const;
		BIPExport void	SetScaleFactor(float val);


		BIPExport float	GetFSExtractionTol() const;
		BIPExport void	SetFSExtractionTol(float val);

		BIPExport float	GetFSSlidingDist() const;
		BIPExport void	SetFSSlidingDist(float val);
		BIPExport float	GetFSSlidingAngle() const;
		BIPExport void	SetFSSlidingAngle(float val);

		BIPExport float	GetFSVerticalTol() const;
		BIPExport void	SetFSVerticalTol(float val);
		
		BIPExport float	GetFSZLevel() const;
		BIPExport void	SetFSZLevel(float val);
		
		BIPExport BOOL	GetFSUseVerticalTol() const;
		BIPExport void	SetFSUseVerticalTol(BOOL val);

		BIPExport BOOL	GetFSUseFlatten() const;
		BIPExport void	SetFSUseFlatten(BOOL val);

		BIPExport int	GetStartFrame() const;
		BIPExport void	SetStartFrame(int val);
		BIPExport int	GetEndFrame() const;
		BIPExport void	SetEndFrame(int val);
		BIPExport BOOL	GetUseLoopFrame() const;
		BIPExport void	SetUseLoopFrame(BOOL val);
		BIPExport int	GetLoopFrameCount() const;
		BIPExport void	SetLoopFrameCount(int val);
		
		BIPExport float	GetKeyReductionTol(int part) const;
		BIPExport void	SetKeyReductionTol(int part, float val);
		BIPExport int	GetKeyReductionSpacing(int part) const;
		BIPExport void	SetKeyReductionSpacing(int part, float val);
		BIPExport BOOL	GetKeyReductionFilter(int part) const;
		BIPExport void	SetKeyReductionFilter(int part, BOOL onOff);

		BIPExport int	GetLimbOrientation(int limb) const;
		BIPExport void	SetLimbOrientation(int limb, int val);

		BIPExport int	LoadMocapParameters(const MCHAR *fname);
		BIPExport int	SaveMocapParameters(const MCHAR *fname);

		// marker name dialog
		BIPExport const MCHAR* GetMarkerNameFile() const;
		BIPExport bool	LoadMarkerNameFile(const MCHAR *fname);
		BIPExport BOOL	GetUseMarkerNameFile() const;
		BIPExport void	SetUseMarkerNameFile(BOOL onOff);
		
		BIPExport const MCHAR* GetJointNameFile() const;
		BIPExport bool	LoadJointNameFile(const MCHAR *fname);
		BIPExport BOOL	GetUseJointNameFile() const;
		BIPExport void	SetUseJointNameFile(BOOL onOff);

		BIPExport int	BatchConvert(const MCHAR* inDir, const MCHAR* outDir, const MCHAR* ext);

		BIPExport BOOL	GetDispKnownMarkers() const; 
		BIPExport void	SetDispKnownMarkers(BOOL onOff);
		BIPExport int	GetDispKnownMarkersType() const; // 0 - Sel objects, 1 - all
		BIPExport void	SetDispKnownMarkersType(int type); // 0 - Sel objects, 1 - all
		BIPExport BOOL	GetDispUnKnownMarkers() const;
		BIPExport void	SetDispUnKnownMarkers(BOOL onOff);
		BIPExport BOOL	GetDispPropMarkers() const; 
		BIPExport void	SetDispPropMarkers(BOOL onOff);
};

//The mixer manager class.
class MixerManager: public MaxHeapOperators {
	public:
		BIPExport BOOL   GetSnapFrames();
		BIPExport void   SetSnapFrames(BOOL onOff);
		BIPExport BOOL   GetShowTgRangebars();
		BIPExport void   SetShowTgRangebars(BOOL onOff);
		BIPExport BOOL   GetShowWgtCurves();
		BIPExport void   SetShowWgtCurves(BOOL onOff);
		BIPExport BOOL   GetShowTimeWarps();
		BIPExport void   SetShowTimeWarps(BOOL onOff);
		BIPExport BOOL   GetShowClipBounds();
		BIPExport void   SetShowClipBounds(BOOL onOff);
		BIPExport BOOL   GetShowGlobal();
		BIPExport void   SetShowGlobal(BOOL onOff);
		BIPExport BOOL   GetShowClipNames();
		BIPExport void   SetShowClipNames(BOOL onOff);
		BIPExport BOOL   GetShowClipScale();
		BIPExport void   SetShowClipScale(BOOL onOff);
		BIPExport BOOL   GetShowTransStart();
		BIPExport void   SetShowTransStart(BOOL onOff);
		BIPExport BOOL   GetShowTransEnd();
		BIPExport void   SetShowTransEnd(BOOL onOff);
		BIPExport BOOL   GetShowBalance();
		BIPExport void   SetShowBalance(BOOL onOff);
		BIPExport BOOL   GetSnapToClips();
		BIPExport void   SetSnapToClips(BOOL onOff);
		BIPExport BOOL   GetLockTransitions();
		BIPExport void   SetLockTransitions(BOOL onOff);
		BIPExport void   SetAnimationRange();
		BIPExport void   ZoomExtents();
		BIPExport void   UpdateDisplay();
		BIPExport void   AddBipedToMixerDisplay(IBipMaster *mc);
		BIPExport void   RemoveBipedFromMixerDisplay(IBipMaster *mc);
		BIPExport void   ShowMixer();
		BIPExport void   HideMixer();
		BIPExport void	 ToggleMixer();
};


// Base Biped Key. For pelvis, spine, neck, tail, pony1, pony2 keys
class IBipedKey : public IKey {
	public:
		float	tens, cont, bias, easeIn, easeOut;
		int		type;
};

// Biped COM Vertical Key
class IBipedVertKey : public IBipedKey {
	public:
		float	z, dynBlend, ballTens;
};

// Biped COM Horizontal Key
class IBipedHorzKey : public IBipedKey {
	public:
		float	x, y, balFac;
};

// Biped COM Turn Key
class IBipedTurnKey : public IBipedKey {
	public:
		Quat q;
};


// Biped Body Key for the Arm and Leg Keys. contains ik blend and pivot point info.
// IK spaces
#define BODYSPACE   0
#define WORLDSPACE  1	// Currently not valid
#define OBJECTSPACE 2
class IBipedBodyKey : public IBipedKey {
	public:
		float	ik_blend,
				ik_ankle_ten;
		int		ik_space,				
				ik_joined_pivot,
				ik_pivot_index,
				ik_num_pivots;
		Point3	ik_pivot_pts[NUMPIVOTS];
};

// Biped Head Key
class IBipedHeadKey : public IBipedKey {
	public :
		float head_blend;
};


// Biped Prop  Key
#define WORLD_PROP	0
#define BODY_PROP	1
#define RHAND_PROP	2
#define LHAND_PROP	3

class IBipedPropKey : public IBipedKey {
	public:
		int pos_space;
		int rot_space;
};


//Biped FootStep Key
#define FS_LSEL			(1<<0)
#define FS_RSEL			(1<<1)

#define FS_LFT				1
#define FS_RGT				0

class IBipedFSKey : public IKey {
	public:
		DWORD		edgeSel;
		BOOL		active;
		Matrix3		mat;
		int			side;	// LFT, RGT		
		TimeValue	duration;
};


// Multiple footstep params. One will be created to store each gait's paramters (walk, run, jump)
class MultFprintParams: public MaxHeapOperators  {
    public:
    int numnewfprints;
    float aswid;
    float pswid;
    float aslen;
    float pslen;
    float ashgt;
    int cycle; 
    float aslen2;
    float pslen2;
    float ashgt2;
    int cycle2;
    int AutoTiming;
    int InterpTiming;
    int Alternate;
    int MultiInsertInTime;  
    MultFprintParams(int gait) {init(gait);}
    BIPExport void init(int gait);
};

// returns multiple footsteps parameters for different types of gaits (walk, run, jump)
BIPExport MultFprintParams* GetMultFprintParams(int gait);

// Start Left/Start Right radio buttons in "Create Mutliple Footsteps Dialog"
BIPExport int	GetFSAddSide();
BIPExport void	SetFSAddSide(int side);


