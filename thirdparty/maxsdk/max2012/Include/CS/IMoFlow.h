/*********************************************************************
 *<
	FILE: bipedapi.h

	DESCRIPTION: Biped motion flow interface classes

	CREATED BY:	Ravi Karra, Michael Zyracki

	HISTORY: Created 18 October 1999
			
 *>	Copyright (c) 1999-2003 All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\maxheap.h"
#include "BipExp.h"
#include "keytrack.h"
#include "CSConstants.h"
#include "..\assetmanagement\AssetUser.h"
#include "..\point2.h"
#include "..\ipoint2.h"

/*
	Using the Motion Flow classes.
	1. You need to make sure you are linking with the biped.lib import library. This is only currently necessary
	for linking with these motion flow clases. If you are just using the interface in BipedApi.h,there is no
	need to link with this import library.
	2. Get the IMoFlow Interface from a corresponding BipMaster.
	3. The general MotionFlow class hierarcy.
		A.  Each Motion Flow consists of a collection of Snippets(stored animation clips) and Scripts(instances
		of the Snippets strung together.).
		B. Each Snippet Contains an animation (a .bip file), and a collection of Transistions.
		C. Each Transistion specifies an one way motion blend from one snippet to another.
		D. Each Script in the IMoFlow is a set of animations(snippets), which are  contains a list of ScriptItem's.
		E. Each ScriptItem contains a pointer back to the Snippet it instances.

	NOTE that there is a close correspondence between these SDK classes and functions and the CS motion
	flow maxscript implementation, so that may be used as reference for this SDK also.

	NOTE That these classes contain internal biped classes.  These classes are the actually ones used
	internally, they aren't interfaces.
	
	Example of creating a motion flow.
		IBipMaster *bipMaster = GetCurrentBipMaster(); //see
		POINT graphLocation;
		char str[32];
		//begin motion flow mode.
		bipMaster->BeginModes(BMODE_MOTIONFLOW);
		//get the motion flow
		IMoFlow *moFlow = bipMaster->GetMoFlow();
		//create a walk snippet
		sprintf(str,"c:\\Sumo\\walk.bip"); //the file for the snippert
		graphLocation.x = 20;
		graphLocation.y = 10;
		Snippet *walk =  moFlow->NewSnippet(str, graphLocation,true,true);
		//create a run snippet
		sprintf(str,"c:\\Sumo\\run.bip"); //the file for the snippert
		graphLocation.x = 50;
		graphLocation.y = 10;
		Snippet *run =  moFlow->NewSnippet(str, graphLocation,true,true);
		//create an optimized transistion between the 2 snippets.
		walk->AddTransition(run,TRUE);
		run->AddTransition(walk,TRUE);
		//call this global function to make sure all graphs and moflows are updated.
		UpdateMotionFlowGraph();
		//create a script and set it to be active
		sprintf(str,"SDK Script");
		moFlow->AddScript(str,true);
		//get the script
		Script *script	= moFlow->GetActiveScript();
		//set the start pos and direction
		Point3 startPos(0,0,0);
		script->SetStartPos(startPos);
		script->SetStartRot(0); //set a 0 heading degree
		//create the snipped
		script->AddSnippet(run);
		moFlow->CurscriptToActiveLinks(); //set the transistions in the script 
		script->AddSnippet(walk);
		moFlow->CurscriptToActiveLinks(); //set the transistions in the script 
		script->AddSnippet(run);
		moFlow->CurscriptToActiveLinks(); //set the transistions in the script 
		//now compute the animation.
		moFlow->ComputeAnimation(TRUE);
		//end the motion flow mode
		//unify the motion(convert the motion to free form)
		bipMaster->UnifyMotion()
		bipMaster->EndModes(BMODE_MOTIONFLOW);
		return;

*/


// Forward Declarations--note that there are certain classes here (class BipMaster, class animal, etc..)
// which are internal classes.
class Snippet;    
class MoFlow;
//Internal classes
class BipMaster;
class Transition;
class MFL_IOProcessor;
class animal;
class Layer;
class path_properties;
class crowd_script;

