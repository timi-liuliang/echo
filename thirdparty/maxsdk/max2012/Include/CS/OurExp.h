/******************************************************************************
 *<
	FILE: ourexp.h
				  
	DESCRIPTION:  Export Interface Functionality for Biped
	              This is used internally inside CS plugins
				  It is not for use by non-CS developers, since it contains all sorts of internal function calls
				  Other developers should use bipexp.h

	CREATED BY: Susan Amkraut 

	HISTORY: created February 2001

 *>     Copyright (c) Unreal Pictures, Inc. 1997, 2001 All Rights Reserved.
 *******************************************************************************/

#pragma once

#include "..\maxheap.h"
#include "bipexp.h" // included to avoid double definitions
#include "..\control.h"
// This is the interface ID for a Biped Controller Interface
#define I_OURINTERFACE	0x00100111


// IOurBipExport: This class can be returned by calling the method GetInterface() from a Biped controller
// Given controller *c points to a Biped controller, then:
// IOurBipExport *BipIface = (IOurBipExport *) (c->GetInterface(I_OURINTERFACE));
// will return the interface for this Biped Controller, else returns NULL.
#pragma warning(push)
#pragma warning(disable:4100)
class IOurBipExport: public MaxHeapOperators
{
	public:
		
		BIPExport virtual ~IOurBipExport() {}

        // For the BipedExport you got from the center of mass (root) controller, send in: 
        // VERTICAL_SUBANIM, HORIZONTAL_SUBANIM, and ROTATION_SUBANIM 
        // to get information for those tracks.
        // For other BipedExports this method is irrelevant.
		// The SetSubAnim method is actually included for future releases, 
		// when more methods will be added to the IOurBipExport class.
		// Then you could call these methods for the three center of mass subanims.
		// It is not presently useful.
        BIPExport virtual void SetSubAnim (int i) {};

		// call this from any IOurBipExport instance to remove or restore non uniform scaling
		BIPExport virtual void RemoveNonUniformScale(BOOL onOFF) {};

        // call these to begin and end figure mode
		BIPExport virtual void BeginFigureMode(int redraw) {};
		BIPExport virtual void EndFigureMode(int redraw) {};

		BIPExport virtual ScaleValue GetBipedScale(TimeValue t, INode *node){ return ScaleValue(); }
		BIPExport virtual Point3 GetBipedPosition(TimeValue t, INode *node){ return Point3(); }
		BIPExport virtual Quat GetBipedRotation(TimeValue t, INode *node){ return Quat(); }
		BIPExport virtual void SetBipedScale(const ScaleValue &scale, TimeValue t, INode *node){}
		BIPExport virtual void SetBipedPosition(const Point3 &p, TimeValue t, INode *node){}
        BIPExport virtual void SetBipedRotation(const Quat &q, TimeValue t, INode *node,  int global_reference){}
        BIPExport virtual void ScaleBiped(const float scale) {};
		BIPExport virtual float GetHeight() {return 1.0;}
		BIPExport virtual void GetVelocity(int frame, Point3 *vel) {};
		BIPExport virtual void GetProjectedVel(int frame, Point3 *vel) {};
		BIPExport virtual void SendDesiredProperties(int frame, Point3 *pos, Point3 *vel, Point3 *goal, int goal_exists, int stop_needed, int stop_time, float stop_distance, float speed_change){};
		BIPExport virtual void GetRedirectedVel(int frame, Point3 *desiredVel, Point3 *vel, Point3 *biped_skel_dir, int after_avoid_pass_flag, int avoid_needed) {};
		BIPExport virtual void SetAfterAvoidVel(int frame, Point3 *vel, Point3 *bip_vel_before_avoid, Point3 *bip_vel_after_avoid){};
		BIPExport virtual void EndFlow(int end_frame){};
		BIPExport virtual void InitializeFlow( Point3 *pos, Point3 *vel, float *max_speed, int start_frame, int use_random_start){};
		//BIPExport virtual void InitiateBackTrack(int start_frame, int last_frame){};
		BIPExport virtual void FinishBacktrack(){};
		BIPExport virtual void StartBacktrack(){}; 
		BIPExport virtual void FinishRetrace(){};
		BIPExport virtual void StartRetrace(){}; 
		BIPExport virtual void ClearPathsAhead(){};
		//BIPExport virtual void SendBackTrackingState(int  backtrack){};
		BIPExport virtual void GetBackTrackingFrame( int *backtrack_frame, int last_frame){};
		//BIPExport virtual void NumPossibleTransitions(int *num_transitions){};
	 	//BIPExport virtual void NumBlockedTransitions(int *num_transitions){};
		BIPExport virtual void GetPosition(int frame, Point3 *pos) {};
		BIPExport virtual void SetFlowHeight(int frame, float height) {};
		BIPExport virtual BOOL IsReadyForCrowdComputation(int starttype) {return FALSE;};
		BIPExport virtual void SetStartFrame(int startframe) {};
		BIPExport virtual void InitializeMoflowAnalysis() {};
		BIPExport virtual void SetDetourAngle(float angle) {};

		// make authorization available in one place to all dlls
		BIPExport virtual int  BipAuthorize(HWND hwndParent) {return FALSE;};
		BIPExport virtual int  BipIsAuthorized() {return FALSE;};
		BIPExport virtual void BipAuthorizedFromOutside() {};
};

#pragma warning(pop)


