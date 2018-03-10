
//*********************************************************************
//	Crowd / Unreal Pictures / bhvr.h
//	Copyright (c) 1999, All Rights Reserved.
//*********************************************************************

#pragma once

#include "..\maxheap.h"
#include "..\iparamm2.h"
#include "..\control.h"

#define MAXBHVRNAME	256

/*! \defgroup BehaviorActions Actions performed by the behavior
The following are flags which can be combined and returned from the Perform function.
*/
//@{
#define BHVR_SETS_FORCE	0x00000001   //!< flag indicating that a behavior sets a force
#define BHVR_SETS_GOAL	0x00000002   //!< flag indicating that a behavior sets a goal
#define BHVR_SETS_SPEED	0x00000004   //!< flag indicating that a behavior sets the speed
//@}

//! \brief This is the super class id of all behaviors.
#define BEHAVIOR_SUPER_CLASS_ID REF_TARGET_CLASS_ID  //!< behavior super class id

//! \brief This structure is sent to the BaseBehavior Perform function.
//! \see BaseBehavior::Perform
struct PerformOut: public MaxHeapOperators
{
	//! \brief a force acting upon a delegate
	Point3 frc;
	//! \brief a world space goal of a delegate
	Point3 goal;
	//! \brief the speed of a delegate (to be multiplied by the delegate's average speed)
	float speedwt;
	//! \brief the intended eventual speed of a delegate
	float speedAtGoalwt;
};

//! \brief This structure is sent to the BaseBehavior Constraint function.
//! \see BaseBehavior::Constraint
struct ConstraintInOut: public MaxHeapOperators
{
	//! \brief velocity of a delegate
	Point3 vel;
	//! \brief speed of a delegate
	float speed;
	//! \brief position of a delegate
	Point3 pos;
	//! \brief goal of a delegate
	Point3 desGoal;

};

//! \brief All behaviors must inherit from this base class
class BaseBehavior: public ReferenceTarget {
    public:
		enum BEHAVIOR_TYPE
		{
			FORCE = 0,
			CONSTRAINT,
			ORIENTATION
		};

		BaseBehavior() {}

//! \name Identification Functions
//! These functions are used to uniquely define the behavior and to describe its type.
//@{
		//! \brief Returns the super class id of the behavior
		/*! The default implementation should be used, to make certain that the new behavior class
			is recognized by the Crowd system and appears in the list of available behaviors inside
			Crowd.  Use ClassID to define your own class id specific to your behavior.
			\return the behavior super class ID
		*/
		SClass_ID SuperClassID() { return BEHAVIOR_SUPER_CLASS_ID; }

		#pragma warning(push)
		#pragma warning(disable:4100)

		//! \brief Sets the name of the behavior.  
		/*! When the user changes the name of the behavior from the Crowd interface,
			this function is called.  It should store the name in the behavior.
			\param[in] newname - the new name of the behavior.
		*/
		virtual void SetName(const MCHAR *newname) {}

		//! \brief Gets the name of the behavior.  
		/*! Usually this name will be stored internally within a paramblock.  The name usually
			starts out being the same as the behavior's class name.  The name can then be changed
			from within the Crowd system, which sets it by calling this function.
			\return the behavior name
		*/
		virtual const MCHAR *GetName() {return NULL;}

		//! \brief Returns the behavior's type, either FORCE, CONSTRAINT, or ORIENTATION. 
		/*!
			\li Force: Behaviors of this type force the delegates to move in a particular direction.
			Examples of force behaviors are Seek and Repel. 
			Force behaviors work by returning a force vector in the direction that the behavior
			wants the delegate to go in, and in some cases the speed it should be traveling and
			the goal it is trying to reach.
			\li Constraint: Behaviors of this type restrict the position and velocity of a delegate.
			An example of a constraint behavior is SurfaceFollow. Constraint behaviors set the
			velocity and sometimes may even change the delegate's position, in order to meet the
			constraint. The user can have only one active constraint behavior per delegate per frame.
			\li Orientation: Behaviors of this type affect only the orientation of the delegates. 
			An example of an orientation behavior is Orientation. These behaviors don't work with
			forces but instead return an orientation that the delegate should be at, represented by
			a quaternion. Any active orientation behavior will override the default orientation of
			the delegate. The velocity determines the default orientation. Like a constraint
			behavior, the user can have only one active orientation behavior per delegate per frame.\n
			\return one of three possible values: force, constraint, or orientation 
		*/
		virtual BEHAVIOR_TYPE BehaviorType() {return FORCE;}

