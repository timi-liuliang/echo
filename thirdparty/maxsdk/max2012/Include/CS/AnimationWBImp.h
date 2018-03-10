/*==============================================================================

  file:     AnimationWBImp.h

  author:   Michael Zyracki

  created:  2003

  description:

    Interface definition for the CS Animation Workbench

  modified:	


© 2003//2004 Autodesk
==============================================================================*/
#pragma once

#include "..\ifnpub.h"
#include "..\interval.h"
#include "..\strbasic.h"

//! \brief Interface ID for the IAnalyzer Interface
//! \see IAnalyzer
#define ANALYZER_INTERFACE Interface_ID(0x14bf34cc, 0x519c0633)

//! \brief Interface ID for the IFixer Interface
//! \see IFixer
#define FIXER_INTERFACE Interface_ID(0xfcd6b18, 0x6e714e23)

//! \brief Interface ID for the IFilter Interface
//! \see IFilter
#define FILTER_INTERFACE Interface_ID(0x36ca302f, 0x23d147a6)

//! \brief Interface ID for the IWorkBench Interface
//! \see IWorkbench
#define WORKBENCH_INTERFACE Interface_ID(0x78aa2c29, 0x19a55d39)

//! \brief Interface ID for the IWorkBench2 Interface
//! \see IWorkbench
#define WORKBENCH_INTERFACE2 Interface_ID(0x5a6c11a8, 0x706b0c70)

//! \brief The interface to the CS Workbench.

//! This class contains all of the functionality found in the UI of the workbench. Note that these functions
//! also affect the drawing and viewing of CS curves in the normal trackview.  Thus you can extend the
//! normal curve editor, via quad menus, etc.. to basically work like the workbench.
//! You get this interface by calling GetCOREInterface(WORKBENCH_INTERFACE)
//! \see IWorkbench2
//! \see IAnylyzer
//! \see IFixer
//! \see IFilter
class IWorkBench : public FPStaticInterface
{
public:
	
	DECLARE_DESCRIPTOR(IWorkBench);
	//! \brief Open the Workbench dialog
	virtual void Open(); //pops up in viewprt
	//! \brief Toggle showing the bipeds fcurve x axis
	virtual void ToggleShowX();
    //! \brief Retrieves display status of biped's fcurve X axis. 
    //! \return TRUE if X axis is displayed, otherwise FALSE
	virtual BOOL GetShowX();
	//! \brief Toggle showing the bipeds fcurve y axis
	virtual void ToggleShowY();
    //! \brief Retrieves display status of biped's fcurve Y axis. 
    //! \return TRUE if X axis is displayed, otherwise FALSE
	virtual BOOL GetShowY();
	//! \brief Toggle showing the bipeds fcurve z axis
	virtual void ToggleShowZ();

	//! \brief Retrieves display status of biped's fcurve Z axis. 
	//! \return TRUE if Z axis is displayed, otherwise FALSE
	virtual BOOL GetShowZ();

	//! \brief Toggle showing the layer edit tool
	virtual void ToggleLayerEdit();
	//! \brief Get whether or not the z axis is shown
	virtual BOOL GetLayerEdit();

	//! \brief Toggle interactive drawing of the biped fcurve
	virtual void ToggleDrawDuringMove();
	//! \brief Get whether or not we interatively when moving the biped fcurve
	virtual BOOL GetDrawDuringMove();

	//! \brief Toggle limiting biped euler curves to -180/180
	virtual void ToggleLimit180();
	//! \brief Get whether or not euler curves are limited to -180/180
	virtual BOOL GetLimit180();

	//! \brief Show the biped quaternion curves
	virtual void ShowQuatCurve();
	//! \brief Show the biped position curves
	virtual void ShowPosCurve();
	//! \brief Show the biped angular speed curves
	virtual void ShowAngSpeedCurve();
	//! \brief Show the biped angular acceleration curves
	virtual void ShowAngAccelCurve();
	//! \brief Show the biped angular jerk curves
	virtual void ShowAngJerkCurve();
	//! \brief Show the biped position speed curves
	virtual void ShowPosSpeedCurve();
	//! \brief Show the biped position acceleration curves
	virtual void ShowPosAccelCurve();
	//! \brief Show the biped position jerk curves
	virtual void ShowPosJerkCurve();

