/**********************************************************************
 *<
	FILE: channel.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/



#pragma once
#include "maxtypes.h"

typedef ULONG ChannelMask;

/*! \name Channels within the object. */
//!@{
#define NUM_OBJ_CHANS 11
//!@}

/*! \defgroup ObjectChannels Object Channels
Some of the sample code specifies these channels as PART_* as opposed to *_CHANNEL.
For example, PART_GEOM|PART_TOPO instead of GEOM_CHANNEL|TOPO_CHANNEL. 
The proper usage is the *_CHANNEL version.
\sa The Geometry Pipeline System, Modifiers, and Class Mesh in the <b>3ds Max Programmer's Guide</b>, and
%Object Channel Indices <a href="#Object_Channel_Indices">here</a>.

*/
//!@{

/*! \name Channels
<a name="Channels"></a>
The pipeline is divided into the following channels: */

//@{
//! \brief The topology channel - the face or polygon structures. 
/*! Smoothing groups and materials are also part of this channel. 
Edge visibility is also part of this channels since it is an attribute of the face structure. */
#define TOPO_CHANNEL    	(1<<0)
//! \brief The vertices of the object. 
/*! Most modifiers only alter this channel. */
#define GEOM_CHANNEL    	(1<<1)
//! \brief The texture vertices and procedural mappings.
#define TEXMAP_CHANNEL  	(1<<2)
//! \brief This is no longer used. 
/*! Materials are rolled into the Face data structure and are part of the topology channel. */
#define MTL_CHANNEL     	(1<<3)
//! \brief The sub-object selection channel. 
/*! An object's selection flows down the pipeline. What the selection is actually 
comprised of is up to the specific object type. For example, TriObjects have bits 
for face, edge and vertex selection. This channel is the actual BitArray used, 
like selLevel of the class Mesh. */
#define SELECT_CHANNEL  	(1<<4)
//! \brief The current level of selection
/*! Every object that flows down the pipeline is at a certain level that corresponds 
to the Sub-Object drop down in the 3ds Max user interface. This channel indicates 
which level the object is at. This is also specific to the object type. There are 
32 bits to represent the level of selection. When all the bits are 0, the object 
is at object level selection. */
#define SUBSEL_TYPE_CHANNEL (1<<5)
//! \brief The miscellaneous bits controlling the item's display. 
/*! These bits are specific to the type of object. For the Mesh object these 
are the surface normal scale, display of surface normals, edge visibility and display flags. */
#define DISP_ATTRIB_CHANNEL (1<<6)
//! \brief The color per vertex channel. 
/*! This is also used for the second texture mapping channel. */
#define VERTCOLOR_CHANNEL   (1<<7)
//! \brief The used internally by 3ds Max for stripping. 
/*! Plug-In developers don't need to specify this channel as being changed or used in their plug-ins. */
#define GFX_DATA_CHANNEL	(1<<8) // stripping, edge list, etc.
//! \brief Displacement approximation
#define DISP_APPROX_CHANNEL	(1<<9) 
//! \brief The channel used by extension channel objects.
#define EXTENSION_CHANNEL	(1<<13)
//! \brief The ObjectState TM that flows down the pipeline. 
/*! This TM may be modified by modifiers. */
#define TM_CHANNEL		(1<<10)
// watje
//! \brief For internal use
/*! Edge visiblity channel used internally to create an edit mesh edge vis cache */
#define EDGEVISIBLITY_CHANNEL		(1<<11)
// watje
//! \brief For internal use
/*! A channel used to determine whether we should use tri strips or not */
#define DONT_RECREATE_TRISTRIP_CHANNEL		(1<<12)
//! \brief This is no longer used. 
/*! Material applied to object as whole. Materials are rolled into the Face data 
structure and are part of the topology channel. */
#define GLOBMTL_CHANNEL (1<<31)
//! \todo Describe this symbol
#define OBJ_CHANNELS  (TOPO_CHANNEL|GEOM_CHANNEL|SELECT_CHANNEL|TEXMAP_CHANNEL|MTL_CHANNEL|SUBSEL_TYPE_CHANNEL|DISP_ATTRIB_CHANNEL|VERTCOLOR_CHANNEL|GFX_DATA_CHANNEL|DISP_APPROX_CHANNEL|EXTENSION_CHANNEL)
//! \todo Describe this symbol
#define ALL_CHANNELS  (OBJ_CHANNELS|TM_CHANNEL|GLOBMTL_CHANNEL)

//@}

/*! \name Object Channel Indices
<a name="Object_Channel_Indices"></a>
The following are the indices that may be used for the object channels. These values 
are used in the methods Object::UpdateValidity(), Object::SetChannelValidity() and 
Object::ChannelValidity(). */
//!@{
//! \brief The topology channel number.
#define TOPO_CHAN_NUM 0
//! \brief The geometry channel number.
#define GEOM_CHAN_NUM 1
//! \brief The texture vertices and procedural mappings channels number.
#define TEXMAP_CHAN_NUM 2
//! \brief This is no longer used.
#define MTL_CHAN_NUM 3
//! \brief The sub-object selection channel number.
#define SELECT_CHAN_NUM 4
//! \brief This is the current level of selection number.
#define SUBSEL_TYPE_CHAN_NUM 5 
//! \brief The display channel number. 
#define DISP_ATTRIB_CHAN_NUM 6
//! \brief The vertex colors number. This is also used for the second mapping channel. 
#define VERT_COLOR_CHAN_NUM 7
//! \brief The stripping, edge list, etc. channel number.
#define GFX_DATA_CHAN_NUM 8
//! \brief The displacement approximation channel number.
#define DISP_APPROX_CHAN_NUM 9
//! \brief The extension object channel number.
/** Developers tend to confuse these object channel numbers 
(\ref TOPO_CHAN_NUM, \ref GEOM_CHAN_NUM, etc.) and the channel bits (\ref TOPO_CHANNEL, 
\ref GEOM_CHANNEL, etc.). Some methods refer to the channel by number and some 
by bit. Developers must not confuse these two as the compiler will not catch 
this as an error. See <a href="#Channels">Channels</a>. */
#define EXTENSION_CHAN_NUM 10

//!@}

//!@}      ******* END OF ObjectChannels

