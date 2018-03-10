

#pragma once

#include "object.h"

/********************************************************************************

  IPainterInterface contains all the interfaces to let a plugin interact with
  Maxs Paint system

  Basically the system consists of 2 components the IPainterInterface and IPainterCanvasInterface

  For a plugin to use the paint system it must sub class of IPainterCanvasInterface_V5 or greater 
  and implement the virtual methods and be a sub class of ReferenceTarget.  A canvas is what the 
  painterInterface paints on. Basically  IPainterCanvasInterface_V5 consists of functions to 
  identify the canvas version and functions that are called a as the system paints, basically a 
  start stroke, stroke and endstroke.

  The IPainterInterface is what does all the hit testing against the mesh using a quad tree.  
  Basically it tells the canvas what the stroke looks like.  In addition the canvas can use it
  do additional hit testing not associated with a stroke.

  In its simplest from all that needs to be done is
  
	1.  Subclass your plugin from IPainterCanvasInterface_V5 and ReferenceTarget
	2.  Fill out these virtual methods

		StartStroke() - called when a stroke is started
		PaintStroke()/PaintStroke(<..>) - called as a stroke is going on
		EndStroke() - called when the stroke ends and accepted.
		EndStroke(<...>) - called when the system is set to non interactive mode.
		CancelStroke() - called if a stroke is canceled.
		SystemEndPaintSession() - is called when the painter wants to end a paint session for some reason
		See the header for me description of the parameters

        You also need to add the painter interface to the GetInterface method from ReferenceTarget.  
		Something along the lines below.  This lets the painter set what interfaces the canvas supports.
			
			  void* PainterTester::GetInterface(ULONG id)
				{
				switch(id)
					{
					case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
						break;
					default: return ReferenceTarget::GetInterface(id);
					break;
					}
				}



    3.  In addition to implementing these methods, The plugin needs to hook up to 
	    the painterInterface.  Using these functions.  The pointer to the painter 
		can be gotten by using the CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID). 
		Once that pointer is gotten you need to grab a specific version of the painterInterface
		using GetPaintInterface.   See sample below

	
			ReferenceTarget *painterRef = (ReferenceTarge *) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);	
//set it to the correct verion
			if (painterRef)			
				IPainterInterface_V5 *painter = (IPainterInterface_V5 *) painterRef->GetInterface(PAINTERINTERFACE_V5);

		Once you have a valid pointer you need to hook your self up to the Painter using the
		InitializeCallback method.  This is should be done just before you want to start a paint seesion.
		All you are doing is passing a pointer of yourself to the painter system so it knows where 
		to send the messages.  Note yourself to IPainterCanvasInterface	since the Painter will 
		interogate you to find out which version of the canvas you are.

			painter->InitializeCallback((ReferenceTarget *) this);

		Once you have called InitializedCallback and are ready to start a paint session you need load up which 
		Nodes you want to paint on.  This is done with InitializeNodes(int flags, Tab<INode*> &nodeList).

				painter->InitializeNodes(0, nodes);

		Then StartPaintSession() is called.  Once this is called everytime the the users drag paints across
		the nodes the Canvas's StartStroke/PaintStroke/EndStroke will get called.
		
				painter->StartPaintSession();

		If the canvas changes the topology or geometry of the painted nodes UpdateMeshes() needs to be called
		so the quad tree gets updated.

				painter->UpdateMeshes(BOOL updatePoitnCache);

		
		Once the canvas is done with the painter it needs to call EndPaintSession() so that all the data can
		be freed and it can be unhooked.  Otherwise the StartStroke/PaintStroke/EndStroke will still be called.

				painter->EndPaintSession();

		You can also bring up the Painters Option dialog using the BringUpOptions().

				painter->BringUpOptions() ;

		In addition there will be some helper functions for gathering points within a stroke and their weigths; and 
		tools for intersecting rays against the quad tree.  See the header for more descriptions.


		See the PaintTester project for a simple implementation of a canvas and interaction with the painter.



*********************************************************************************/


#include "maxheap.h"
#include "iFnPub.h"
#include "icurvctl.h"


#define PAINTERINTERFACE_V5 0x78ffe181
#define PAINTERINTERFACE_V7 0x78ffe182
#define PAINTERINTERFACE_V14 0x78ffe183
#define PAINTERCANVASINTERFACE_V5 0x29ff7ac9
#define PAINTERCANVASINTERFACE_V5_1 0x29ff7ad0
#define PAINTERCANVASINTERFACE_V7 0x1e962374

#define PAINTERINTERFACE_CLASS_ID	Class_ID(0x2f2ef7e9, 0x78ffe181)
#define PAINTERINTERFACE_SUPERCLASS_ID	REF_TARGET_CLASS_ID

#define PAINTERINTERFACEV5_INTERFACE Interface_ID(0x53b4520c, 0x29ff7ac9)

//These are defines used to when mirroring is enabled with point gathering
//a point can be hit by the original brush, the mirror brushed, or both
#define NO_MIRRROR		0
#define MIRRRORED		1
#define MIRRROR_SHARED	2


