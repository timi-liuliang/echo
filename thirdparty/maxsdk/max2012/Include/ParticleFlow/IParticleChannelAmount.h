/*! \file IParticleChannelAmount.h
    \brief Channel-generic interfaces ParticleChannelAmount
				This is a part of every particle channel and
				particle container
				ParticleChannelAmount is a two-part interface:
				for read (R) and write (W)
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-05-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\ifnpub.h"

// interface ID
#define PARTICLECHANNELAMOUNTR_INTERFACE Interface_ID(0x74f93b00, 0x1eb34500)
#define PARTICLECHANNELAMOUNTW_INTERFACE Interface_ID(0x74f93b00, 0x1eb34501)

#define GetParticleChannelAmountRInterface(obj) ((IParticleChannelAmountR*)obj->GetInterface(PARTICLECHANNELAMOUNTR_INTERFACE))
#define GetParticleChannelAmountWInterface(obj) ((IParticleChannelAmountW*)obj->GetInterface(PARTICLECHANNELAMOUNTW_INTERFACE))


class IParticleChannelAmountR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kCount
	};

	// Function Map for Function Publish System
	//***********************************
	#pragma warning(push)
	#pragma warning(disable:4100)
	BEGIN_FUNCTION_MAP

	FN_0(kCount, TYPE_INT, Count);

	END_FUNCTION_MAP
	#pragma warning(pop)
      /** @defgroup IParticleChannelAmount IParticleChannelAmount.h
      *  @{
      */

      /*! \fn virtual int		Count() const = 0; 
      *  \brief Particle count management; number of particles in the channel
      */
	virtual int		Count() const = 0; 

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELAMOUNTR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELAMOUNTR_INTERFACE); }
};

class IParticleChannelAmountW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kZeroCount,
			kSetCount,
			kDeleteByIndex,
			kDeleteByArray,
			kSplit,
			kSpawn,
			kAppendNum,
			kAppend
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_0(kZeroCount, ZeroCount);
	FN_1(kSetCount, TYPE_bool, SetCount, TYPE_INT);
	FN_2(kDeleteByIndex, TYPE_INT, Delete, TYPE_INT, TYPE_INT);
	FN_1(kDeleteByArray, TYPE_INT, Delete, TYPE_BITARRAY_BR);
	FN_1(kSplit, TYPE_IOBJECT, Split, TYPE_BITARRAY_BR);
	FN_1(kSpawn, TYPE_bool, Spawn, TYPE_INT_TAB_BR);
	FN_1(kAppendNum, TYPE_bool, AppendNum, TYPE_INT);
	FN_1(kAppend, TYPE_bool, Append, TYPE_IOBJECT);

	END_FUNCTION_MAP


      /** @defgroup IParticleChannelAmount IParticleChannelAmount.h
      *  @{
      */

      /*! \fn virtual void	ZeroCount() = 0;
      *  \brief set number of particles in the channel to zero
      */
	virtual void	ZeroCount() = 0;  

      /*! \fn virtual bool	SetCount(int n) = 0; 
      *  \brief set number of particles in the channel to n
      */
	virtual bool	SetCount(int n) = 0; 

      /*! \fn virtual int		Delete(int start,int num) = 0;
      *  \brief Delete num particles from start index. Returns number of particles left in the channel.
      */
	virtual int		Delete(int start,int num) = 0;

      /*! \fn virtual int		Delete(BitArray& toRemove) = 0;
      *  \brief Delete particles according to the BitArray (bit set == remove). Returns number of particles left in the channel
      */
	virtual int		Delete(BitArray& toRemove) = 0;

      /*! \fn virtual IObject*		Split(BitArray& toSplit) = 0;
      *  \brief To new "split" particle channel. Returns new particle channel; IObject type is chosen to be
	   compatible with the available return types. All "bit-set" particles are moved.
      */
	virtual IObject*		Split(BitArray& toSplit) = 0;
	   
      /*! \fn virtual bool	Spawn( Tab<int>& spawnTable ) = 0; 
      *  \brief Spawn particles according to the spawnTable. Returns true if the operation is successful.
	   Size of the table equals to number of particles in the channel. Table value is how many particles are cloned from the index particle.
      */
	virtual bool	Spawn( Tab<int>& spawnTable ) = 0; 

      /*! \fn virtual bool	AppendNum(int num) = 0; 
      *  \brief Append num particles to the channel(s). Returns true if the operation is successful.
      */
	virtual bool	AppendNum(int num) = 0; 

      /*! \fn virtual bool	Append(IObject* obj) = 0; 
      *  \brief Append all particles of another channel/container, then the input channel/container is deleted.
	   Returns true if the operation is successful.
      */
	virtual bool	Append(IObject* obj) = 0; 

      /*! \fn virtual void	ResetAddedAmount() { ; }
      *  \brief For internal use; no need to implement
      */
	virtual void	ResetAddedAmount() { ; }

      /*! \fn virtual int		GetAddedAmount() { return 0; }
      *  \brief For internal use; no need to implement
      */
	virtual int		GetAddedAmount() { return 0; }

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELAMOUNTW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELAMOUNTW_INTERFACE); }
};

