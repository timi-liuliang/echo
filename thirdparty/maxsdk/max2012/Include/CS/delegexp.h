/******************************************************************************
 *<
	FILE: delegexp.h
				  
	DESCRIPTION:  Export Interface Functionality for Crowd Delegate

	CREATED BY: Susan Amkraut

	HISTORY: created December, 1999

 *>     Copyright (c) Unreal Pictures, Inc. 1999 All Rights Reserved.
 *******************************************************************************/

#pragma once

#include <WTypes.h>
#include "..\maxheap.h"
#include "..\maxtypes.h"
#include "..\matrix3.h"
#include "..\color.h"
#include "..\units.h"
#include "..\trig.h"

#ifdef BLD_DELEG
#define DELEGexport __declspec( dllexport )
#else
#define DELEGexport __declspec( dllimport )
#endif



// This is the interface ID for a Delegate Interface
#define I_DELEGINTERFACE	0x00100101


// This is the Class ID for Vector Field Objects.
#ifndef BLD_DELEG

#define	DELEG_CLASSID		Class_ID(0x40c07baa, 0x245c7fe6)
#define CROWD_CLASS_ID		Class_ID(0x60144302, 0x43455584) // to avoid selecting a crowd

#endif
//! \brief An interface class to the delegate.
/*! The class provides functions to directly query specific delegate information. 
It is mainly intended to be used by a crowd behavior plugin.
This class can be returned by calling the method GetInterface() from a Delegate node. For Example:
\code
 Sample Code, starting with an INode(node)

     Object *o = node->GetObjectRef();
     if ((o->ClassID() == DELEG_CLASS_ID)
     {

         // Get the Delegate Export Interface from the node 
         IDelegate *Iface = (IDelegate *) o->GetInterface(I_DELEGINTERFACE);

         // Get the delegate's average speed at time t
         float AverageSpeed = Iface->GetAverageSpeed(t);
     }
		
	// Release the interface. NOTE that this function is currently inactive under MAX.
	//o->ReleaseInterface(I_DELEGINTERFACE,Iface);
\endcode
*/

class IDelegate: public MaxHeapOperators
{
	public:
//! \name Delegate Parameter Query Functions
//! These functions query the delegate about the values of its parameters, 
//! all of which appear in the delegate’s modify panel interface.
//@{
		//! \brief Queries whether the delegate is constrained in the Z plane
		//! \return true if the delegate is constrained to the z plane, false if not
		DELEGexport virtual BOOL IsConstrainedInZ() {return FALSE;}

		#pragma warning(push)
		#pragma warning(disable:4100)
		//! \brief Gets the average speed of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the average speed of the delegate
		DELEGexport virtual float GetAverageSpeed(TimeValue t) {return 1.0;}

		//! \brief Gets the maximum acceleration of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum acceleration of the delegate
		DELEGexport virtual float GetMaxAccel(TimeValue t){return 1.0;}

		//! \brief Gets the maximum heading velocity of the delegate
		//! \param[in] t - the time to retrieve the value 
		//! \return the maximum heading velocity of the delegate, in degrees.  
		//! This is shown as the Max Turn Velocity in the delegate's interface.
		DELEGexport virtual float GetMaxHeadingVel(TimeValue t) {return 1.0;}
		
		//! \brief Gets the maximum heading acceleration of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum heading acceleration of the delegate, in degrees.   
		//! This is shown as the Max Turn Accel in the delegate's interface.
		DELEGexport virtual float GetMaxHeadingAccel(TimeValue t){return 1.0;}

		//! \brief Gets the maximum pitch velocity of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum pitch velocity of the delegate, in degrees. 
		//! This is shown as the Max Turn Velocity in the delegate's interface.
		DELEGexport virtual float GetMaxPitchVel(TimeValue t) {return 1.0;}

		//! \brief Gets the maximum pitch acceleration of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum pitch acceleration of the delegate, in degrees.  
		//! This is shown as the Max Turn Accel in the delegate's interface.
		DELEGexport virtual float GetMaxPitchAccel(TimeValue t){return 1.0;}

		//! \brief Gets the maximum incline angle of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum incline angle of the delegate, in degrees
		DELEGexport virtual float GetMaxIncline(TimeValue t){return 1.0;}