/*! \defgroup painterColorIDs Painter Interface Color Defines 
These are IDs for the colors of the brush for use with the color manager
\sa Class IPainterInterface_V5 */
//@{
#define			RINGCOLOR			0x445408e0
#define			NORMALCOLOR			0x445408e1
#define			RINGPRESSEDCOLOR	0x445408e2
#define			NORMALPRESSEDCOLOR	0x445408e3
#define			TRACECOLOR			0x445408e4
#define			GIZMOCOLOR			0x445408e5
//@}

/*! \defgroup painterPressureIDs Painter Interface Pressure Defines
These are IDs to define what the pressure affects on a stroke
\sa Class IPainterInterface_V5 */
//@{
#define PRESSURE_AFFECTS_NONE	0
#define PRESSURE_AFFECTS_STR	1
#define PRESSURE_AFFECTS_SIZE	2
#define PRESSURE_AFFECTS_BOTH	3
//@}


/*! \sa IPainterCanvasInterface_V5 
\par description
This class is available in Max 7 and higher only\n\n
Any plugin that wants to be able to be painted 
on must sub class from this class or a later version of it. */
class IPainterInterface_V5: public MaxHeapOperators
	{
	public:
	
//these are the esswential tools to get you started		
//they cover the basics to get you started


	//! \brief The canvas passes a pointer of itself to the painter.
	/*! This is so the painter knows where to send the stroke messages
		\param canvas - is the pointer to the canvas */
		virtual BOOL  InitializeCallback(ReferenceTarget *canvas) = 0;//tested

	//! \brief this loads of the nodes that you want to paint on
	/*! Anytime you want to add delete a node this must be called
		\params flags - not yet implemented does nothing but there to allow 
						flags per node for special conditions
		\params nodeList - a table of nodes that you want to paint on */
		virtual BOOL  InitializeNodes(int flags, Tab<INode*> &nodeList) = 0;//tested

	//! \brief This forces the quadtree to rebuilt.  
	/*! Any time you change a geometry or togopology
		of a node you passed to the InitializedNodes methode this must be called.
		Ideally I could listen to the notifyremessage changed and rebuild on that 
		but the since rebuilding the quad tree is inetensive, I leave it up to the 
		canvas as to when to rebuild so we can better control the amount of stack re-evals
		\param updatePointGather - determines whether the pointGather data gets updated also
								  normally if your mesh does not change shape or topology you dont
								  need to update the pointgather.  For instance if you rotate a view
								  your quad tree needs to get updated but not the point list */
		virtual BOOL  UpdateMeshes(BOOL updatePointGather) = 0;

	//! This is called when the a canvas wants to start a paint session.
		virtual BOOL  StartPaintSession() = 0;//tested

	//! This is called when the a canvas wants to end a paint session.
		virtual BOOL  EndPaintSession() = 0;//tested


	//! This return whether the user is in the paint mode, so a plugin can determine how to paint the UI paint button
		virtual BOOL  InPaintMode()=0;//tested

	//! This brings up the Painter Options dialog that lets the users sets various setting of the painter system
		virtual BOOL  BringUpOptions() = 0;//tested

	//! \brief This lets you access the time stamp of each sample of a stroke.  
	/*! This lets you look at the acceleration of the mouse as it moves if say
		you wanted to paint a stroke and use it for an animation path.  */
		virtual int *RetrieveTimeList(int &ct) = 0;

//These functions are additional tools to further hit testing and point gathering
//to help with finding points on a mesh that are within the stroke and theoir weights

	//! \brief this function lets you hit test against the quad tree given a mouse coord
	/*! \param mousePos  - the position in screen space that you want to hit test agains
		\param worldPoint - the world hit point position 
		\param worldNormal - the world hit point normal
		\param localPoint - the local hit point position
		\param localNormal - the local hit point normal
		\param bary - the barycentry coord of the face that was hit
		\param index - the index of the face that was hit
		\param node - the node that was hit
		\param mirrorOn - whether mirroring was on or off
		\param worldMirrorPoint - the world hit point position after it was mirrored
		\param worldMirrorNormal - the world hit point normal after it was mirrored
		\param localMirrorPoint - the local hit point position after it was mirrored
		\param localMirrorNormal - the local hit point normal after it was mirrored
		\return TRUE if that point insterects the quad tree */
		virtual BOOL TestHit(
						  IPoint2 mousePos,
						  Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal
						  ) = 0;

	//! \brief Retrieves a random hit point around the last hit point or a specified hit point within the brush 
	/*! Useful if you want to do airbrush type effects or to just sample around the hit point
		\param[out] worldPoint - the world hit point position
		\param[out] worldNormal - the world hit point normal
		\param[out] localPoint - the local hit point position
		\param[out] localNormal - the local hit point normal
		\param[out] bary - the barycentry coord of the face that was hit
		\param[out] index - the index of the face that was hit
		\param[out] strFromFalloff - the strength of the point based on the fall off of the brush
		\param[out] node - the node that was hit
		\param[out] mirrorOn - whether mirroring was on or off
		\param[out] worldMirrorPoint - the world hit point position after it was mirrored
		\param[out] worldMirrorNormal - the world hit point normal after it was mirrored
		\param[out] localMirrorPoint - the local hit point position after it was mirrored
		\param[out] localMirrorNormal - the local hit point normal after it was mirrored
		\param tabIndex - What hit you want to sample around if 0 or less it will hit around the last hit test
		\return FALSE if a hit point was not found */
		virtual BOOL RandomHit(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex) = 0;

	//! \brief This will do random hit point along the stroke segment
	/*!	\param[out] worldPoint - the world hit point position
		\param[out] worldNormal - the world hit point normal 
		\param[out] localPoint - the local hit point position
		\param[out] localNormal - the local hit point normal
		\param[out] bary - the barycentry coord of the face that was hit
		\param[out] index - the index of the face that was hit
		\param[out] strFromFalloff - the strength of the point based on the fall off of the brush
		\param[out] node - the node that was hit
		\param[out] mirrorOn - whether mirroring was on or off
		\param[out] worldMirrorPoint - the world hit point position after it was mirrored
		\param[out] worldMirrorNormal - the world hit point normal after it was mirrored
		\param[out] localMirrorPoint - the local hit point position after it was mirrored
		\param[out] localMirrorNormal - the local hit point normal after it was mirrored
		\param tabIndex is what segment you want to sample around if 0 or less it will hit around the last segment
		\return FALSE if it does not find a hit */
		virtual BOOL RandomHitAlongStroke(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex) = 0;


	//! \brief this clears out all the stroke data for the current stroke
	/*! This method is used if you want to do a custom stroke.   Say for instance
		you wanted to just stroke a straight line, by default the painter uses a path stroke.
		so what you do on the PaintStroke method record the first and last mousePos, clear all the 
		stroke data and then Add your custom stroke to the system. */
		virtual BOOL ClearStroke()=0;

	//! \brief this adds a hit test to the current stroke
	/*! This method is used if you want to do a custom stroke.   Say for instance
		you wanted to just stroke a straight line, by default the painter uses a path stroke.
		so what you do on the PaintStroke method record the first and last mousePos, clear all the 
		stroke data and then Add your custom stroke to the system.
		\param mousePos - the point that want to test to add
		\param rebuildPointGatherData - this determines whether the poing gather data get rebuilt
					this allows you to delay the building of the data if you are addding mulitple
					points at once
		\param updateViewport - determines if the viewports get updated after this call */
		virtual BOOL AddToStroke(IPoint2 mousePos, BOOL rebuildPointGatherData, BOOL updateViewport)=0;

	//These are direct to stroke data lists if you are doing your own point gather
	//Each stroke contains an arrays of data that are used to describe it such as
	//position, str, radius etc.

	//! \brief returns the number of sample points in the current stroke
		virtual int GetStrokeCount() = 0;
	//! \brief this returns a pointer to an array of floats where each entry is the str of a point sample
		virtual float *GetStrokeStr() = 0;
	//! \brief this returns a pointer to an array of floats where each entry is the radius of a point sample
		virtual float *GetStrokeRadius() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the world space hit point of the sample
		virtual Point3 *GetStrokePointWorld() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the world space normal of the sample
		virtual Point3 *GetStrokeNormalWorld() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the world space hit point of the sample after it has been mirrored
		virtual Point3 *GetStrokePointWorldMirror() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the world space normal of the sample after it has been mirrored
		virtual Point3 *GetStrokeNormalWorldMirror() = 0;
	//! \brief this returns a pointer to an array of floats where each entry is the pressure of a point sample either from a pressure sensitive tablet
	//! or from a predefined presssure graph
		virtual float *GetStrokePressure() = 0;

	//! \brief this returns a pointer to an array of point3s where each entry is the local space hit point of the sample
		virtual Point3 *GetStrokePointLocal() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the local space normal of the sample
		virtual Point3 *GetStrokeNormalLocal() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the local space hit point of the sample after it has been mirrored
		virtual Point3 *GetStrokePointLocalMirror() = 0;
	//! \brief this returns a pointer to an array of point3s where each entry is the world space normal of the sample after it has been mirrored
		virtual Point3 *GetStrokeNormalLocalMirror() = 0;

	//! \brief this returns a pointer to an array of Ipoint2s where each entry is the mouse pos in screen space for the sample
		virtual IPoint2 *GetStrokeMousePos() = 0;
	//! \brief this returns a pointer to an array of s where each entry is whether the sample hit the mesh or not
	/*! the system allows the user to paint off the mesh, where all hitpoint are projected onto a plane
		based on the last hit point and normal */
		virtual BOOL *GetStrokeHitList() = 0;

	//! \brief this returns a pointer to an array of point3s where each entry is the barycentri coords of the sample
		virtual Point3 *GetStrokeBary() = 0;
	//! \brief this returns a pointer to an array of ints where each entry is the index of the face of the sample 
		virtual int *GetStrokeIndex() = 0;

	//! \brief this returns a pointer to an array of bools where each entry is the state of the shift of the sample 
		virtual BOOL *GetStrokeShift() = 0;
	//! \brief this returns a pointer to an array of bools where each entry is the state of the ctrl of the sample 
		virtual BOOL *GetStrokeCtrl() = 0;
	//! \brief this returns a pointer to an array of bools where each entry is the state of the alt of the sample 
		virtual BOOL *GetStrokeAlt() = 0;

	//! \brief this returns a pointer to an array of INode where each entry is the INode of the sample 
		virtual INode **GetStrokeNode() = 0;
	//! \brief this returns a pointer to an array of ints where each entry is the time stamp of the sample 
		virtual int *GetStrokeTime() = 0;


	//! \brief given a point in world space it returns the str of that point based on the current stroke
		virtual float GetStrFromPoint(Point3 point) = 0;

//These functions let you interogate and set the state of the options dialog

	//! \brief Lets you interogate and set the state of the options dialog
	/*! this is used to ask the system if a stroke str changes as it is painted
		the user can set predetermined shapes graphs and attach them to the str 
		of brush based on the position of the in the curve it is.  If you are in an interactive
		mode this data will always be changing so you can use this to get the current str/sizes
		of the hit points.  If you are in a non interactive mode you do not need to call this
		since all the correct sizes/str are sent to the end stroke arrays.  If the user has not
		specified any predetermined graphs the arrays will be NULL
		\param ct - count of the arrays */
		//these return array of floats one entry for each point on the stroke
		 virtual float *GetPredefineStrStrokeData(int &ct)=0;

	 //! \brief Lets you interogate and set the state of the options dialog
	/*! this is used to ask the system if a stroke size changes as it is painted
		the user can set predetermined shapes graphs and attach them to the size
		of brush based on the position of the in the curve it is.  If you are in an interactive
		mode this data will always be changing so you can use this to get the current str/sizes
		of the hit points.  If you are in a non interactive mode you do not need to call this
		since all the correct sizes/str are sent to the end stroke arrays.  If the user has not
		specified any predetermined graphs the arrays will be NULL
		\param ct - count of the arrays */
		 virtual float *GetPredefineSizeStrokeData(int &ct)=0;


		 //put in access to all the dialog properties

	//! Access to the BuildNormalData property
	/*! This will build a vertex normal list that you can access through RetrievePointGatherNormals
		This is by default is off to save memory.  Also if you use a custom point list through
		LoadCustomPointGather no normals will be built since there is no topo data to build them from
		\return whether we build normal data or not */
		virtual BOOL  GetBuildNormalData() = 0;
	//! Access to the BuildNormalData property
	/*! This will build a vertex normal list that you can access through RetrievePointGatherNormals
		This is by default is off to save memory.  Also if you use a custom point list through
		LoadCustomPointGather no normals will be built since there is no topo data to build them from
		\param enable - Sets whether we build normal data or not */
		virtual void  SetBuildNormalData(BOOL enable) = 0;

//These functions deal with the point gather.  The painter can automatically determines the weights
//of points by using the  PointGather

	//! \brief Gets the state of point gathering
	/*! If this is enabled, the points of the mesh will be  used as your points 
		\return TRUE if point gather is enabled */
		virtual BOOL  GetEnablePointGather() = 0;
	//! \brief Turn on/off the point gather
	/*! If this is enabled, the points of the mesh will be  used as your points 
		param enabled - The new state of the point gather */
		virtual void  SetEnablePointGather(BOOL enable) = 0;//tested

	//! \brief This lets you set up a custom list of points to weight to override the currentlist
	/*! for instance if you have a non mesh you will need to do this since by default the
		point gather uses the mesh points to weight
		\param ct - the number of points that you want to add
		\param points - the points you want to add in world space
		\param node - which node you want to assign them to */
		virtual	BOOL LoadCustomPointGather(int ct, Point3 *points, INode *node) = 0;//tested

		//Once a stroke has started you can retrieve data about you point list such
		//as weigths, str etc.
	//! \brief This retrieves the weight of the points based on the current stroke
	/*! \param node - the node that you want to inspect
		\param[out] ct - the number of points in the array 
		\return a pointer to an array of weights */
		virtual float *RetrievePointGatherWeights(INode *node, int &ct) = 0;
	//! \brief This retrieves the strength of the points based on the current stroke
	/*! \param node - the node that you want to inspect
		\param[out] ct - the number of points in the array 
		\return a pointer to an array of point strengths */
		virtual float *RetrievePointGatherStr(INode *node, int &ct) = 0;//tested
	//! \brief This retrieves the whether the point was affected by a mirror stroke
	/*! \param node - the node that you want to inspect
		\param[out] ct - the number of points in the array 
		\return an pointer to an array of bools indicating if points were mirrored */
		virtual BOOL *RetrievePointGatherIsMirror(INode *node, int &ct) = 0;//tested
	//! \brief This retrieves the the array of the points 
	/*! \param node - the node that you want to inspect
		\param[out] ct - the number of points in the array 
		\return a pointer to an array of gather points */
		virtual Point3 *RetrievePointGatherPoints(INode *node, int &ct) = 0;//tested
		
	//! \brief This retrieves the the array of the normals 
	/*! \param node	- the node that you want to inspect
		\param[out] ct - the number of points in the array
		\return An array of normals in local space.
			Note that these are only valid if you do not use a CustomPointGather */
		virtual Point3 *RetrievePointGatherNormals(INode *node, int &ct) = 0;

	//! \brief This retrieves the the array of the U vals, this is how far along the stroke that point is 
	/*! \param node - the node that you want to inspect
		\param[out] ct - the number of points in the array
		\return a pointer to an array of float U values */
		virtual float *RetrievePointGatherU(INode *node, int &ct) = 0;

		

//functions to get the mirror plane data
//NOTE all mirror function work in world space
	//! \brief returns if the mirror plane is on or off
		virtual BOOL  GetMirrorEnable() = 0 ;//tested
	//! \brief lets you set whether the mirror plane is on/off
		virtual void  SetMirrorEnable(BOOL enable) = 0;
	//! \brief returns the center of the mirror plane in world space coords
	/*! the mirror plane is always aligned to the world axis
		\return The center of the mirror plane in world space */
		virtual Point3 GetMirrorPlaneCenter() = 0;//tested
	//! \brief returns which mirror axis is active
	/*! \return One of the following values:\n
			\t 0 = x axis\n
			\t 1 = y axis\n
			\t 2 = z axis\n */
		virtual int GetMirrorAxis() = 0;//tested
	//! \brief lets you set the mirror axis
	/*! \param dir - One of the following values:\n
			\t 0 = x axis\n
			\t 1 = y axis\n
			\t 2 = z axis\n */
		virtual void SetMirrorAxis(int dir) = 0;
	//! \brief Returns the mirror offset
		virtual float GetMirrorOffset() = 0;//tested
	//! \brief Sets the mirror offset
		virtual void  SetMirrorOffset(float offset) = 0;//tested


	//! \brief Lets you get the quad tree depth
	/*! The deeper the quad tree the more memory you consume, but the 
		more memory you consume (the memory consumption is exponential so be careful)
		\return the current quad tree depth */
		virtual int GetTreeDepth() = 0;//tested
	//! \brief Lets you set the quad tree depth
	/*! The deeper the quad tree the more memory you consume, but the 
		more memory you consume (the memory consumption is exponential so be careful) 
		\param depth - the new quad tree depth */
		virtual void SetTreeDepth(int depth) = 0;//tested

	//! \brief Lets you get the Update on Mouse Up option
	/*! When this is enabled you will not get PaintStroke calls.
		Instead you will get all the points at the end through the endStoke function 
		\return the current Update on Mouse Up state */
		virtual BOOL GetUpdateOnMouseUp() = 0;
	//! \brief Lets you get the Update on Mouse Up option
	/*! When this is enabled you will not get PaintStroke calls.
		Instead you will get all the points at the end through the endStoke function 
		\param update - Enables or disables the Update on Mouse Up option state. */
		virtual void SetUpdateOnMouseUp(BOOL update) = 0;

	//! \brief These 2 function let you get and set the lag rate
	/*! When this is enabled you get PaintStroke delayed by the lag rate calls.
		every x(lagrate) stroke points you willget the strokes. 
		\return The current lag rate */
		virtual int GetLagRate() = 0;
	//! \brief These 2 function let you get and set the lag rate
	/*! When this is enabled you get PaintStroke delayed by the lag rate calls.
		every x(lagrate) stroke points you willget the strokes. 
		\param lagRate - The new lagrate */
		virtual void SetLagRate(int lagRate) = 0;

//These functions control how the brush behaves

	//! \brief Let you get the min strength for a brush.
	/*! If there is no pressure sensitive device attached only the max str is used
		\return The current min strength */
		virtual float GetMinStr() = 0;//tested
	//! \brief lets you set the min strength for a brush.
	/*! If there is no pressure sensitive device attached only the max str is used
		\param str - The new min strength */
		virtual void  SetMinStr(float str) = 0;//tested
	//! \brief lets you get the max strength for a brush.
	/*! If there is no pressure sensitive device attached only the max str is used
		\return The current max strength */
		virtual float GetMaxStr() = 0;//tested
	//! \brief lets you set the max strength for a brush.
	/*! If there is no pressure sensitive device attached only the max str is used
		\param str - The new max strength */
		virtual void  SetMaxStr(float str) = 0;//tested

	//! \brief lets you get the min radius for a brush.
	/*! If there is no pressure sensitive device attached only the max radius is used
		\return the current min radius */
		virtual float GetMinSize() = 0;//tested
	//! \brief lets you set the min radius for a brush.
	/*! If there is no pressure sensitive device attached only the max radius is used
		\param str - the new size */
		virtual void  SetMinSize(float str) = 0;//tested
	//! \brief lets you get the max radius for a brush.
	/*! If there is no pressure sensitive device attached only the max radius is used
		\return the current max radius */
		virtual float GetMaxSize() = 0;//tested
	//! \brief lets you set the max radius for a brush.
	/*! If there is no pressure sensitive device attached only the max radius is used
		\param str - the new max size */
		virtual void  SetMaxSize(float str) = 0;//tested

	//! Gets the aditive mode
	/*! When additive mode is off the weight is absolutely set based on the current stroke hit.
		Previous stroke data is over written.  In Additive mode the strength is added to current
		strength and is not capped.
		\return True if we are in additive mode */
		virtual BOOL  GetAdditiveMode()=0;//tested
	//! Sets the aditive mode
	/*! When additive mode is off the weight is absolutely set based on the current stroke hit.
		Previous stroke data is over written.  In Additive mode the strength is added to current
		strength and is not capped.
		\param enable - True to enable additive mode, false to set absolute */
		virtual void  SetAdditiveMode(BOOL enable)=0;//tested
	//! \brief This returns the brush falloff curve if you want to handle the doing the brush 
	//! falloff yourself
		virtual ICurve *GetFalloffGraph()=0;


//These functions allow you to control the display of a stroke
//Colors are stored in the color manager. See the color ID defines at the top

	//! \brief This lets you get whether the ring is drawn around the hit point
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual BOOL  GetDrawRing()=0;//tested
	//! \brief This lets you set whether the ring is drawn around the hit point
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual void  SetDrawRing(BOOL draw)=0;//tested

	//! This lets you get whether the normal vector is drawn at the hit point
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual BOOL  GetDrawNormal()=0;//tested
	//! This lets you set whether the normal vector is drawn at the hit point
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual void  SetDrawNormal(BOOL draw)=0;//tested

	//! This lets you get whether the a line is left behind a stroke as it is drawn
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual BOOL  GetDrawTrace()=0;//tested
	//! This lets you set whether the a line is left behind a stroke as it is drawn
	/*! \sa The <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_color_i_ds.html">Color ID defines</a> */
		virtual void  SetDrawTrace(BOOL draw)=0;//tested


//These functions deal with the pressure sensitive devices and mimicing pressure sensitivity

	//! \brief let you get whether pressure sensistivity is turned on
	/*! when Pressure is enabled it can affect Str, Radius, Both Str and Radius or Nothing
		You would nothing fr instance if you wanted to do a custom affect for pressure.
		\return TRUE if pressure is enabled */
		virtual BOOL  GetPressureEnable()=0;//tested
	//! \brief let you set whether pressure sensistivity is turned on
	/*! when Pressure is enabled it can affect Str, Radius, Both Str and Radius or Nothing
		You would nothing fr instance if you wanted to do a custom affect for pressure.
		\param enable - TRUE to enable pressure sensitivity */
		virtual void  SetPressureEnable(BOOL enable)=0;//tested

	//! \brief Get what the pressure of a brush affects
	/*! You can effect Str. Radius, Str and Radius or None
		\sa See the <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_pressure_i_ds.html">Pressure defines</a> in IPainterInterface.h */
		virtual BOOL  GetPressureAffects()=0;//tested
	//! \brief Set what the pressure of a brush affects
	/*! You can effect Str. Radius, Str and Radius or None
		\param affect - One of the <a href="ms-its:3dsMaxSDKRef.chm::/group__painter_pressure_i_ds.html">Pressure defines</a> in IPainterInterface.h */
		virtual void  SetPressureAffects(int affect)=0;//tested

	//! \brief get whether a predefined str is enabled for a stroke.
	/*! A predefined str stroke lets the user graph the str of stroke over the length of stroke */
		virtual BOOL  GetPredefinedStrEnable()=0;//tested
	//! \brief set whether a predefined str is enabled for a stroke.
	/*! A predefined str stroke lets the user graph the str of stroke over the length of stroke */
		virtual void  SetPredefinedStrEnable(BOOL enable)=0;//tested

	//! \brief get whether a predefined radius is enabled for a stroke.
	/*! A predefined radius stroke lets the user graph the radius of stroke over the length of stroke */
		virtual BOOL  GetPredefinedSizeEnable()=0;//tested
	//! \brief set whether a predefined radius is enabled for a stroke.
	/*! A predefined radius stroke lets the user graph the radius of stroke over the length of stroke */
		virtual void  SetPredefinedSizeEnable(BOOL enable)=0;//tested
	//! \brief get the predefined radius graph set for a stroke.
	/*! A predefined radius stroke lets the user graph the radius of stroke over the length of stroke */
		virtual ICurve *GetPredefineSizeStrokeGraph()=0;
	//! \brief get the predefined strength graph set for a stroke.
	/*! A predefined strength stroke lets the user graph the strength of stroke over the length of stroke */
		virtual ICurve *GetPredefineStrStrokeGraph()=0;

		virtual float GetNormalScale() = 0;
		virtual void SetNormalScale(float scale) = 0;

		virtual BOOL GetMarkerEnable() = 0;
		virtual void SetMarkerEnable(BOOL on) = 0;
		virtual float GetMarker() = 0;
		virtual void SetMarker(float pos) = 0;

	//! \brief Returns the mesh hit type
	/*! \return One of the following values: \n
			\t 0 = creates a plance based on your last hit point and normal\n
			\t 1 = a zdepth into the screen\n
			\t 2 = a point in world space aligned to current view */
		virtual int GetOffMeshHitType() = 0;

	//! \brief Sets the mesh hit type
	/*! \param type - One of the following values: \n
			\t 0 = creates a plance based on your last hit point and normal\n
			\t 1 = a zdepth into the screen\n
			\t 2 = a point in world space aligned to current view */
		virtual void SetOffMeshHitType(int type) = 0;

		virtual float GetOffMeshHitZDepth() = 0;
		virtual void SetOffMeshHitZDepth(float depth) = 0;

		virtual Point3 GetOffMeshHitPos() = 0;
		virtual void SetOffMeshHitPos(Point3 pos) = 0;


	};