		//! \brief Queries whether or not the behavior may be modified by the behavior weight scalar. 
		/*! For the user, this weight appears in the assignment dialog for each assignment.  
			Usually FORCE types are weightable, while CONSTRAINT and ORIENTATION behaviors aren't.
			\return 1 if the behavior is weightable, 0 if not.
		*/
		virtual BOOL IsWeightable() 
		        {if (BehaviorType() == BaseBehavior::CONSTRAINT)  return FALSE;
	             if (BehaviorType() == BaseBehavior::ORIENTATION) return FALSE;
				 return TRUE;}
				
		//! \brief Queries whether or not the behavior can convert itself to a script
		/*! This function is for future versions of Crowd.  It is not used now.
			\return true if the behavior can convert itself to maxScript, false if not
		*/
		virtual int CanConvertToMaxScript() {return FALSE;}
//@}	

//! \name Initialization Functions
//! These functions allow a behavior to initialize itself at different times during a simulation. 
//! They also allow a behavior to keep track of data associated with delegates assigned to it during
//!	a simulation.
//@{
		//! \brief Sends the behavior a list of all delegates which will participate in the simulation.
		/*! At the beginning of a simulation, this function is called for each behavior in the simulation.  
			All active delegates assigned via an active assignment to any behavior or 
			cognitive controller in the crowd system are included in the participants INodeTab.  
			So this function tells the behavior the maximum number of delegates that could possibly
			perform this behavior during the simulation, and which delegates those are.  
			With this information, the behavior can set up whatever data structures it requires to
			keep track of parameters on a per delegate basis.  At the end of the simulation, 
			this function is again called for each behavior, this time with an empty participants
			list, in order to allow the behavior to clear its data structures.
			\param[in] participants - a tab of all of the delegates in the simulation.  
			The delegates must be active, and they must be assigned, via an active assignment, 
			to at least one behavior or cognitive controller.
		*/
		virtual void SetUpDelegates(INodeTab& participants){}; //called at beginning of simulation.

		//! \brief Notifies the behavior that it should start to act upon a particular delegate.
		/*! This function is called during a simulation, whenever a cognitive controller activates 
			the behavior for a particular delegate.  It allows a behavior to keep track of 
			which delegates it is acting upon.
			\param[in] t - the current simulation time.
			\param[in] node - the delegate which started the behavior
		*/
		virtual void BehaviorStarted(TimeValue t, INode *node){};

		//! \brief Initializes a behavior at the start of each simulation frame.
		/*! This function is called at the beginning of each frame of the simulation.  
			It allows the behavior to set up or clear out any information that it might
			need each frame.
			\param[in] t - the current simulation time
		*/
		virtual void InitAtThisTime(TimeValue t){}

