/*! \file IParticleContainer.h
    \brief An interface class to ParticleContainer. ParticleContainer is a collection of different particle channels.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 9-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "IParticleChannelAmount.h"
#include "IChannelContainer.h"

class IParticleContainer:	public IObject, 
							public IChannelContainer,
							public IParticleChannelAmountR,
							public IParticleChannelAmountW
{
public:

      /** @defgroup IParticleContainer IParticleContainer.h
      *  @{
      */

      /*! \fn  MCHAR* GetIObjectName();
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	MCHAR* GetIObjectName();

      /*! \fn  int NumInterfaces();
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	int NumInterfaces();

      /*! \fn  BaseInterface* GetInterfaceAt(int index);
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	BaseInterface* GetInterfaceAt(int index);

      /*! \fn  BaseInterface* GetInterface(Interface_ID id);
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	BaseInterface* GetInterface(Interface_ID id);

      /*! \fn  void AcquireIObject();
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	void AcquireIObject();

      /*! \fn  void ReleaseIObject();
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	void ReleaseIObject();

      /*! \fn  void DeleteIObject();
      *  \brief Implemented in ParticleContainer as the corresponding interface from IObject interface
      */
//	void DeleteIObject();

      /*! \fn  FPInterfaceDesc* GetDescByID(Interface_ID id);
      *  \brief from FPMixinInterface
      */
//	FPInterfaceDesc* GetDescByID(Interface_ID id);

      /*! \fn  int NumChannels() const;
      *  \brief from IChannelContainer
      */
//	int NumChannels() const;

      /*! \fn  IObject* GetChannel(int i) const;
      *  \brief from IChannelContainer
      */
//	IObject* GetChannel(int i) const;

      /*! \fn  bool AddChannel(IObject* channel);
      *  \brief from IChannelContainer
      */
//	bool AddChannel(IObject* channel);

      /*! \fn  int Count() const;
      *  \brief from IParticleChannelAmountR
      */
//	int Count() const;

      /*! \fn  void	ZeroCount();
      *  \brief from IParticleChannelAmountW
      */
//	void	ZeroCount();

      /*! \fn  bool	SetCount(int n);
      *  \brief from IParticleChannelAmountW
      */
//	bool	SetCount(int n);

      /*! \fn  int	Delete(int start,int num);
      *  \brief from IParticleChannelAmountW
      */
//	int	Delete(int start,int num);

      /*! \fn  int	Delete(BitArray& toRemove);
      *  \brief from IParticleChannelAmountW
      */
//	int	Delete(BitArray& toRemove);

      /*! \fn  FPInterface*	Split(BitArray& toSplit);
      *  \brief from IParticleChannelAmountW
      */
//	FPInterface*	Split(BitArray& toSplit);

      /*! \fn  bool	Spawn( Tab<int>& spawnTable );
      *  \brief from IParticleChannelAmountW
      */
//	bool	Spawn( Tab<int>& spawnTable );

      /*! \fn  bool	AppendNum(int num);
      *  \brief from IParticleChannelAmountW
      */
//	bool	AppendNum(int num);

      /*! \fn  bool	AppendChannel(IObject* channel);
      *  \brief from IParticleChannelAmountW
      */
//	bool	AppendChannel(IObject* channel);

      /*! \fn  void DeleteThis() { DeleteIObject(); }
      *  \brief to make "Delete" interface similar to Animatable
      */
//	void DeleteThis() { DeleteIObject(); }
};