/*! \sa Class IPainterInterface_V7
	\par Description:
	This is a class that lets you go from mesh space to patch space
	basically the painter only paints on meshes
	you can create an array of this type so you can look up a hit face 
	and then get a patch index from it and the UVW of the hit space on the patch */
	class FaceDataFromPatch: public MaxHeapOperators
	{
	public:
		int owningPatch;  //!< the patch that own this face
		Point2 st[3];	 //!< the UVW space of the corners of the patch
	};

/*! \sa Class IPainterInterface_V7
	\par Description:
	This class is used to get right-click events when the painting mode is exited.
	Implement your own local version of this virtual class, and submit it to 
	IPainterInterface_V7::StartPaintSession to be notified if the user right-clicks to exit painting. */
	class IPainterRightClickHandler: public MaxHeapOperators
	{
	public:
	//! This method cues you that the user has right-clicked to exit painting.  Handle as desired.
		virtual void RightClick ()=0;
	};

/*!	\sa IPainterInterface_V5
	\par Description:
	This class is available in Max 7 and higher only\n\n
	Any plugin that wants to be able to be painted 
	on must sub class from this class or a later version of it. */
	class IPainterInterface_V7 : public IPainterInterface_V5
	{
	public:
	//! \brief this loads of the nodes that you want to paint on
	/*! anytime you want to add delete a node this must be called.  This use the object state passed to it
		instead of the one gotten from the node to get the hit mesh.  NOTE I do not hang onto
		any of the data in the ObjectState.  I just get the a copy of the mesh from it.
		\param flags				- not yet implemented does nothing but there to allow 
									  flags per node for special conditions
		\param nodeList				- a table of nodes that you want to paint on
		\param objList				- a table of objects states one per node to use as your hit node */
		virtual BOOL  InitializeNodesByObjState(int flags, Tab<INode*> &nodeList, Tab<ObjectState> &objList) = 0;//tested

	//! \brief This forces the quadtree to rebuilt.
	/*! Any time you change a geometry or togopology
		of a node you passed to the InitializedNodes methode this must be called.
		Ideally I could listen to the notifyremessage changed and rebuild on that 
		but the since rebuilding the quad tree is inetensive, I leave it up to the 
		canvas as to when to rebuild so we can better control the amount of stack reevals
		NOTE I do not hang onto any of the data in the ObjectState.  I just get the a copy of the mesh from it.
		\param updatePointGather - determines whether the pointGather data gets updated also
								  normally if your mesh does not change shape or topo you dont
								  need to update the pointgather.  For instance ifyou rotate a view
								  your quad tree needs to get updated but not the point list
		\param objList - a table of objects states one per node to use as your hit node */
		virtual BOOL  UpdateMeshesByObjState(BOOL updatePointGather, Tab<ObjectState> &objList) = 0;

	//! \brief This lets you take a patch and get a list or tri mesh faces wher each tri has an owning
	//! patch index and the UVW space of that face.  This lets you get patch UVW space
		virtual void  GetPatchFaceData(PatchMesh &patch, Tab<FaceDataFromPatch> &faceData) = 0;

	//! \brief This overload of the method used to start a paint session allows you to submit a class
	//! which will handle right-click events in the paint mouse proc.
	/*! This allows you to end the paint proc however you need to.
		If rightClicker is NULL, or if the no-argument version of StartPaintSession is used,
		nothing will happen on right-click.
		\param rightClicker - The handler class to be notified on a right click, or NULL */
		virtual BOOL  StartPaintSession (IPainterRightClickHandler *rightClicker) = 0;
	// I have to add this one over again, otherwise it doesn't show up in the V7 interface:
	//! This is called when the a canvas wants to start a paint session.
		virtual BOOL  StartPaintSession() = 0;//tested

	//! this get the str limit min for while dragging to chnage the str
		virtual float GetStrDragLimitMin() = 0;
	//! this set the str limit min for while dragging to chnage the str
		virtual void SetStrDragLimitMin(float l) = 0;

	//! this get the str limit max for while dragging to chnage the str
		virtual float GetStrDragLimitMax() = 0;
	//! this set the str limit max for while dragging to chnage the str
		virtual void SetStrDragLimitMax(float l) = 0;


	};