		//! \brief Initializes the behavior at the start of a simulation.
		/*!	This function is called once just before the beginning of the simulation.  
			It is called so that the behavior knows which frame the simulation is actually
			starting at so it can perform any actions necessary to assure
			its repeatability.  For example, if a behavior knows that the simulation is 
			starting at frame 10 instead of frame 0, it may run it's random number generator
			10 times so that the value it generates will be consistent with what it normally
			computes when the simulation starts from the beginning, as specified by the SimStart
			parameter.
			\param[in] FirstFrame	- the value of the first frame for this particular run of the simulation.
			This is the same as Start Solve in the crowd solve rollout.  
			It is not in time ticks, but in frames.
			\param[in] n			- the delegate
			\param[in] SimStart		- the value of the frame when the simulation starts
			This is the same as Simulation Start in the crowd solve rollout.  
			It is not in time ticks, but in frames.
			\param[in] AssignIndex	- the index of the assignment used to apply this behavior to the delegate.
			If this equals -1, the behavior has been assigned using a cognitive controller.  
			The AssignIndex can be used as a parameter to the IDelegate class's IsAssignmentActive 
			function, which allows you to find out if this assignment is active at a given time.  
			You may need this information to properly initialize a behavior before a simulation.
		*/
		virtual void InitBeforeSim(int FirstFrame, INode *n, int SimStart, int AssignIndex) {}
//@}

//! \name Performance Functions
//! These functions are called for each delegate assigned to a behavior, 
//! each frame during a simulation. Depending on the value returned from the BehaviorType function,
//! only one of the following three functions must be implemented.
//@{
		//! \brief This is the main function for FORCE behaviors.
		/*! It is called every frame for each delegate the behavior is acting upon.  
			It is within this function that the behavior may set any combination of the following:
			the force acting upon the delegate, the delegate's speed, or a goal towards 
			which the delegate should move.
			\param[in] node - the delegate
			\param[in] t - the current simulation time
			\param[in] numsubsamples - how many subsamples are being computed per frame. Currently not used.
			\param[in] DisplayHelpers - whether or not the behavior should display it's visual
			information.  This parameter exists so that the Crowd system has global control over when the 
			behavior information is displayed.
			\param[in] BhvrWeight - A float value, greater than or equal to 0.0, that corresponds
			to the BehaviorWeight value in the Crowd Assignment dialog.  This value should
			be used to scale the returned force value in the out parameter.
			\param[out] out - A PerformOut structure that is used to pass out information that the behavior calculates.  
			The structure has 4 items that may be set, depending upon whether the behavior sets a force, 
			a speed or a goal.  
			\li frc: If the behavior sets a force, the frc value should be filled 
			with a vector normalized to the average speed of the delegate it is acting upon. 
			Usually this normalized vector will be scaled also by the BhvrWeight parameter.
			\li goal: If the behavior sets a goal, then the goal item should be set with the world space
			goal that the delegate is trying to reach.  It is important to note that in order for the 
			behavior to work with Biped crowds, it needs to set a goal.  Even if the behavior doesn't 
			appear to have a clear-cut goal, a goal may still be set for it by placing the goal along 
			the force vector at some reasonable distance.  The delegate's average speed works well.
			\li speedwt: If the behavior sets a speed, then the speedwt should be set
			to the behavior's desired speed for the delegate at the current simulation time.  
			The speedwt will be multipled by the delegate's average speed. (so a value of 1.0 is equal 
			to the delegate's average speed). 
			\li speedAtGoalwt: The speedAtGoalWt is only used by delegates associated with bipeds.  
			If the speedAtGoalWt equals 1.0, the biped will attempt to maintain its current speed.  
			If it is 0.0, the biped will attempt to stop.  If it is < 0, the biped will try to slow down.  
			If it is > 1, the biped will try to speed up. \n
			Note that the behavior doesn't need to be setting a goal in order to set the
			speedAtGoalwt.  This is how the SpeedVary function works.  The instantaneous speed is
			the speed at the current time, while the speedAtGoalwt is the speed that it wants to
			reach at some time in the future.
			\return 0 if the behavior isn't active for this particular delegate, or if the behavior, 
			for any reason, wishes to have no effect on the delegate at this time.  Otherwise, it 
			returns the union of three #defines, BHVR_SETS_FORCE, BHVR_SETS_GOAL, and BHVR_SETS_SPEED, 
			depending upon whether or not the behavior sets a force, a goal or the speed.   For example, 
			a behavior that sets both a force and goal will return BHVR_SETS_FORCE | BHVR_SETS_GOAL.  
			Which of these values are returned determines which items in the PeformOut structure are valid.
		*/
		virtual int Perform(INode *node, TimeValue t, int numsubsamples, BOOL DisplayHelpers, 
							float BhvrWeight, PerformOut &out) {return FALSE;}

