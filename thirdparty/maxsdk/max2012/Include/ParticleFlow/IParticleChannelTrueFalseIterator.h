/**********************************************************************
 *<
	FILE: IParticleChannelTrueFalseIterator.h

	DESCRIPTION: Channel-generic interface for boolean type particle channels.
				The interface is used for fast iteration through the subset of
				true (or false) values. 

	CREATED BY:	Oleg Bayborodin

	HISTORY:	created 2007-09-05

 *>	Copyright (c) 2007 Orbaz Technologies, Inc. All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleContainer.h"
#include "..\ifnpub.h"
#include "..\bitarray.h"
// forward declarations
class OrbazTrueBlock;

// generic particle channel "group selection"
// interface ID
#define PARTICLECHANNELTRUEFALSEITERATOR_INTERFACE Interface_ID(0x91cd191c, 0x1eb34500)
#define GetParticleChannelTrueFalseIteratorInterface(obj) ((IParticleChannelTrueFalseIterator*)obj->GetInterface(PARTICLECHANNELTRUEFALSEITERATOR_INTERFACE))


class IParticleChannelTrueFalseIterator : public FPMixinInterface
{
public:

	// resets iterator to first true value of the channel; returns particle index
	// if unable to find a true value then returns number of particles in the channel (end index)
	virtual int	GetFirstTrue(void) = 0;
	// for multi-threaded usage
	virtual int GetFirstTrue(int& localIndex) = 0;

	// finds the next true value; returns particle index
	// if unable to find the next true value then returns number of particles in the channel (end index)
	virtual int	GetNextTrue(void) = 0;
	// for multi-threaded usage
	virtual int GetNextTrue(int& localIndex) const = 0;

	// updates the table of true values and returns number of true indices
	virtual int GetTrueCount(void) = 0;
	// returns index of the i-th true value
	virtual int GetTrueIndex(int i) const = 0;

	// update the table of blocks of true values and returns number of trueBlocks
	virtual int GetTrueBlockCount(void) = 0;
	// returns TrueBlock with given index
	virtual const OrbazTrueBlock& GetTrueBlock(int i) const = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTRUEFALSEITERATOR_INTERFACE); }
};

class OrbazTrueEnumerator : public BitArrayCallback
{
public:
	OrbazTrueEnumerator(int*& iterIndices)
		: m_iterIndices(iterIndices)
		, m_index(0)
	{
	}

	void proc(int n)
	{
		m_iterIndices[m_index++] = n;
	}
	OrbazTrueEnumerator& operator=(const OrbazTrueEnumerator& rhs)
	{
		if (this != & rhs)
		{
			m_iterIndices = rhs.m_iterIndices;
			m_index       = rhs.m_index;
		}
		return *this;
	}
private:
	int*&		m_iterIndices;
	int			m_index;
};

class OrbazTrueBlock : public MaxHeapOperators
{
public:	
	OrbazTrueBlock()
		:	m_majorIndex(0)
		,	m_blockIndices(0)
	{
	}
	OrbazTrueBlock(const OrbazTrueBlock& block)
		:	m_majorIndex(block.m_majorIndex)
		,	m_blockIndices(block.m_blockIndices)
	{
	}
	OrbazTrueBlock(int majorIndex, DWORD32 blockIndices)
		:	m_majorIndex(majorIndex)
		,	m_blockIndices(blockIndices)
	{
	}

	inline int GetMajorIndex(void) const { return m_majorIndex; }
	inline void	SetMajorIndex(int index) { m_majorIndex = index; }
	void AddBlockIndex(int index) { m_blockIndices |= (1<<(index&kBitsPerDWORDMask)); }

	// returns number of bits set in the TrueBlock
	int NumberSet(void) const
	{
		static NumberBitsSetInUChar nbc;
		int numSet = 0;
		uchar*  miniBlocks = (uchar*)(&m_blockIndices);
		for(int i=0; i<4; ++i)
			numSet += nbc.getNumBitsSet(miniBlocks[i]);
		return numSet;
	}

	// returns number of trueBlocks allocated and set
	static int SetAllTrueBlocks(int particleCount, OrbazTrueBlock*& trueBlocks)
	{
		if (trueBlocks != NULL)
		{
			delete [] trueBlocks;
			trueBlocks = NULL;
		}

		int numTrueBlocks = particleCount>>5;
		bool incompleteEndBlock = (particleCount%kBitsPerDWORD != 0);
		int numCompleteBlocks = numTrueBlocks;
		if (incompleteEndBlock)
			++numTrueBlocks;
		trueBlocks = new OrbazTrueBlock[numTrueBlocks];
		int i=0;
		for(; i<numCompleteBlocks; ++i)
			trueBlocks[i] = OrbazTrueBlock(i<<5, 0xFFFFFFFF);
		if (incompleteEndBlock)
			for(int j=i<<5; j<particleCount; ++j)
				trueBlocks[i].AddBlockIndex(j);
		return numTrueBlocks;
	}

	static int GetMaxNumTrueBlocks(int particleCount)
	{
		int numTrueBlocks = particleCount >> 5;
		if (particleCount % kBitsPerDWORD != 0)
			++numTrueBlocks;
		return numTrueBlocks;
	}

	static int OptimizeMemoryUsage(int numBlocksSet, int numBlocksAllocated, OrbazTrueBlock*& trueBlocks)
	{
		if (numBlocksSet != numBlocksAllocated)
		{
			OrbazTrueBlock* temp = new OrbazTrueBlock[numBlocksSet];
			memcpy(temp, trueBlocks, sizeof(OrbazTrueBlock)*numBlocksSet);
			delete [] trueBlocks;
			trueBlocks = temp;
		}
		return numBlocksSet;
	}

protected:
	int		m_majorIndex;
	DWORD32 m_blockIndices;

private:
	static const int kBitsPerDWORDMask = 31;
	static const int kBitsPerDWORD = 32;

	class NumberBitsSetInUChar : public MaxHeapOperators
	{
	public:
		NumberBitsSetInUChar()
		{
			m_numBits[0] = 0;
			for(int i=0, j=1, index=1; i<8; ++i, j<<1)
			{
				int lowIndex = index-j;
				for(int k=0; k<j; ++k)
				{
					m_numBits[index++] = m_numBits[lowIndex++] + 1;
				}
			}
		}

		inline uchar getNumBitsSet(uchar n)
		{
			return m_numBits[n];
		}

	private:
		uchar m_numBits[256];
	};
};

class OrbazTrueBlockIterator : public OrbazTrueBlock
{
public:
	OrbazTrueBlockIterator(const OrbazTrueBlock& block)
		:	OrbazTrueBlock(block)
		,	m_iter(0)
		,	m_mask(1)
	{
	}

	// finds the next true value; if used for the first time after constructor finds first true value of the block;
	// return particle index; if unable to find the next true value then returns -1
	int GetNextTrue(void)
	{
		for(; m_iter < kBitsPerDWORD; ++m_iter, m_mask<<=1)
		{
			if (m_mask & m_blockIndices)
			{
				m_mask <<= 1;
				return (m_majorIndex + (m_iter++));
			}
		}
		return -1;
	}

	static const int kBitsPerDWORD = 32;

private:
	OrbazTrueBlockIterator(void)
		:	OrbazTrueBlock()
		,	m_iter(0)
		,	m_mask(1)
	{}

	int m_iter;
	int m_mask;
};

class OrbazTrueBlockEnumerator : public BitArrayCallback
{
public:
	OrbazTrueBlockEnumerator(OrbazTrueBlock*& trueBlocks)
		: m_trueBlocks(trueBlocks)
		, m_blockIndex(-1)
	{
	}

	void proc(int n)
	{
		int curMajorIndex = n & kMajorIndexMask;
		if (m_blockIndex  >= 0)
		{
			if (m_trueBlocks[m_blockIndex].GetMajorIndex() != curMajorIndex)
			{
				m_trueBlocks[++m_blockIndex].SetMajorIndex(curMajorIndex);
			}
		}
		else
		{
			m_blockIndex = 0;
			m_trueBlocks[0].SetMajorIndex(curMajorIndex);
		}
		m_trueBlocks[m_blockIndex].AddBlockIndex(n);
	}

	inline int NumBlocksSet(void)  { return m_blockIndex+1; }
	OrbazTrueBlockEnumerator& operator=(const OrbazTrueBlockEnumerator& rhs)
	{
		if (this != &rhs)
		{
			m_trueBlocks = rhs.m_trueBlocks;
			m_blockIndex = rhs.m_blockIndex;
		}
		return *this;
	}
private:
	OrbazTrueBlock*&	m_trueBlocks;
	int					m_blockIndex;
	static const DWORD	kMajorIndexMask = 0xFFFFFFE0;
};


