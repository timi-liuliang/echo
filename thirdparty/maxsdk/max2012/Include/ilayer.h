/**********************************************************************
 *<
	FILE: ILayer.h

	DESCRIPTION: Declaration of the ILayer interface

	CREATED BY:	Peter Sauerbrei

	HISTORY: Created 19 October 1998

 *>	Copyright (c) 1998-99, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "maxtypes.h"
#include "ref.h"
// forward declarations
class View;
class Point3;
class INode;

#define NODE_LAYER_REF		6

/*! \sa  Class ReferenceTarget, Class ILayerManager.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
The methods of this class are currently unused and reserved for internal use as
of release 3.0.\n\n
class LayerProperty : public ReferenceTarget  */
class LayerProperty : public ReferenceTarget
{
private: 
	int m_id;
	MSTR m_name;

public:
	/*! \remarks Constructor.\n\n
	This will set the layer ID to -1 and an empty name string.
	\par Default Implementation:
	<b>{ }</b> */
	LayerProperty() : m_id(-1), m_name("") {}
	/*! \remarks Constructor.\n\n
	This will initialize the layer property with the name and ID of the
	property.
	\par Default Implementation:
	<b>{ }</b> */
	LayerProperty(const MSTR & name, int id) : m_id(id), m_name(name) {}
	/*! \remarks Destructor.
	\par Default Implementation:
	<b>{ }</b> */
	virtual ~LayerProperty() {}

	// child methods
	/*! \remarks This method allows you to set the integer property.
	\par Parameters:
	<b>const int d</b>\n\n
	The property to set. */
	virtual void SetProperty(const int d) = 0;
	/*! \remarks This method allows you to set the floating point property.
	\par Parameters:
	<b>const float d</b>\n\n
	The property to set. */
	virtual void SetProperty(const float d) = 0;
	/*! \remarks This method allows you to set the Point3 property.
	\par Parameters:
	<b>const Point3 \& d</b>\n\n
	The property to set. */
	virtual void SetProperty(const Point3 & d) = 0;
	/*! \remarks This method allows you to set the string property.
	\par Parameters:
	<b>const MSTR \& d</b>\n\n
	The property to set. */
	virtual void SetProperty(const MSTR & d) = 0;
	/*! \remarks This method allows you to set the property.
	\par Parameters:
	<b>void * d</b>\n\n
	A pointer to the property data to set. */
	virtual void SetProperty(void * d) = 0;
	/*! \remarks This method returns the layer property.
	\par Parameters:
	<b>int \& i</b>\n\n
	The property data.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool GetProperty(int & i) const = 0;
	/*! \remarks This method returns the layer property.
	\par Parameters:
	<b>float \& f</b>\n\n
	The property data.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool GetProperty(float & f) const = 0;
	/*! \remarks This method returns the layer property.
	\par Parameters:
	<b>Point3 \& p</b>\n\n
	The property data.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool GetProperty(Point3 & p) const = 0;
	/*! \remarks This method returns the layer property.
	\par Parameters:
	<b>MSTR \& n</b>\n\n
	The property data.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool GetProperty(MSTR & n) const = 0;
	/*! \remarks This method returns the layer property.
	\par Parameters:
	<b>void * v</b>\n\n
	The property data.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool GetProperty(void * v) const = 0;

	// local methods
	/*! \remarks This method returns the property ID. */
	int GetID() const { return m_id; }
	/*! \remarks This method allows you to set the property ID.
	\par Parameters:
	<b>int id</b>\n\n
	The ID to set. */
	void SetID(int id) { m_id = id; }
	/*! \remarks This method returns the property name. */
	MSTR GetName() const { return m_name; }
	/*! \remarks This method allows you to set the property name.
	\par Parameters:
	<b>const MSTR \& name</b>\n\n
	The property name to set. */
	void SetName(const MSTR & name) { m_name = name; }
};