/*!	\sa IPainterInterface_V14
	\par Description:
	This class is available in Max 14 (2012) and higher only\n\n
	It adds functionality for constraining the brush to a spline */
	class IPainterInterface_V14 : public IPainterInterface_V7
	{
	public:
	//! This checks if constraining the brush to a spline is turned on
		virtual BOOL GetUseSplineConstraint() = 0;
	//! This turns on and off constraining the brush to a spline
		virtual void SetUseSplineConstraint(BOOL onoff) = 0;
	//! This sets the spline node to use for constraining the brush.
	//! Returns false if the node is not a shape object.
		virtual BOOL SetSplineConstraintNode(INode* snode) = 0;
	//! This checks if a valid node has been assigned and is in the scene.
		virtual BOOL IsSplineConstraintNodeValid() = 0;
	};


/*! \sa Class IPainterInteface_V5
	\par Description: 
	This is the Max5 version of a PainterCanvas.   Any plugin that wants to be able to be painted 
	on must sub class from this class or a later version of it.
	Basically this calls contains all the methods that deal with a paint stroke. */
class IPainterCanvasInterface_V5: public MaxHeapOperators 
	{
	public:
	
	//! This is called when the user start a pen stroke
		virtual BOOL  StartStroke() = 0;


	//! \brief This is called as the user strokes across the mesh or screen with the mouse down
	/*! This only gets called if the interactive mode is off (the user has turned off Update on Mouse Up)
		\param hit - if this is a hit or not, since the user can paint off a mesh
		\param mousePos - this is mouse coords of the current hit in view space
		\param worldPoint - this is the hit point on the painted mesh in world space
		\param worldNormal - this is the normal of the hit point on the painted mesh in world space
		\param localPoint - this is the hit point on the painted mesh in local space of the mesh that was hit
		\param localNormal - this is the normal of the hit point on the painted mesh in local space of the mesh that was hit
		\param bary - this the barcentric coords of the hit point based on the face that was hit
					      this may or may not be valid depending on the state of the stack.  For instance
						  if a paint modifier was below a MeshSmooth, the barycentric coords would be based on
						  the MeshSmooth'ed mesh and not the mesh at the point of the paint modifier
		\param index - the index of the face that was hit.  See the warning in Point3 bary above.
		\param shift, ctrl, alt - the state of the shift, alt, and ctrl keys when the point was hit
		\param radius - of the radius of the brush when the point was hit, this is after all modifiers
							have been applied like pressure or predefined radius etc
		\param str - of the strength of the brush when the point was hit, this is after all modifiers
							have been applied like pressure or predefined radius etc
		\param pressure - if a pressure sensitive tablet is used this value will be the pressure of the stroke
						   ranging from 0 to 1
		\param node  - this node that got hit
		\param mirrorOn - whther the user had mirror on for the stroke, you can ignore the next for params if false
		\param worldMirrorPoint - the mirrored world stroke pos
		\param worldMirrorNormal - the mirrored world stroke normal
		\param localMirrorPoint - the mirros local stroke pos
		\param localMirrorNormal - the mirros local stroke normal */
		virtual BOOL  PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) = 0;

	//! This is called as the user ends a strokes when the users has it set to always update
		virtual BOOL  EndStroke() = 0;

	//! \brief This is called as the user ends a strokes when the users has it set to update on mouse up only
	/*! the canvas gets a list of all points, normals etc instead of one at a time
		\param ct - the number of elements in the following arrays
		\param everythingElse - see PaintStroke() these are identical except they are arrays of values */
		virtual BOOL  EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal	) = 0;

	//! This is called as the user cancels a stroke by right clicking
		virtual BOOL  CancelStroke() = 0;	

	//! This is called when the painter want to end a paint session for some reason.
		virtual BOOL  SystemEndPaintSession() = 0;

	//! \brief this is called when the painter updates the view ports
	//! this will let you do custom drawing if you need to
		virtual void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) = 0;


	};