	//! \brief Show the biped position curve relative to the world origin
	virtual void PosCurveToWorld();
	//! \brief Show the biped position curve relative to the biped's root node.
	virtual void PosCurveToBipRoot();
	//! \brief Show the biped position curve relative to the passed in node
	//! \param[in] node - The node the curve is relative to.
	virtual void PosCurveToThisNode(INode *node);

	enum WorkBenchFunctions
	{
		toggleShowX = 0,getShowX,toggleShowY,getShowY,toggleShowZ,getShowZ,
		toggleLayerEdit,getLayerEdit,toggleDrawDuringMove,getDrawDuringMove,
		toggleLimit180,getLimit180,
		showQuatCurve,showPosCurve,showAngSpeedCurve,showAngAccelCurve,showAngJerkCurve,
		showPosSpeedCurve,showPosAccelCurve,showPosJerkCurve,posCurveToWorld, posCurveToBipRoot,
		posCurveToThisNode,open
	};

	BEGIN_FUNCTION_MAP
			VFN_0(toggleShowX,ToggleShowX);
			FN_0(getShowX,TYPE_BOOL,GetShowX);
			VFN_0(toggleShowY,ToggleShowY);
			FN_0(getShowY,TYPE_BOOL,GetShowY);
			VFN_0(toggleShowZ,ToggleShowZ);
			FN_0(getShowZ,TYPE_BOOL,GetShowZ);
			VFN_0(toggleLayerEdit,ToggleLayerEdit);
			FN_0(getLayerEdit,TYPE_BOOL,GetLayerEdit);
			VFN_0(toggleDrawDuringMove,ToggleDrawDuringMove);
			FN_0(getDrawDuringMove,TYPE_BOOL,GetDrawDuringMove);
			VFN_0(toggleLimit180,ToggleLimit180);
			FN_0(getLimit180,TYPE_BOOL,GetLimit180);
			VFN_0(showQuatCurve,ShowQuatCurve);
			VFN_0(showPosCurve,ShowPosCurve);
			VFN_0(showAngSpeedCurve,ShowAngSpeedCurve);
			VFN_0(showAngAccelCurve,ShowAngAccelCurve);
			VFN_0(showAngJerkCurve,ShowAngJerkCurve);
			VFN_0(showPosSpeedCurve,ShowPosSpeedCurve);
			VFN_0(showPosAccelCurve,ShowPosAccelCurve);
			VFN_0(showPosJerkCurve,ShowPosJerkCurve);
			VFN_0(posCurveToWorld,PosCurveToWorld);
			VFN_0(posCurveToBipRoot,PosCurveToBipRoot);
			VFN_1(posCurveToThisNode,PosCurveToThisNode,TYPE_INODE);
			VFN_0(open,Open);

	END_FUNCTION_MAP


};



//! \brief Additional interface for the CS Workbench.

//! This class contains new functionality found in the UI of the workbench. 
//! You get this interface by calling GetCOREInterface(WORKBENCH2_INTERFACE)
//! \see IWorkbench
class IWorkBench2 : public IWorkBench
{
public:
	DECLARE_DESCRIPTOR(IWorkBench2); 

	//! \brief Show Quat fcurve as XYZ Euler.
	virtual void ShowQuatXYZ();
	//! \brief Show Quat fcurve as XZY Euler.
	virtual void ShowQuatXZY();
	//! \brief Show Quat fcurve as YXZ Euler.
	virtual void ShowQuatYXZ();
	//! \brief Show Quat fcurve as YZK Euler.
	virtual void ShowQuatYZX();
	//! \brief Show Quat fcurve as ZXY Euler.
	virtual void ShowQuatZXY();
	//! \brief Show Quat fcurve as ZYX Euler.
	virtual void ShowQuatZYX();

	enum WorkBenchFunctions
	{
		showQuatXYZ = IWorkBench::open,
		showQuatXZY,showQuatYXZ,showQuatYZX,showQuatZXY,showQuatZYX
	};

	BEGIN_FUNCTION_MAP_PARENT(IWorkBench)
			VFN_0(showQuatXYZ,ShowQuatXYZ);
			VFN_0(showQuatXZY,ShowQuatXZY);
			VFN_0(showQuatYXZ,ShowQuatYXZ);
			VFN_0(showQuatYZX,ShowQuatYZX);
			VFN_0(showQuatZXY,ShowQuatZXY);
			VFN_0(showQuatZYX,ShowQuatZYX);
	END_FUNCTION_MAP
};