		//! \brief This is the main function for CONSTRAINT behaviors.  
		/*!	It is called multiple times per frame for each delegate the behavior is acting upon.  
			It is within this function that the behavior may set a modified velocity or 
			modified position for a delegate.
			\param[in] node - the delegate
			\param[in] t - the current simulation time
			\param[in] numsubsamples - how many subsamples are being computed per frame. Currently not used.
			\param[in] DesGoalExists - whether or not a goal currently exists for this delegate.  
			If it exists, it will be passed in via the inOut parameter.
			\param[in] DisplayHelpers - whether or not the behavior should display it's visual
			information.  This parameter exists so that the Crowd system has global control over 
			when the behavior information is displayed.
			\param[in] finalSet - whether or not this is the final time that the constraint
			will be called during the frame.  Currently, each constraint behavior is called two
			times per frame (this may change in the future) - once after all of the forces have 
			been assembled and just before a possible avoid behavior, and then once again at the
			end of the frame.  It is called multiple times per frame so that the behavior may
			work correctly with any active avoid behavior.  This flag lets the behavior know if
			it is the last time it will be called during that frame and thus should cache any
			information that it needs for the next frame calculation.  Usually if the behavior
			isn't doing any sort of per frame caching it should just ignore this flag.
			\param[in,out] &inOut - a ConstraintInOut structure that is used to pass in information
			that the behavior will modify and then pass out.  The structure contains 4 parameters:
			\li vel: the newly calculated velocity of the delegate
			\li pos: the current position of the delegate
			\li speed: the current speed of the delegate
			\li desGoal: the current desired goal of the delegate\n
			The behavior may change one or all of these parameters.  Note that the
			next position that the delegate will be at is nextPos = pos + normalized(vel)*Speed.
			So changing the current position doesn't actually change where it is, but it just 
			changes the position that is used to calculate it's new position.  Usually the 
			behavior will change the velocity parameter, and it will only change the pos parameter
			if it needs to so that the Speed, and thus the energy of the delegate, doesn't change.
			Note that the velocity that you return should be normalized.
			\return 0 if the behavior isn't active for this particular delegate, or if the behavior, 
			for any reason, wishes to have no effect on the delegate at this time.  Otherwise, it returns 1.
		*/
		virtual int Constraint(INode *node,
								TimeValue t,
								int numsubsamples, 
								BOOL DesGoalExists,
								BOOL DisplayHelpers,
								BOOL finalSet,
								ConstraintInOut &inOut) {return FALSE;}

		//! \brief This is the main function for ORIENTATION behaviors.  
		/*! It is called every frame for each delegate the behavior is acting upon.  
			It is within this function that the behavior may set a quaternion 
			representing the delegate's orientation.
			\param[in] vel - the current velocity of the delegate
			\param[in] node - the delegate
			\param[in] t - the current simulation time
			\param[out] quat - The quaternion specifying the new orientation of the delegate
			\return 0 if the behavior isn't active for this particular delegate, or if the behavior, 
			for any reason, wishes to have no effect on the delegate at this time.  Otherwise, it returns 1.
			*/
			virtual int Orient(const Point3 &vel,INode *node, TimeValue t,Quat &quat) {return FALSE;}
//@}

//! \name Display Functions
//! These functions provide an option to display an apparatus along with the behavior
//! at all times - not just during solve.  The behavior can offer an option in its user interface
//! to turn this on and off.  Keep in mind this apparatus will be displayed
//! as part of the crowd object, and so may enlarge the damaged rectangle significantly.
//@{
		//! \brief Using this function, a behavior can display text or graphics inside the viewports.
		/*! This is typically used to display a bounding area or a target.  Often the behavior 
			will allow the user to turn on or off this display by providing a checkbox in its interface.
			\param[in] t - the time to display the object.
			\param[in] vpt - an interface pointer that may be used to call functions associated with the viewports
			\return 1 if something is drawn, otherwise 0.
		*/
		virtual int  Display(TimeValue t, ViewExp *vpt) {return 0;}

		//! \brief Gets the world bounding box of what the behavior draws.
		//!	\param[in] t  - the time at which to compute the bounding box
		//!	\param[in] vpt - an interface pointer that may be used to call functions associated with the viewports
		//!	\param[out] box	- the bounding box
		virtual void GetWorldBoundBox(TimeValue t, ViewExp *vpt, Box3& box ) {}
//@}
	#pragma warning(pop)
};


