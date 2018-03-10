/*! \file ParticleChannelMask.h
    \brief An interface for ParticleChannelMask.
				 The interface is used by Operators/Elements to define
				 what channels are read/write.
				 The class is implemented by system.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 9-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include <WTypes.h>
#include "..\MaxHeap.h"
// forward declarations
class Interface_ID;

// all generic particle channel listed (implemented by max)
enum PCU_genericChannelIndex {
	PCG_float=		1<<0,
	PCG_int=		1<<1,
//	PCG_RGBA=		1<<2,
	PCG_Point3=		1<<3,
	PCG_bool=		1<<4,
//	PCG_String=		1<<5,
//	PCG_TimeValue=	1<<6,
	PCG_PTV=		1<<7,	// Precise Time Value
//	PCG_TexMap=		1<<8,
	PCG_INode=		1<<9,
	PCG_Matrix3=	1<<10,
	PCG_void=		1<<11,
//	PCG_Interval=	1<<12,
	PCG_AngAxis=	1<<13,
	PCG_Quat=		1<<14,
//	PCG_Point2=		1<<15,
//	PCG_BitArray=	1<<16,
	PCG_Mesh=		1<<17,
//	PCG_TabPoint3=	1<<18,
//	PCG_TabFace=	1<<19
	PCG_TabUVVert=	1<<20,
	PCG_TabTVFace=	1<<21,
	PCG_Map=		1<<22,
	PCG_MeshMap=	1<<23
};

// all standard particle channels listed (implemented by max)
// Note to Developer: update index2InterfaceID and InterfaceID2index methods 
// if number/content of standard classes is changed
// maximum amount of standard particle channels is 31; restricted by bit size of DWORD
enum PCU_channelIndex {
	PCU_Undefined=		   0,
	PCU_Amount=			1<<0, // fictional channel to indicate that Operator changes number of particles
	PCU_New=			1<<1, // channel to trace newly born/entered particles
	PCU_ID=				1<<2, // channel to keep particleID=(index,born)
	PCU_Time=			1<<3, // current valid time for a particle (PCG_PTV)
	PCU_BirthTime=		1<<4, // time when particle was born (PCG_PTV)
	PCU_EventStart=		1<<5, // time when particle entered the current event (PCG_PTV)
	PCU_Position=		1<<6, // particle position (PCG_Point3)
	PCU_Speed=			1<<7, // particle speed (PCG_Point3)
	PCU_Acceleration=	1<<8, // particle acceleration (PCG_Point3)
	PCU_Orientation=	1<<9, // 3D orientation of a particle (PCG_Quad)
	PCU_Spin=			1<<10, // angular velocity of a particle (PCG_AngAxis)
	PCU_Scale=			1<<11, // scaling factor for particle (Point3: x, y, z axes)
	PCU_Shape=			1<<12, // shape as a whole mesh (PCG_Mesh)
//	PCU_ShapeTopology=	1<<13, // topology data (Tab<Face>)
//	PCU_ShapeGeometry=	1<<14, // geometry data (Tab<Point3>)
//	PCU_ShapeNormals=	1<<15, // normals data
	PCU_ShapeTexture=	1<<16, // texture coordinate data
//	PCU_Mass=			1<<17, // particle mass
	PCU_MtlIndex=		1<<18, // material index for particles
	PCU_Selection=		1<<19,  // selection status
	PCU_MXSInteger=		1<<20,	// maxscript integer value
	PCU_MXSFloat=		1<<21,	// maxscript float value
	PCU_MXSVector=		1<<22,	// maxscript Point3 value
	PCU_MXSMatrix=		1<<23,	// maxscript Matrix3 value
	PCU_DeathTime=		1<<24,	// time when particle is expected to die (PCG_PTV)
	PCU_Lifespan=		1<<25	// lifespan of a particle
};

class ParticleChannelMask: public MaxHeapOperators 
{
public:

    /** @defgroup ParticleChannelMask ParticleChannelMask.h
    *  @{
    */

      /*! \fn  PFExport ParticleChannelMask();
      *  \brief constructors/destructor/assigning
      */
	PFExport ParticleChannelMask();

      /*! \fn  PFExport ParticleChannelMask(const ParticleChannelMask&);
      *  \brief constructors/destructor/assigning
      */
	PFExport ParticleChannelMask(const ParticleChannelMask&);

      /*! \fn  PFExport ParticleChannelMask(DWORD readMask, DWORD writeMask);
      *  \brief constructors/destructor/assigning
      */
	PFExport ParticleChannelMask(DWORD readMask, DWORD writeMask);

      /*! \fn  PFExport ParticleChannelMask& operator=(const ParticleChannelMask&);
      *  \brief constructors/destructor/assigning
      */
	PFExport ParticleChannelMask& operator=(const ParticleChannelMask&);

      /*! \fn  PFExport ~ParticleChannelMask();
      *  \brief constructors/destructor/assigning
      */
	PFExport ~ParticleChannelMask();

      /*! \fn  PFExport int operator==(const ParticleChannelMask&) const;
      *  \brief Test for equality
      */
	PFExport int operator==(const ParticleChannelMask&) const;

      /*! \fn  PFExport int operator!=(const ParticleChannelMask&) const;
      *  \brief Test for equality
      */
	PFExport int operator!=(const ParticleChannelMask&) const;

      /*! \fn  PFExport void SetReadChannels(DWORD channelsFlag);
      *  \brief set standard read/write and custom channels methods. If a custom particle channel is indeed a standard particle channel then it's stored as a flag
      */
	PFExport void SetReadChannels(DWORD channelsFlag);

      /*! \fn  PFExport void SetWriteChannels(DWORD channelsFlag);
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport void SetWriteChannels(DWORD channelsFlag);

      /*! \fn  PFExport bool SetChannels(int numChannels, Interface_ID* channels);
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport bool SetChannels(int numChannels, Interface_ID* channels);

      /*! \fn  PFExport void AddReadChannels(DWORD channelsFlag); // adds more standard read channels 
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport void AddReadChannels(DWORD channelsFlag); // adds more standard read channels

      /*! \fn  PFExport void RemoveReadChannels(DWORD channelsFlag); // removes some standard read channels 
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport void RemoveReadChannels(DWORD channelsFlag); // removes some standard read channels

      /*! \fn  PFExport void AddWriteChannels(DWORD channelsFlag); // adds more standard write channels 
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport void AddWriteChannels(DWORD channelsFlag); // adds more standard write channels

      /*! \fn  PFExport void RemoveWriteChannels(DWORD channelsFlag); // removes some standard write channels 
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport void RemoveWriteChannels(DWORD channelsFlag); // removes some standard write channels

      /*! \fn  PFExport bool AddChannel(Interface_ID& channel);
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport bool AddChannel(Interface_ID& channel);

      /*! \fn  PFExport bool RemoveChannel(Interface_ID& channel);
      *  \brief See PFExport void SetReadChannels()
      */
	PFExport bool RemoveChannel(Interface_ID& channel);

      /*! \fn  PFExport const int GetNumChannels() const; // standard read/write and custom together 
      *  \brief get channels methods
      */
	PFExport const int GetNumChannels() const; // standard read/write and custom together

      /*! \fn  PFExport const Interface_ID GetChannel(int i) const;
      *  \brief 
      */
	PFExport const Interface_ID GetChannel(int i) const;

      /*! \fn  PFExport const bool HasChannel(Interface_ID& channel) const;
      *  \brief has channels methods. Returns true if the channel is present
      */
	PFExport const bool HasChannel(Interface_ID& channel) const;

      /*! \fn  PFExport const bool HasReadChannels(DWORD channelsFlag) const;
      *  \brief returns true if all readChannels are present
      */
	PFExport const bool HasReadChannels(DWORD channelsFlag) const;

      /*! \fn  PFExport const bool HasWriteChannels(DWORD channelsFlag) const;
      *  \brief returns true if all writeChannels are present
      */
	PFExport const bool HasWriteChannels(DWORD channelsFlag) const;

      /*! \fn  PFExport static int NumStandardParticleChannels();
      *  \brief maximum number of available standard particle channels (constant)
      */
	PFExport static int NumStandardParticleChannels();

      /*! \fn  PFExport static const ParticleChannelMask& Null();
      *  \brief empty mask
      */
	PFExport static const ParticleChannelMask& Null();