//The Transistion Info. Each transistion contains an array of traninfos.
class TranInfo: public MaxHeapOperators {

  public:
	//data
    int start1,start2,length;  //source and dest start, and length
    int rolling1,rolling2,randpct;
	int transFocus;
    float angle,easein,easeout,cost;
	float flow_angle;
	path_properties prop;
    MCHAR note[MAXTRANNOTE];

    BIPExport		TranInfo();
	BIPExport		TranInfo& operator=(const TranInfo& TI);
	BIPExport		int operator==(const TranInfo& TI);
    
	//internal I/O functions
	void			Output(MFL_IOProcessor *IOProc);
    void			Input(MFL_IOProcessor *IOProc);

	inline int		GetSourceStart() const	{ return start1; }
	inline void		SetSourceStart(int s)	{ start1 = s; }
	inline int		GetDestStart() const	{ return start2; }
	inline void		SetDestStart(int s)		{ start2 = s; }	
	inline int		GetSourceState() const	{ return rolling1; }
	inline void		SetSourceState(int s)	{ rolling1 = s; }
	inline int		GetDestState()  const	{ return rolling2; }
	inline void		SetDestState(int s)		{ rolling2 = s; }
	inline int		GetLength()	 const		{ return length; }
	inline void		SetLength(int l)		{ length = l; }
	inline float	GetAngle()  const		{ return angle; }
	inline void		SetAngle(float a)		{ angle = a; }
	inline int		GetRandPercent() const	{ return randpct; }
	inline void		SetRandPercent(float r)	{ randpct = (int)r; }
	inline float	GetCost() const			{ return cost; }
	inline void		SetCost(float c)		{ cost = c; }
	inline float	GetEaseIn()	 const		{ return easein; }
	inline void		SetEaseIn(float in)		{ easein = in; }
	inline float	GetEaseOut() const		{ return easeout; }
	inline void		SetEaseOut(float out)	{ easeout = out; }
	inline int		GetTransFocus()	 const	{ return transFocus; }
	inline void		SetTransFocus(int t)	{ transFocus = t; }
	inline const	MCHAR* GetNote() const	{ return note; }
	inline void		SetNote(const MCHAR* n) { _tcscpy_s(note, MAXTRANNOTE, n); }
};

class Transition: public MaxHeapOperators {
  public:
	Snippet *from;
    Snippet *to;
    int active;
    int selected;
    int start1,start2,length;
    int rolling1,rolling2;
    float easein,easeout;
    float angle;
	float flow_angle;
	int transFocus;
	path_properties prop;
	int randpct;
	float cost;
    int index;
    int nTransInfo;
	int maxTransInfo;
    TranInfo *TranList;
    MCHAR note[MAXTRANNOTE];
	HWND hwnd;

    BIPExport		Transition();
	BIPExport		~Transition();
	BIPExport Transition& operator=(const Transition& T);
    BIPExport void	SetTranListSize(int NUM);
    BIPExport void	InfoToTransition(int i);
    BIPExport void	TransitionToInfo(int i);
    BIPExport int	NewTranInfo(); // returns the index of the inserted item
    BIPExport void	ShiftTranInfo(int TIindex, int storecurrent);
    BIPExport void	DeleteTranInfo(int TIindex);
    void			Output(MFL_IOProcessor *IOProc);
    void			Input(MFL_IOProcessor *IOProc);

	inline int		GetRandPercent() const	{ return randpct; }
	inline void		SetRandPercent(float r)	{ randpct = (int)r; }
	inline float	GetCost() const			{ return cost; }
	inline void		SetCost(float c)		{ cost = c; }
	
	inline int		GetTranListSize() const	{ return nTransInfo; }
	inline Snippet* GetFromSnippet() const	{ return from; } 
	inline void		SetFromSnippet(Snippet* s) { from = s; }

	inline Snippet* GetToSnippet() const	{ return to; }
	inline void		SetToSnippet(Snippet* s){ to = s; }

	inline BOOL		GetActive() const		{ return active; }
	inline void		SetActive(BOOL act)		{ active = act; }

