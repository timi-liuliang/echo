/*! \file RandObjLinker.h
    \brief Class definitions for RandObjLinker
				RandObjLinker links (creates) a RandGenerator for each
				supplied Object (usually its a particle system).
				Tests/Operators that use randomization have this an instance
				of this class as a member. A Test/Operator may serve
				several particle systems. For each particle system the
				Test/Operator has to keep a dedicated copy of RandGenerator.
				This copy of RandGenerator is used exclusively for the
				designated particle system. This scheme allows not to mix
				randomization for different particle systems.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY:	created 12-04-2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\RandGenerator.h"
#include "..\maxheap.h"
#include "..\tab.h"
// forward declarations
class IObject;


class RandObjLinker: public MaxHeapOperators {
public:

	PFExport RandObjLinker();
	PFExport ~RandObjLinker();

	// to initialize random generator for a particle container "pCont"
	PFExport bool Init(IObject* pCont, int randomSeed);
	// to release random generator for a particle container "pCont"
	PFExport void Release(IObject* pCont);
	// to get a random generator associated with a particle container "pCont"
	PFExport RandGenerator* GetRandGenerator(IObject* pCont) const;
	// to deallocate all random generators and all data
	PFExport void FreeAll();

private:
	bool HasPContainer(IObject* pCont) const;
	bool AddPContainer(IObject* pCont, int randomSeed);

	// const access to class members
	int							num()							const { return m_num; }
	const Tab<IObject*>&		particleContainers()			const { return m_particleContainers; }
	IObject*					particleContainer(int index)	const { return m_particleContainers[index]; }
	const Tab<RandGenerator*>&	randGenerators()				const { return m_randGenerators; }
	RandGenerator*				randGenerator(int index)		const { return m_randGenerators[index]; }

	// access to class members
	int&					_num()							{ return m_num; }
	Tab<IObject*>&			_particleContainers()			{ return m_particleContainers; }
	IObject*&				_particleContainer(int index)	{ return m_particleContainers[index]; }
	Tab<RandGenerator*>&	_randGenerators()				{ return m_randGenerators; }
	RandGenerator*&			_randGenerator(int index)		{ return m_randGenerators[index]; }

protected:
	int m_num;
	Tab<IObject*> m_particleContainers;
	Tab<RandGenerator*> m_randGenerators;
};