//! \brief  Interface for perfoming workbench analysis functions.

//! This class peforms workbench functions related to analysis of biped fcurves.
//! After analysis, a user may want to perform fixing \see IFixer.
//! You get this interface by calling GetCOREInterface(ANALYZER_INTERFACE)
//! \see IWorkbench
class IAnalyzer : public FPStaticInterface
{
public: 
	DECLARE_DESCRIPTOR(IAnalyzer); 

	//! \brief  Perform Noise Detection on the passed in biped nodes.
	//! \param[in] nodesToAnalyze The nodes which to anlyze.
	//! \param[in] range Interval over which to do the search.
	//! \param[in] deviation Percentage from the curves standard deviation that will trigger a detection.
	//! \param[in] angular Perform it on the angular or position changes.
	//! \param[in] noiseType 0 is speed, 1 is acceleration, 2 is jerk.
	//! \param[in] pNode Only valid for calculating position noise detection, the angular param is false.
	//! This is the parent from which the positions will be calculated related to. If NULL, then it will use the world as the parent.
	virtual void  DoNoiseDetectorAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float deviation,BOOL angular,int noiseType,INode *pNode);
	
	//! \brief Perform Spike Detection on the passed in biped nodes.
	//! \param[in] nodesToAnalyze - The nodes which to anlyze.
	//! \param[in] range -  Interval over which to do the search.
	//! \param[in] deviation -  Percentage from the curves standard deviation that will trigger a detection.
	virtual void  DoSpikeDetectorAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float deviation);
	
	//! \brief Perform Knee Wobble on the passed in biped nodes.
	//! \param[in] nodesToAnalyze The nodes which to anlyze. Only works on biped knee nodes. 
	//! Does nothing for non-biped non-knee nodes.
	//! \param[in] range Interval over which to do the search.
	//! \param[in] frameThreshold How much time, in terms of frames, that the calculation will use to check for the wobble.
	//! \param[in] fluctuationThreshold How much deviation will that will trigger a detection. In degrees, from 0 to 360.
	virtual void  DoKneeWobbleAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float frameThreshold,float fluctuationThreshold);
	
	//! \brief Perform Knee Extenstion detection on the passed in biped nodes.
	//! \param[in] nodesToAnalyze The nodes which to anlyze.  Only works on biped knee nodes.
	//! \param[in] range Interval over which to do the search.
	//! \param[in] kneeAngle How much knee bend beyond to look for. In degrees from 0 to 360.
	virtual void  DoKneeExtensionAnalysis(Tab<INode *>&nodesToAnalyze,Interval range,float kneeAngle);
	
	//! \brief Get The results of any analysis.
	//! \param[in] node The nodes which to get the results for.
	//! \return The times where the most recent analysis returned true.
	virtual Tab<TimeValue >  GetResults(INode *node);

	//! \brief Load an analysis file
	//! \param[in] filename The full path and name of the file.  If file doesn't exist, the function does nothing.
	virtual void LoadAnalysisFile(char *filename);
	
	//! \brief Save the analysis file
	//! \param[in] nodes Save the current analysis on these nodes
	//! \param[in] filename The full path and name of the file. Creates the file if it doesn't exists.
	virtual void SaveAnalysisFile(Tab<INode *> &nodes,char *filename);
	
	//! \brief Clear all of the analysis results. Does so for every node.
	virtual void ClearAnalysisResults();

	enum AnalyzeFunctions
	{
		doNoiseDetectorAnalysis = 0,
		doSpikeDetectorAnalysis,
		getResults,
		loadAnalysisFile,
		saveAnalysisFile,
		clearAnalysisResults,
		doKneeWobbleAnalysis,
		doKneeExtensionAnalysis
	};
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
			VFN_6(doNoiseDetectorAnalysis, DoNoiseDetectorAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_INODE);
			VFN_3(doSpikeDetectorAnalysis, DoSpikeDetectorAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT);
			FN_1(getResults,TYPE_TIMEVALUE_TAB_BV,GetResults,TYPE_INODE);
			VFN_1(loadAnalysisFile,LoadAnalysisFile,TYPE_FILENAME);
			VFN_2(saveAnalysisFile,SaveAnalysisFile,TYPE_INODE_TAB_BR,TYPE_FILENAME);
			VFN_0(clearAnalysisResults,ClearAnalysisResults);
			VFN_4(doKneeWobbleAnalysis, DoKneeWobbleAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT,TYPE_FLOAT);
			VFN_3(doKneeExtensionAnalysis, DoKneeExtensionAnalysis,TYPE_INODE_TAB_BR,TYPE_INTERVAL,TYPE_FLOAT);
	END_FUNCTION_MAP
	#pragma warning(pop)
};

