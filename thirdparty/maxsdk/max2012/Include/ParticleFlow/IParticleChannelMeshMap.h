/*! \file IParticleChannelMeshMap.h
    \brief Channel-generic interface for particle channels
				 that store data in "mesh map" form
				 The channel store a set of MeshMapChannels
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "IParticleChannelMap.h"

// standard particle channel "Mesh Map"
// interface ID
#define PARTICLECHANNELMESHMAPR_INTERFACE Interface_ID(0x74f93c17, 0x1eb34500)
#define PARTICLECHANNELMESHMAPW_INTERFACE Interface_ID(0x74f93c17, 0x1eb34501)

#define GetParticleChannelMeshMapRInterface(obj) ((IParticleChannelMeshMapR*)obj->GetInterface(PARTICLECHANNELMESHMAPR_INTERFACE))
#define GetParticleChannelMeshMapWInterface(obj) ((IParticleChannelMeshMapW*)obj->GetInterface(PARTICLECHANNELMESHMAPW_INTERFACE))

class IParticleChannelMeshMapR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetNumMaps,
			kMapSupport,
			kGetMapReadChannel
	};

	BEGIN_FUNCTION_MAP

	FN_0(kGetNumMaps, TYPE_INT, GetNumMaps);
	FN_1(kMapSupport, TYPE_bool, MapSupport, TYPE_INT);
	FN_1(kGetMapReadChannel, TYPE_INTERFACE, GetMapReadChannel, TYPE_INT);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelMeshMap IParticleChannelMeshMap.h
    *  @{
    */

      /*! \fn virtual int GetNumMaps() const = 0;
      *  \brief returns the number of mapping channels in use.
      */
	virtual int GetNumMaps() const = 0;

      /*! \fn virtual bool MapSupport(int mp) const = 0;
      *  \brief returns true if the specified mapping channel is supported; otherwise false
	\param int mp: specifies the channel.  See List of Mapping Channel Index Values.
      */
	virtual bool MapSupport(int mp) const = 0;

      /*! \fn virtual IParticleChannelMapR* GetMapReadChannel(int mp) = 0;
      *  \brief returns an interface if the specified mapping channel is supported; otherwise NULL
	\param int mp: specifies the channel.  See List of Mapping Channel Index Values.
      */
	virtual IParticleChannelMapR* GetMapReadChannel(int mp) = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHMAPR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHMAPR_INTERFACE); }
};


class IParticleChannelMeshMapW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetNumMaps,
			kSetMapSupport,
			kGetMapChannel,
			kGetMapChannelObject
	};

	BEGIN_FUNCTION_MAP

	VFN_2(kSetNumMaps, SetNumMaps, TYPE_INT, TYPE_bool);
	VFN_2(kSetMapSupport, SetMapSupport, TYPE_INT, TYPE_bool);
	FN_1(kGetMapChannel, TYPE_INTERFACE, GetMapChannel, TYPE_INT);
	FN_1(kGetMapChannelObject, TYPE_IOBJECT, GetMapChannelObject, TYPE_INT);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelMeshMap IParticleChannelMeshMap.h
    *  @{
    */

      /*! \fn virtual void SetNumMaps(int ct, bool keep=false) = 0;
      *  \brief sets the number of texture maps used.  Note that this call is made automatically if SetMapSupport() is called.
	\param int ct: the number of texture maps to use.  This is a value between 0 and MAX_MESHMAPS-1.
	\param bool keep: true to keep the old mapping information after the resize; false to discard it.
      */
	virtual void SetNumMaps(int ct, bool keep=false) = 0;

      /*! \fn virtual void SetMapSupport(int mp, bool support=true) = 0;
      *  \brief sets whether the specified mapping channels is supported or not.
	\param int mp: specifies the channel.  See List of Mapping Channel Index Values.
	\param bool support: true to indicate the channel is supported; otherwise false.
      */
	virtual void SetMapSupport(int mp, bool support=true) = 0;

      /*! \fn virtual IParticleChannelMapW* GetMapChannel(int mp) = 0;
      *  \brief returns an interface if the specified mapping channel is supported; otherwise NULL
	\param int mp: specifies the channel. See List of Mapping Channel Index Values.
      */
	virtual IParticleChannelMapW* GetMapChannel(int mp) = 0;

      /*! \fn virtual IObject* GetMapChannelObject(int mp) const = 0;
      *  \brief returns an object if the specified mapping channel is supported; otherwise NULL
	\param int mp: specifies the channel. See List of Mapping Channel Index Values.
      */
	virtual IObject* GetMapChannelObject(int mp) const = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHMAPW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHMAPW_INTERFACE); }
};