		//! \brief Gets the maximum decline angle of the delegate
		//! \param[in] t - the time to retrieve the value
		//! \return the maximum decline angle of the delegate, in degrees
		DELEGexport virtual float GetMaxDecline(TimeValue t){return 1.0;}

		//! \brief Queries if the the delegate should display the forces acting upon it during a simulation  
		/*! If this returns true, and the behavior has a force to display, then the behavior plugin is expected to display 
		its force during the Perform function, by calling the IDelegate's LineDisplay function.
		\return true if the delegate's Show Forces checkbox is checked, false if not.
		*/
		DELEGexport virtual BOOL OkToDisplayMyForces() {return 1;}

		//! \brief Queries if the delegate should display its velocity during a simulation
		//! \return true if the delegate's Show Velocity checkbox is checked, false if not.  
		//! This is probably not needed by any behavior plugins, since velocity display is performed by the crowd system.
		DELEGexport virtual BOOL OkToDisplayMyVelocity() {return 1;}

		//! \brief Queries if the delegate should display its cognitive controller state during a simulation
		//! \return true if the delegate's Show Cog Control States checkbox is checked, false if not.  
		//! This is probably not needed by any behavior plugins, since cog control states are displayed by the crowd system.
		DELEGexport virtual BOOL OkToDisplayMyCogStates() {return 1;}
//@}

//! \name Position, Speed, Velocity, Transform Matrix during Simulation
//@{
		//! \brief This function should be used instead of calling GetNodeTM during a simulation loop.
		/*! This function must be called to get the transform, because internally as an optimization, 
		the Crowd system caches the delegate's positions
		and doesn't set the Node's MAX transform until after the simulation is done running.  
		If for some reason (based on the active flag, backtracking, etc) this function determines that
		the delegate's values are not cached, it will then call GetNodeTM.  So it is always best to use
		this function to get the delegate's TM, particularly during the Perform, Constrain, or Orient
		function of a behavior, which are only called during a Crowd simulation. 
		\param[in] node - The delegate's node.
		\param[in] t - the time to retrieve the value.
		\return the transformation matrix of the delegate in world space at the current frame in the simulation.
		*/
		DELEGexport virtual Matrix3 GetTM(INode *node,TimeValue t){return Matrix3();}

		//! \brief Gets the position of the delegate
		//! \return the position of the delegate in world space at the current frame in the simulation
		DELEGexport virtual Point3 GetCurrentPosition() {return Point3(0.0,0.0,0.0);}

		//! \brief Gets the velocity of the delegate
		//! \return the velocity of the delegate at the current frame in the simulation.  
		//! It may or may not be normalized. If not, it's length is equal to it's current speed.
		DELEGexport virtual Point3 GetCurrentVelocity() {return Point3(0.0,0.0,0.0);}

		//! \brief Gets the previous velocity of the delegate
		//! \return the velocity of the delegate at the previous frame in the simulation.
		DELEGexport virtual Point3 GetPreviousVelocity() {return Point3(0.0,0.0,0.0);}

		//! \brief Gets the speed of the delegate
		//! \return the speed of the delegate at the current frame in the simulation
		DELEGexport virtual float GetCurrentSpeed() {return 1.0;}

		//! \brief Gets the delegate's velocity at the start of a simulation	
		/*! If the node is a delegate and the start time is the simulation start time, 
		this will return the initial velocity of the delegate, exactly as the Crowd system computes it.  
		This function is particularly useful to call from within a behavior's InitBeforeSim function, 
		in order to make sure the behavior is perfectly repeatable.
		*/
		//! \param[in] n - the node that the velocity is to be calculated for
		//! \param[in] StartTime - the time to retrieve the value 
		//! \return the velocity of the node at a given start time
		DELEGexport virtual Point3 GetSimStartVelocity(INode *n, TimeValue StartTime) {return Point3(0.0,0.0,0.0);}

//@}

//! \name Display Functions during Simulation
//! During a simulation, a behavior might want to display some information.  
//! For instance, the Pathfollow behavior displays the target on the path.  
//! The IDelegate class has some functions for displaying simple primitives during the simulation.  
//! These should be called from within a behavior's Perform, Constrain, or Orient function, 
//! since those are called per frame per delegate during a simulation.
//@{
		//! \brief Draws a line segment while the simulation is running
		//! \param[in] pt1 - the start position of the line segment in world space
		//! \param[in] pt2 - the end position of the line segment in world space
		//! \param[in] clr - the color of the line
		//! \param[in] scale - whether or not the line should be scaled by the Vector Scale value specified in the Crowd Solve rollout.
        DELEGexport virtual void LineDisplay(Point3& pt1, Point3& pt2, Color clr, BOOL scale) {}