	inline BOOL		GetSelected() const		{ return selected; }
	inline void		SetSelected(BOOL sel)	{ selected = sel; }

	inline TranInfo GetTranInfo(int TIindex) const 
						{ assert (TIindex < nTransInfo); return TranList[TIindex]; }
	inline void		SetTranInfo(int TIindex, TranInfo ti) {
						assert (TIindex < nTransInfo); 
						TranList[TIindex] = ti; 
						if (index == TIindex ) InfoToTransition(index); 
					}

	BIPExport void	UpdateUI(bool redraw=false); 
};

class Script;

class Snippet: public MaxHeapOperators {
  public:
	int visited; //used for depth first tranversal
	float distance_to_stop;
	float distance_to_loop;
    int start;
    int end;
    MCHAR sname[MAXNAME];
    MaxSDK::AssetManagement::AssetUser file;
    int posx;
    int posy;
    int width;
    int nTrans;
    int curtime; // set anytime you compute A for a particular frame (necessary for looping)
    int active;
    int validload; // currently loaded file, although possibly inactive, is still valid (or not)
    BOOL randstart;    // can this snippet start a random traversal script
	int  randstartpct; // percentage of time this snippet starts a random traversal script    
	Transition *Transitions;
	HWND hwnd;
	int orgposx; // for interaction - post-fido
	int orgposy; // for interaction - post-fido

  public:
    Snippet *next;
    animal *A;
  
	BIPExport		Snippet(); 
	BIPExport		virtual ~Snippet();
    BIPExport void	ClearActiveTransitions();
    BIPExport void	ClearSelectedTransitions();
    BIPExport int	ActivateTransitionTo(Snippet *toSN);
    BIPExport Transition *GetTransitionTo(Snippet *toSN);
    BIPExport int	GetTransitionIndex(Transition *theTR);
    BIPExport float	AddTransition(Snippet *child,  int optimize); // returns the floating point cost
    BIPExport float	ComputeTransitionPoints(Transition *nTR, Snippet *Src , Snippet *Dest, int preference, int optimize);
	BIPExport float	ComputeOptimalTransitionPoints(BOOL SearchAll,int PrefTranLen,int SearchBefore, int SearchAfter, Transition *nTR, Snippet *Src , Snippet *Dst);
    BIPExport void	RecomputeTransitions();// recompute existing transitions from this new snippet
    BIPExport int	DeleteTransitionsTo(Snippet *child);
    BIPExport int	DeleteTransition(int index);
    BIPExport void	DeleteSelectedTransitions(MoFlow *MF);
    BIPExport int	IsChild(Snippet *Child);
    void			Paint(HDC hdc, int selected, int startnode, int editnode, int transhow);
    void			PaintTransitions(HDC hdc, BOOL transhow);
	void			ComputeWidth(HDC hdc, BOOL transhow);
    int				Inside(POINT &mp, float *dist);
	BIPExport void	UpdateUI();
	BIPExport Snippet& operator=(const Snippet& SN);    
    Transition*		TranHitTest(POINT &mp);
    void			TranRegionSelect(POINT &min, POINT &max, int set, int active);    
    void			Output(MFL_IOProcessor *IOProc);
    void			Input(MFL_IOProcessor *IOProc);
    BIPExport int	LoadFile(bool UpdateTheUI = true, int ErrorType = 3); // 3 => shows errors in dialog box
    Snippet         *GetNextRandScriptSnippet(int *transindex);
		
	
	         ///MG added for new MF based forward simulatiom
	Snippet *	NextRealTime(BipMaster *mc, Script *scr, animal *A, int frame, path_properties *desired_properties, int global_frame, int global_last_clip_start, int currentScript_index,  int *transindex, int *found);

	

	inline int		GetStart() const			{ return start; }
	inline void		SetStart(int s)				{ start = s; }
	inline int		GetEnd() const				{ return end; }
	inline void		SetEnd(int e)				{ end = e; }
	
	inline const	MCHAR* GetClipName() const	{ return sname; }
	inline void		SetClipName(const MCHAR* n) { _tcscpy_s(sname, MAXNAME, n); }