/*! \sa Class IPainterCanvasInterface_V5
	\par Description: 
	This is the Max5.1 version of a PainterCanvas.   
	This interface changes how the command mode behaves
	It adds the ability to send back to the canvas whether to start or end the paint mode
	This bacially replaces the SystemEndPaintSession and also */
class IPainterCanvasInterface_V5_1: public MaxHeapOperators
	{
	public:

	//! \brief Called on painting start
	/*! the painter interface will call this when it wants to start the painter
		an example of this is when the user scrubs the time slider when in a paint session
		the system will turn off the paint mode while scrubbing so you will get a CanvasEndPaint
		then when the user stops scrubbing it will try to turn it back on using CanvasStartPaint.
		so if you use this interface after you call StartPaintSession() you will imediately get 
		a CanvasStartPaint callback where all your setup code and UI code should be handled */
		virtual void CanvasStartPaint()=0;
	//! \brief Called on painting end
	/*! the painter interface will call this when it wants to stop the painter
		an example of this is when the user scrubs the time slider when in a paint session
		the system will turn off the paint mode while scrubbing so you will get a CanvasEndPaint
		then when the user stops scrubbing it will try to turn it back on using CanvasStartPaint.
		so if you use this interface after you call StartPaintSession() you will imediately get 
		a CanvasStartPaint callback where all your setup code and UI code should be handled */
		virtual void CanvasEndPaint()=0;
	

	};

//! This is the additional functionality in the Max7.0 version of the PainterCanvas
class IPainterCanvasInterface_V7: public MaxHeapOperators
{
	public:
		//! \brief This method indicates that the paintbrush used to paint has changed.
		/*! (This usually cues applications to refresh their dialogs.) */
		virtual void OnPaintBrushChanged() = 0;
};