private:
	// conversion between channel index and standard channel Class_ID
		// returns PCU_Undefined if the channel isn't standard particle channel
	PFExport static const PCU_channelIndex InterfaceID2readIndex(Interface_ID& id);
	PFExport static const PCU_channelIndex InterfaceID2writeIndex(Interface_ID& id);
		// returns false if not a valid index
	PFExport static const Interface_ID readIndex2InterfaceID(PCU_channelIndex index);
	PFExport static const Interface_ID writeIndex2InterfaceID(PCU_channelIndex index);

	void invalidateNumSRChannels() { _numSRChannelsValid() = false; }
	void invalidateNumSWChannels() { _numSWChannelsValid() = false; }

	// const access to class members
	const DWORD			readFlag()					const	{ return m_readFlag; }
	const DWORD			writeFlag()					const	{ return m_writeFlag; }
	const bool			numSRChannelsValid()		const	{ return m_numSRChannelsValid; }
	const bool			numSWChannelsValid()		const	{ return m_numSWChannelsValid; }
	const int			numStandardReadChannels()	const; // updates num of standard read channels if invalid
	const int			numStandardWriteChannels()	const; // updates num of standard write channels if invalid
	const int			numCustomChannels()			const	{ return m_numCustomChannels; }
	const Interface_ID*	customChannels()			const	{ return m_customChannels; }

	// access to class members
	DWORD&			_readFlag()					{ return m_readFlag; }
	DWORD&			_writeFlag()				{ return m_writeFlag; }
	bool&			_numSRChannelsValid()		{ return m_numSRChannelsValid; }
	bool&			_numSWChannelsValid()		{ return m_numSWChannelsValid; }
	int&			_numStandardReadChannels()	{ return m_numStandardReadChannels; }
	int&			_numStandardWriteChannels()	{ return m_numStandardWriteChannels; }
	int&			_numCustomChannels()		{ return m_numCustomChannels; }
	Interface_ID*&	_customChannels()			{ return m_customChannels; }


protected:

	DWORD m_readFlag, m_writeFlag;
	mutable bool m_numSRChannelsValid, m_numSWChannelsValid;
	mutable int m_numStandardReadChannels, m_numStandardWriteChannels;
	int m_numCustomChannels;
	Interface_ID *m_customChannels;
	const static DWORD kChannelsCast;
	const static int kNumStandardParticleChannels;
	const static ParticleChannelMask kNull; // empty mask
};