	inline const	MaxSDK::AssetManagement::AssetUser&  GetFile() const	{ return file; }
	inline void		SetFile(const MaxSDK::AssetManagement::AssetUser& assetUser) { file = assetUser; }
	
	inline virtual	IPoint2	GetPosition() const { return IPoint2(posx, posy); }
	inline virtual	void	SetPosition(IPoint2 p){ posx = p.x; posy = p.y; }
	inline BOOL		GetActive() const			{ return active; }
	inline void		SetActive(BOOL act)			{ active = act; }

	inline BOOL		GetRandStart() const		{ return randstart; }
	inline void		SetRandStart(BOOL rs)		{ randstart = rs; }
	inline BOOL		GetRandStartPercent() const	{ return randstartpct; }
	inline void		SetRandStartPercent(int rsp){ randstartpct = rsp; }
	
	inline int		NumTransitions()			{ return nTrans; }
	inline Transition* GetTransition(int Tindex) const 
						{ assert (Tindex < nTrans); return &Transitions[Tindex]; }
	inline void	SetTransition(int Tindex, Transition* ti) 
						{ assert (Tindex < nTrans); Transitions[Tindex] = *ti; }
};
		 

class ScriptItem: public MaxHeapOperators { 
  public:
    Snippet     *snip;
    int         transindex;
	vector      flow_pos;
	vector2D	flow_pivot;
    float       flow_yaw;
    int         glob_snipstart;
	MCHAR       *tempSnipName; // for global moflow i/o

	//MG added for CS4
	int foothold_frame[2];
	int footlift_frame[2];

	inline Snippet*	GetSnippet()			{ return snip; }
	inline void		SetSnippet(Snippet* s)	{ snip = s; }
	inline int		GetTransIndex() const	{ return transindex; }
	inline void		SetTransIndex(int i)	{ transindex = i; }
	inline vector	GetFlowPos() const		{ return flow_pos; }
	inline void		SetFlowPos(vector p)	{ flow_pos = p; }
	inline vector2D	GetFlowPivot() const	{ return flow_pivot; }
	inline void		SetFlowPivot(vector2D p){ flow_pivot = p; }
	inline float	GetFlowYaw() const		{ return flow_yaw; }
	inline void		SetFlowYaw(float y)		{ flow_yaw = y; }
	inline int		GetSnipStart() const	{ return glob_snipstart; }
	inline void		SetSnipStart(int s)		{ glob_snipstart = s; }

	
    BIPExport	ScriptItem();
	BIPExport	ScriptItem& operator=(const ScriptItem& SI);
};

class Script: public MaxHeapOperators {
  public:
    MCHAR name[MAXSCRIPTNAME];
    Point3 startpos;
    float startrot;
    int startframe;
    int nitems;
    int maxitems;
    ScriptItem *items;
    int nlayers;
    Layer *layers;
    Layer *EditLayer;
    Script *next;

    BIPExport		Script();
    BIPExport		~Script();
	BIPExport Script& operator=(const Script& P);
    BIPExport int	AddSnippet(Snippet *SN);
    BIPExport int	InsertSnippet(int index, Snippet *SN);
	BIPExport int	DeleteSnippet(int index);
	BIPExport int   Get_Backtrackframe(); //mg
    BIPExport int	GetScriptItemIndex(ScriptItem *SI);
	BIPExport int	InsertScriptItem(int index, ScriptItem *SI);
	BIPExport int   GetLastTransitionIndex(); //MG m28
	          Transition *GetLastTransition(); //S43
	inline	  int	NumScriptItems() { return nitems; }
	inline	  ScriptItem* GetScriptItem(int ind)
						{ assert (ind < nitems); return &items[ind]; }
    BIPExport int	IncludesSnippet(Snippet *SN);
    BIPExport int	IncludesTransition(Transition *TR);
			  void	Output(MFL_IOProcessor *IOProc);
			  void	Input(MFL_IOProcessor *IOProc, MoFlow *MF);	
	inline	  int	GetStartFrame() { return startframe; }
	inline	  void	SetStartFrame(int f) { startframe = f; }
	BIPExport void	Set_crowd_script(crowd_script *CS);
	inline	 Point3	GetStartPos() { return startpos; }
	inline	  void	SetStartPos(Point3 p) { startpos = p;}
	inline	  float	GetStartRot() { return startrot; }
	inline	  void	SetStartRot(float r) { startrot = r;}
	inline    const	MCHAR* GetName() const	{ return name; }
	inline    void	SetName(const MCHAR* n) { _tcscpy_s(name, MAXSCRIPTNAME, n); }