//! \brief Interface for perfoming workbench fixer functions

//! This class peforms workbench functions related to the fixing of fcurves.  
//! Fixing is done after analysis is done and works by fixing the analysis detection locations. \see IAnalysis
//! You get this interface by calling GetCOREInterface(FIXER_INTERFACE)
//! \see IWorkbench
class IFixer : public FPStaticInterface
{
public:
	DECLARE_DESCRIPTOR(IFixer); 

	//! \brief Perform Angular Smooth fixing on the passed in biped nodes.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] width How many frames to do the fixing over.
	//! \param[in] damping Damping value.
	virtual void DoAngSmoothing(Tab<INode *>&nodes,int width, float damping);
	
	//! \brief Perform Angular Blur fixing on the passed in biped nodes.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] width How many frames to do the fixing over.
	//! \param[in] damping Damping value.
	virtual void DoAngBlurring(Tab<INode *>&nodes,int width, float damping);
	
	//! \brief Perform Advanced Angular Smooth fixing on the passed in biped nodes.
	//! \param[in] nodes - The nodes which to fix.
	//! \param[in] width - How many frames to do the fixing over.
	//! \param[in] damping - Damping value.
	virtual void DoAdvAngSmoothing(Tab<INode *>&nodes,int width, float damping);
	
	//! \brief Perform Position Smooth fixing on the passed in biped nodes.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] width How many frames to do the fixing over.
	//! \param[in] damping Damping value.
	virtual void DoPosSmoothing(Tab<INode *>&nodes,int width, float damping);

	//! \brief Perform Position Blur fixing on the passed in biped nodes.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] width How many frames to do the fixing over.
	//! \param[in] damping Damping value.
	virtual void DoPosBlurring(Tab<INode *>&nodes,int width, float damping);
	
	//! \brief Perform Remove Key fixing on the passed in biped nodes. Removes keys as the fix.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] intervalWidth How many frames to do the fixing over.
	//! \param[in] deleteKeys Whether or not to delete the keys or just move them as the fix.
	virtual void DoRemoveKeys(Tab<INode *>&nodes,int intervalWidth,BOOL deleteKeys);
	
	//! \brief Perform Knee Wobble Fix.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] frameThreshold How much time, in terms of frames, that the calculation will use to fix the wobble.
	//! \param[in] fluctuationThreshold How much deviation will that will trigger a detection. In degrees, from 0 to 360.
	virtual void DoKneeWobbleFix(Tab<INode *>&nodes,float frameThreshold,float fluctuationThreshold);
	
	//! \brief Perform Knee Extension Fix.
	//! \param[in] nodes The nodes which to fix.
	//! \param[in] kneeAngle Knee Angle. In degrees, from 0 to 360.
	virtual void DoKneeExtensionFix(Tab<INode *>&nodes,float kneeAngle);


	enum FilterFunctions
	{
		doAngSmoothing = 0,
		doAngBlurring,
		doAdvAngSmoothing,
		doPosSmoothing,
		doPosBlurring,
		doRemoveKeys,
		doKneeWobbleFix,
		doKneeExtensionFix
	};

	BEGIN_FUNCTION_MAP
			VFN_3(doAngSmoothing, DoAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doAngBlurring, DoAngBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doAdvAngSmoothing, DoAdvAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);

			VFN_3(doPosSmoothing, DoPosSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doPosBlurring, DoPosBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT);
			VFN_3(doRemoveKeys, DoRemoveKeys,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_BOOL);
			VFN_3(doKneeWobbleFix, DoKneeWobbleFix,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_FLOAT);
			VFN_2(doKneeExtensionFix, DoKneeExtensionFix,TYPE_INODE_TAB_BR,TYPE_FLOAT);
	END_FUNCTION_MAP
};