/*! \sa  Class ReferenceTarget, Class ILayerManager, Class LayerProperty, Class INode.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is an interface to the Layers functionality provided by 3D Studio
VIZ. Basically, Layers govern (override) some properties of the nodes which are
on the layer. For example, a Layer could be used to freeze all the nodes on it
without having to set this property of each node individually.\n\n
Some of the methods below are not functional in 3ds Max. Such cases are noted
in the remarks for the method.
\par Data Members:
<b>static const SClass_ID kLayerSuperClassID;</b>\n\n
The super class ID of the layer interface class.  */
class ILayer : public ReferenceTarget
{
public:
	static const SClass_ID kLayerSuperClassID;

	// from Animatable
	SClass_ID SuperClassID() { return kLayerSuperClassID; }

	// from ILayerRecord
	/*! \remarks Adds the specified node to this layer.
	\par Parameters:
	<b>INode *rtarg</b>\n\n
	The node to add. */
	virtual bool AddToLayer(INode * rtarg) = 0;
	/*! \remarks Deletes the specified node from this layer. Note: This method
	does nothing in 3ds Max.
	\par Parameters:
	<b>INode *rtarg</b>\n\n
	The node to delete from this layer. */
	virtual bool DeleteFromLayer(INode * rtarg) = 0;
	/*! \remarks Sets the name of this layer.
	\par Parameters:
	<b>const MSTR \&name</b>\n\n
	The name for this layer. */
	virtual void SetName(const MSTR & name) = 0;
	/*! \remarks Returns the name of this layer. Note: The user of this method
	must delete the returned string. */
	virtual MSTR GetName() const = 0;		// user must delete the string
	/*! \remarks Sets the wire frame color.
	\par Parameters:
	<b>DWORD newcol</b>\n\n
	See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>. */
	virtual void SetWireColor(DWORD newcol) = 0;
	/*! \remarks Returns the wire frame color. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>. */
	virtual DWORD GetWireColor() const = 0;
	/*! \remarks Sets the hidden state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for hidden; false for not hidden. */
	virtual void Hide(bool onOff) = 0;
	/*! \remarks Returns true if hidden; false if not hidden. */
	virtual bool IsHidden() const = 0;
	/*! \remarks Sets the frozen state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void Freeze(bool onOff) = 0;
	/*! \remarks Returns true if frozen; false if not. */
	virtual bool IsFrozen() const = 0;
	/*! \remarks Sets the renderable state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void SetRenderable(bool onOff) = 0;
	/*! \remarks Returns true if renderable; false if not. */
	virtual bool Renderable() const = 0;
// mjm - 06.12.00 - begin
	/*! \remarks This method allows you to set or unset the primary visibility
	flag for the layer.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset. */
	virtual void SetPrimaryVisibility(bool onOff) = 0;
	/*! \remarks This method returns TRUE if the primary visibility flag for
	the layer is set. */
	virtual bool GetPrimaryVisibility() const = 0;
	/*! \remarks This method allows you to set the secondary visibility flag
	for the layer.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset. */
	virtual void SetSecondaryVisibility(bool onOff) = 0;
	/*! \remarks This method returns TRUE if the secondary visibility flag for
	the layer is set. */
	virtual bool GetSecondaryVisibility() const = 0;
// mjm - end
	/*! \remarks Sets the X-Ray material property.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void XRayMtl(bool onOff) = 0;
	/*! \remarks Returns true if X-Ray material is set; false if not. */
	virtual bool HasXRayMtl() const = 0;
	/*! \remarks Sets the ignore extents property.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void IgnoreExtents(bool onOff) = 0;
	/*! \remarks Returns true if ignore extents is on; false if off. */
	virtual bool GetIgnoreExtents() const = 0;
	/*! \remarks Sets the box mode state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void BoxMode(bool onOff) = 0;
	/*! \remarks Returns true if box mode is on; false if off. */
	virtual bool GetBoxMode() const = 0;
	/*! \remarks Sets the all edges setting.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void AllEdges(bool onOff) = 0;
	/*! \remarks Returns true if all edges is on; false if off. */
	virtual bool GetAllEdges() const = 0;
	/*! \remarks Sets the vertex ticks state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void VertTicks(bool onOff) = 0;
	/*! \remarks Returns true if vertex ticks is on; false if off. */
	virtual bool GetVertTicks() const = 0;
	/*! \remarks Sets the backface culling state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void BackCull(bool onOff) = 0;
	/*! \remarks Returns true if backface culling is on; false if not. */
	virtual bool GetBackCull() const = 0;
	/*! \remarks Sets the color per vertex display mode.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void SetCVertMode(bool onOff) = 0;
	/*! \remarks Returns true if the color vertex display mode is on;
	otherwise false. */
	virtual bool GetCVertMode() const = 0;
	/*! \remarks Sets the shaded color vertex display mode.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void SetShadeCVerts(bool onOff) = 0;
	/*! \remarks Returns true if the shaded color vertex mode is on; false if
	off. */
	virtual bool GetShadeCVerts() const = 0;
	/*! \remarks Sets the cast shadow state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void SetCastShadows(bool onOff) = 0;
	/*! \remarks Returns true if cast shadow is on; false if off. */
	virtual bool CastShadows() const = 0;
	/*! \remarks Sets the receives shadow state.
	\par Parameters:
	<b>bool onOff</b>\n\n
	Use true for on; false for off. */
	virtual void SetRcvShadows(bool onOff) = 0;
	/*! \remarks Returns true if receives shadow is on; false if off. */
	virtual bool RcvShadows() const = 0;
// mjm - 06.12.00 - begin
	virtual void SetApplyAtmospherics(bool onOff) = 0;
	virtual bool ApplyAtmospherics() const = 0;
// mjm - end
	/*! \remarks Sets the type of motion blur used by the layer.
	\par Parameters:
	<b>int kind</b>\n\n
	The kind of motion blur. One of the following values:\n\n
	<b>0</b>: None.\n\n
	<b>1</b>: Object Motion Blur.\n\n
	<b>2</b>: Image Motion Blur. */
	virtual void SetMotBlur(int kind) = 0;
	/*! \remarks Returns the type of motion blur used by the layer. One of the
	following values:\n\n
	<b>0</b>: None.\n\n
	<b>1</b>: Object Motion Blur.\n\n
	<b>2</b>: Image Motion Blur. */
	virtual int MotBlur() const = 0;
	/*! \remarks This method is for internal use. */
	virtual int GetRenderFlags() const = 0;
	/*! \remarks This method is for internal use. */
	virtual void SetRenderFlags(int flags) = 0;
	/*! \remarks This method is for internal use. */
	virtual int GetDisplayFlags() const = 0;
	/*! \remarks This method is currently unused and reserved for future use.
	*/
	virtual int AddProperty(LayerProperty & lprop) = 0;
	/*! \remarks This method is currently unused and reserved for future use.
	*/
	virtual int SetProperty(LayerProperty & lprop) = 0;
	/*! \remarks This method is currently unused and reserved for future use.
	*/
	virtual int GetProperty(LayerProperty & lprop) const = 0;
	/*! \remarks Returns true if the layer is used (nodes have been added);
	otherwise false. */
	virtual bool Used(void) const = 0;
	/*! \remarks This method is for internal use. */
	virtual bool GetFlag(int mask) const = 0;
	/*! \remarks This method is for internal use. */
	virtual bool GetFlag2(int mask) const = 0;
	/*! \remarks This method is for internal use in VIZ. Note: This method
	does nothing in 3ds Max. */
	virtual void UpdateSelectionSet(void) = 0;
	/*! \remarks This method returns the render flags associated with the
	layer.
	\par Parameters:
	<b>int oldlimits</b>\n\n
	The old limits flag. */
	virtual int GetRenderFlags(int oldlimits) const = 0;
	/*! \remarks This method allows you to set the inherit visibility flag for
	the layer.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset. */
	virtual void SetInheritVisibility(bool onOff) = 0;
	/*! \remarks This method returns TRUE if the inherit visibility flag for
	the layer is set. */
	virtual bool GetInheritVisibility() const = 0;
	/*! \remarks This method allows you to set the display trajectory flag for
	the layer.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset.\n\n
	<b>bool temp = false</b>\n\n
	This is used internally. */
	virtual void Trajectory(bool onOff, bool temp = false) = 0;
	/*! \remarks This method returns TRUE if the display trajectory flag for
	the layer is set. */
	virtual bool GetTrajectory() const = 0;
	/*! \remarks This method allows you to set the display by layer flag on a
	per-node basis.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset.\n\n
	<b>Node *</b>\n\n
	The pointer to the node. */
	virtual void SetDisplayByLayer(BOOL onOff, INode *) = 0;
	/*! \remarks This method allows you to set the render by layer flag on a
	per-node basis.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset.\n\n
	<b>Node *</b>\n\n
	The pointer to the node. */
	virtual void SetRenderByLayer(BOOL onOff, INode *) = 0;
	/*! \remarks This method allows you to set the motion by layer flag on a
	per-node basis.
	\par Parameters:
	<b>bool onOff</b>\n\n
	TRUE to set; FALSE to unset.\n\n
	<b>Node *</b>\n\n
	The pointer to the node. */
	virtual void SetMotionByLayer(BOOL onOff, INode *) = 0;
	/*! \remarks This method returns the state of the display by layer flag
	for the specified node.
	\par Parameters:
	<b>Node *</b>\n\n
	The pointer to the node.
	\return  TRUE if set; FALSE if not set. */
	virtual BOOL GetDisplayByLayer(INode *) const = 0;
	/*! \remarks This method returns the state of the render by layer flag for
	the specified node.
	\par Parameters:
	<b>Node *</b>\n\n
	The pointer to the node.
	\return  TRUE if set; FALSE if not set. */
	virtual BOOL GetRenderByLayer(INode *) const = 0;
	/*! \remarks This method returns the state of the motion by layer flag for
	the specified node.
	\par Parameters:
	<b>Node *</b>\n\n
	The pointer to the node.
	\return  TRUE if set; FALSE if not set. */
	virtual BOOL GetMotionByLayer(INode *) const = 0;
	/*! \remarks This method will select the objects of the layer. */
	virtual void SelectObjects(void) = 0;
	virtual void SetVisibility(TimeValue t, float vis) = 0;
	virtual float GetVisibility(TimeValue t,Interval *valid=NULL) const = 0;
	virtual float GetVisibility(TimeValue t,View & view, Interval *valid=NULL) const = 0;
	/*! \remarks This method allows you to set the image blur multiplier for
	the layer.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The timevalue to get the image blur multiplier for. */
	virtual float GetImageBlurMultiplier(TimeValue t) const = 0;
	/*! \remarks This method allows you to set the image blur multiplier.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The timevalue to set the image blur multiplier for.\n\n
	<b>float m</b>\n\n
	The multiplier to set. */
	virtual void  SetImageBlurMultiplier(TimeValue t, float m) = 0;
	/*! \remarks This method returns the state of the motion blur flag at the
	specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the flag. */
	virtual bool GetMotBlurOnOff(TimeValue t) const = 0;
	/*! \remarks This method allows you to set the state of the motion blur
	flag at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the flag.\n\n
	<b>bool m</b>\n\n
	TRUE to turn on; FALSE to turn off. */
	virtual void  SetMotBlurOnOff(TimeValue t, bool m) = 0;
	virtual bool IsHiddenByVisControl() = 0;
	virtual float GetLocalVisibility(TimeValue t,Interval *valid) const = 0;

	//New methods in R4
	virtual void SetShowFrozenWithMtl( bool onOff) = 0;
	virtual int	ShowFrozenWithMtl() const = 0;

	virtual void SetRenderOccluded(bool onOff) = 0;
	virtual int	GetRenderOccluded() const = 0;

	// promoted to public method 030530  --prs.
	virtual bool HasObjects() const = 0;
};