    // The following functions are obsolete
	inline	  int	NumLayers() { return nlayers; }
	inline	  Layer *GetEditLayer() { return EditLayer; }
	BIPExport Layer *GetLayer(int index);
    BIPExport int	AddLayer(Script *SCR, TimeValue t);
    BIPExport void	DeleteLayer(Layer *delLY);
    BIPExport void	SortLayer(Layer *SORTLY);
    BIPExport int	GetLayerIndex(Layer *LYR);
    BIPExport int	InsertLayer(Layer *newLY);
    BIPExport void	CloneLayer(Layer *orgLY,TimeValue t);
    BIPExport void	DeleteAllLayers();
};

class IBipMaster;
class IMoFlow: public MaxHeapOperators
{
  public:
	virtual ~IMoFlow() {;}
	virtual	int		NumScripts() const=0;
	virtual	int		NumSelected() const=0;			
	virtual	Script*	GetScript(int index)=0;
	virtual	Script*	GetActiveScript()=0;
	virtual void	SetActiveScript(Script* scr)=0;
	virtual	int		GetScriptIndex(Script *theSCR) const=0;
	virtual	int		AddScript(const MCHAR *newscriptname, bool activate=true)=0;
	virtual	void	DeleteScript(Script *delSCR)=0;
	virtual	void	DeleteScript(int ind)=0;
	virtual	void	DeleteAllScripts()=0;

	virtual	int		NumSnippets() const=0;
	virtual	const	BitArray& GetSelected() const=0;
	virtual	void	SetSelected(const BitArray& sel)=0;
	virtual	Snippet* GetSnip(int index)=0;
	virtual	Snippet* GetSnip(const MCHAR *str)=0;		
	virtual	int		GetIndex(Snippet *theSN) const=0;
	virtual	int 	LoadSnippetFiles(bool UpdateTheUI = true, int ErrorType = 3, bool UpdateSNLengths=false)=0;
	
	virtual	int		GetActiveStartFrame() const=0;
	virtual	int		GetActiveEndFrame() const=0;
	virtual	int		Save(const MCHAR *fname)=0;
	virtual	int		Load(const MCHAR *fname, bool Append=false, bool UpdateTheUI = true, int ErrorType = 3)=0;
	virtual	IBipMaster* GetMaster()=0;
	virtual void	ComputeAnimation(int redraw, int globalsToo = FALSE)=0; //main function that computes the moflow

	//NOTE if you add a snippet to the script this function must be called to make sure that
	//the transistion are all active!
	virtual void CurscriptToActiveLinks() = 0;
	// Adds a new snippet(clip) to the motion flow network, if load==true it's immediately loaded(if not need to call LoadSnipetFiles,
	//if redraw is true then the moflow graph is redraw
	virtual	Snippet* NewSnippet(const MCHAR *fname, POINT &mp, bool load=false, bool redraw=false)=0;
	virtual	void	DeleteSnip(Snippet *DelSnip)=0;

	virtual void	RedrawGraphWnd()=0; // update just motion flow editor
	virtual void	UpdateUI()=0; // updates the motionflow script dialog and motion flow editor
	virtual	void	HoldTrack(int HoldAnimals, BOOL HoldScriptOffsetOnly = false)=0; // HoldAll, holds all biped limbs.use to hold the biped.
					
};

BIPExport void UpdateMotionFlowGraph(); // updates the motionflow graph should be called after mf functions
										// that manipulate transistions