		//! \brief Draws a bounding box while the simulation is running
		//! \param[in] pt1 - the minimum point of the bounding box in world space				
		//! \param[in] pt2 - the maximum point of the bounding box in world space
		//! \param[in] clr - the color of the bounding box
        DELEGexport virtual void BboxDisplay(Point3& pt1, Point3& pt2, Color clr) {}

		//! \brief Draws a sphere while the simulation is running
		//! \param[in] pt1 - the center point of the sphere in world space.
		//! \param[in] radius - the radius of the sphere.		
		//! \param[in] clr - the color of the sphere.
        DELEGexport virtual void SphereDisplay(Point3& pt1, float radius, Color clr) {}
        
        //! \brief Draws text while the simulation is running.
        //! \param[in] pt1 - the world position of where the text will start
        //! \param[in] clr - the color of the text
        //! \param[in] str - the string of text that will be printed
        DELEGexport virtual void TextDisplay(Point3& pt1, Color clr, const MCHAR *str) {}
//@}
		
//! \name Miscellanious Functions
//@{
		//! \brief Queries whether the delegate is currently active in a running crowd simulation
		//! \return true if the delegate is currently active in a running crowd simulation, false if not
		DELEGexport virtual BOOL IsComputing() {return FALSE;}

        //! \brief Queries whether a particular assignment is active at a particular time
		/*! This is a way to ask the Crowd system whether or not a particular 
		 assignment is active at a particular frame.  It is not really related to 
         this particular delegate, but since internally the delegate has access to
         the crowd, it is used to get this information.  Typically, this is called 
         from within BaseBehavior::InitBeforeSim to see whether or not an assignment
         was active at a particular point in time during the simulation.
        //! \param[in] AssignIndex - an index into the list of behavior assignments in the Behavior Assignments dialog
        //! \param[in] t - the time to retrieve the value
        //! \return true if the assignment is active at this time, false if not.
		*/
        DELEGexport virtual BOOL  IsAssignmentActive(int AssignIndex, TimeValue t) {return TRUE;}

		//! \brief  Get the delegate's identifier
		//! \return an integer > 0, that is an unique identifier for a delegate while a crowd simulation is running.
		DELEGexport virtual int GetIndex() {return 0;}

		//! \brief Gets a random id to generate varying behavior for different delegates performing the same behavior
		//! \return an integer value  > 0 that specifies a number that can be used as part
		//! of a seed when calculating random numbers from within a behavior.  For instance, 
		//! this number is used by the pathfollow, speedvary, surfacearrive, and wander behaviors.
		//! For behaviors which use the RandId, two or more delegates that have the same RandId will
		//! exhibit the same random behavior.  Unless the RandId has been specifically changed by the user,
		//! it will be unique for all delegates.  The Crowd system assures this.
		DELEGexport virtual int GetRandId() {return 0;}

        //! \brief Queries the whether the delegate is associated with a computing biped
		//! \return true if this delegate is associated with a biped whose crowd motion 
		//! is currently being computed, false if not
		DELEGexport virtual BOOL ComputingBiped() {return FALSE;}

        //! \brief This function is used internally.
		DELEGexport virtual BOOL ReactToMe() {return FALSE;} 

        //! \brief This function is used internally.
		DELEGexport virtual void ClearBacktracking() {}
		
		//! \brief This function is used internally.
		DELEGexport virtual BOOL  NeedsBacktracking() {return FALSE;}
		
		//! \brief This function is used internally.
		DELEGexport virtual void SetBacktracking(int frame) {}
		#pragma warning(pop)
		DELEGexport virtual ~IDelegate() {}	
	
//@}
};