//! \brief Interface for perfoming workbench filter functions
//! This class peforms workbench functions related to the filtering of fcurves.  
//! Filter can occur without any previous analysis.
//! You get this interface by calling GetCOREInterface(FILTER_INTERFACE)
//! \see IWorkbench
class IFilter : public FPStaticInterface
{
public:	
	DECLARE_DESCRIPTOR(IFilter); 

	//! \brief Perform Angular Smooth filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	
	virtual void DoAngSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Angular Blur filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	virtual void DoAngBlurring(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//!\brief  Perform Angular Boost filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	virtual void DoAngBoosting(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Advanced Angular Smooth filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	virtual void DoAdvAngSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Position Smooth filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range -Range over to perform the filter.
	virtual void DoPosSmoothing(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Position Blur filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	virtual void DoPosBlurring(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Position Boost filtering on the passed in biped nodes.
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] width How many frames to do the filtering over.
	//! \param[in] damping Damping value.
	//! \param[in] range Range over to perform the filter.
	virtual void DoPosBoosting(Tab<INode *>&nodes,int width, float damping,Interval range);
	
	//! \brief Perform Key Reduction filtering on the passed in biped nodes. Similar to mocap key reduction
	//! \param[in] nodes The nodes which to filter.
	//! \param[in] tolerance The maximun angle deviation for the non-COM(Center Of Mass) curves.
	//! \param[in] keySpacing Minimum key spacing.
	//! \param[in] COMTolerance Maximum position deviation for the COM(Center of Mass, the biped root) curves.
	//! \param[in] COMKeySpacing Minimum key spacing for the COM(Center Of Mass,the biped root) curves.
	//! \param[in] range Range over to perform the filter.
	virtual void DoKeyReduction(Tab<INode *>&nodes,float tolerance,int keySpacing, float COMTolerance,
		float COMKeySpacing,Interval range);
	
	//! \brief Add a key per frame over the passsed in nodes.
	//! \param[in] nodes The nodes which to filter.
	virtual void DoKeyPerFrame(Tab<INode *>&nodes);
	
	//! \brief Enable the Position Sub Anims for the passed in nodes.
	//! \param[in] nodes The nodes which to enable.
	//! \param[in] enable TRUE - enable, FALSE - disable.
	virtual void EnablePosSubAnim(Tab<INode *> &nodes,BOOL enable);
	
	//! \brief Enable the Rotation Sub Anims for the passed in nodes.
	//! \param[in] nodes The nodes which to enable.
	//! \param[in] enable TRUE - enable, FALSE - disable.
	virtual void EnableRotSubAnim(Tab<INode *> &nodes,BOOL enable);
	
	//! \brief Enable the Scale Sub Anims for the passed in nodes.
	//! \param[in] nodes The nodes which to enable.
	//! \param[in] enable TRUE - enable, FALSE - disable.
	virtual void EnableScaleSubAnim(Tab<INode *> &nodes,BOOL enable);
	
	//! \brief Collapse the Position Sub Anims for the passed in nodes.
	//! \param[in] nodes The nodes which to collapse.
	//! \param[in] perFrame TRUE - create a key per frame on the biped, FALSE - only create keys where the subanim had keys 
	//! \param[in] deleteSubAnim TRUE - delete the subnim, FALSE - leave the subanim with a weight of 0 in the list.
	virtual void CollapsePosSubAnim(Tab<INode *> &nodes,BOOL perFrame, BOOL deleteSubAnim);
	
	//! \brief Collapse the Rotation Sub Anims for the passed in nodes.
	//! \param[in] nodes The nodes which to collapse.
	//! \param[in] perFrame TRUE - create a key per frame on the biped, FALSE - only create keys where the subanim had keys 
	//! \param[in] deleteSubAnim TRUE - delete the subnim, FALSE - leave the subanim with a weight of 0 in the list.
	virtual void CollapseRotSubAnim(Tab<INode *> &nodes, BOOL perFrame,BOOL deleteSubAnim);
	
	//! \brief Create Position Sub Anims for the passed in nodes based upon the passed in controller.
	//! \param[in] nodes The nodes which to create subanims on.
	//! \param[in] toClone The control to clone.
	//! \param[in] checkIfOneExists  1 - check if one exists of the same class as toClone, if so don't create 0 - always create it.
	virtual void CreatePosSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	
	//! \brief Create Rotation Sub Anims for the passed in nodes based upon the passed in controller.
	//! \param[in] nodes The nodes which to create subanims on.
	//! \param[in] toClone The control to clone.
	//! \param[in] checkIfOneExists TRUE - check if a control of the same class as toClone exists, if so don't create, FALSE - always create it.
	virtual void CreateRotSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	
	//! \brief Create Scale Sub Anims for the passed in nodes based upon the passed in controller.
	//! \param[in] nodes The nodes which to create subanims on.
	//! \param[in] toClone The control to clone.
	//! \param[in] checkIfOneExists TRUE - check if a control of the same class as toClone exists, if so don't create, FALSE - always create it.
	virtual void CreateScaleSubAnim(Tab<INode *> &nodes,Control *toClone,BOOL checkIfOneExists);
	
	//! \brief Perform Knee Wobble Filter.
	//! \param[in] nodes The nodes which to filter. Only works on biped knee nodes.
	//! \param[in] frameThreshold How much time, in terms of frames, that the calculation will use to check for the wobble.
	//! \param[in] fluctuationThreshold How much deviation will that will trigger a detection. In degrees, from 0 to 360.
	//! \param[in] range Range over to perform the filter.
	virtual void DoKneeWobbleFilter(Tab<INode *>&nodes,float frameThreshold,float fluctuationThreshold,Interval range);
	
	//! \brief Perform Knee Extension Filter.
	//! \param[in] nodes The nodes which to filter.  Only works on biped knee nodes.
	//! \param[in] kneeAngle How much knee bend beyond to look for. In degrees from 0 to 360.
	//! \param[in] range Interval over which to do the search.
	virtual void DoKneeExtensionFilter(Tab<INode *>&nodes,float kneeAngle,Interval range);

	enum FilterFunctions
	{
		doAngSmoothing = 0,
		doAngBlurring,doAngBoosting,doAdvAngSmoothing,doPosSmoothing,
		doPosBlurring,doPosBoosting,doKeyReduction,doKeyPerFrame,
		enablePosSubAnim,enableRotSubAnim,enableScaleSubAnim,
		collapsePosSubAnim,collapseRotSubAnim,createPosSubAnim,createRotSubAnim,
		createScaleSubAnim,doKneeWobbleFilter,doKneeExtensionFilter
	};

	BEGIN_FUNCTION_MAP
			VFN_4(doAngSmoothing, DoAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAngBlurring, DoAngBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAngBoosting, DoAngBoosting,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doAdvAngSmoothing, DoAdvAngSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);

			VFN_4(doPosSmoothing, DoPosSmoothing,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doPosBlurring, DoPosBlurring,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_4(doPosBoosting, DoPosBoosting,TYPE_INODE_TAB_BR,TYPE_INT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_6(doKeyReduction, DoKeyReduction,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_INT,TYPE_FLOAT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_1(doKeyPerFrame, DoKeyPerFrame,TYPE_INODE_TAB_BR);
			VFN_2(enablePosSubAnim, EnablePosSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_2(enableRotSubAnim, EnableRotSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_2(enableScaleSubAnim, EnableScaleSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL);
			VFN_3(collapsePosSubAnim, CollapsePosSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL,TYPE_BOOL);
			VFN_3(collapseRotSubAnim, CollapseRotSubAnim,TYPE_INODE_TAB_BR,TYPE_BOOL,TYPE_BOOL);
			VFN_3(createPosSubAnim, CreatePosSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_3(createRotSubAnim, CreateRotSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_3(createScaleSubAnim, CreateScaleSubAnim,TYPE_INODE_TAB_BR,TYPE_CONTROL,TYPE_BOOL);
			VFN_4(doKneeWobbleFilter, DoKneeWobbleFilter,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_FLOAT,TYPE_INTERVAL);
			VFN_3(doKneeExtensionFilter, DoKneeExtensionFilter,TYPE_INODE_TAB_BR,TYPE_FLOAT,TYPE_INTERVAL);
	END_FUNCTION_MAP
};