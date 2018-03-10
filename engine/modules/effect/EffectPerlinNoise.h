#ifndef __EFFECT_PERLIN_H__
#define __EFFECT_PERLIN_H__

#include <engine/core/Memory/MemManager.h>

namespace Echo
{
	class EffectPerlinNoiseBase
	{
	protected:

		enum
		{
			MAX_OCTAVE = 16,
		};

	protected:
		typedef struct _NOISEVALUE
		{
			float	v[3];
		} NOISEVALUE;

		bool	m_bTurbulence;		//flag indicates whether use sum of fractal fabs value or not
		ui32	m_dwSeed;
		i32  	m_nBaseWaveLength;
		Real	m_vBaseAmplitude;
		Real	m_vPersistence;

		i32		m_nActiveOctave;
		i32		m_nOctaveNum;
		i32		m_nStartPos[MAX_OCTAVE];
		i32		m_nWaveLength[MAX_OCTAVE];
		Real	m_vAmplitude[MAX_OCTAVE];

	protected:

		Real	LERP(Real s, Real a, Real b) { return a * (1.0f - s) + b * s; }
		Real	S_CURVE(Real t) { return t * t * (3.0f - 2.0f * t); }

		ui32	RandInteger();
		Real	RandFloat();

		EffectPerlinNoiseBase();
		~EffectPerlinNoiseBase();

		// use 2.0 1 0.25 5 to initialize
		bool InitParams(Real vAmplitude, i32 nWaveLength, Real vPersistence, i32 nOctaveNum);

	public:
		inline void SetTurbulence(bool bTurbulence) { m_bTurbulence = bTurbulence; }
		inline bool GetTurbulence() { return m_bTurbulence; }
		inline void SetActiveOctave(i32 nIndex)		{ m_nActiveOctave = nIndex; }
		inline Real GetBaseAmplitude()				{ return m_vBaseAmplitude; }
	};


	class EffectPerlinNoise1D : public EffectPerlinNoiseBase
	{
	private:
		i32				m_nBufferLen;	
		NOISEVALUE		*m_pValues;		// the spool buffer containing rand values, maximum contains 3 number value;

	protected:
		inline void GetRandValues(i32 n, Real* pvValues, i32 nNumValue);

	public:
		EffectPerlinNoise1D();
		~EffectPerlinNoise1D();

		bool Init(int nBufferLen, Real vAmplitude, i32 nWaveLength, Real vPersistence, i32 nOctaveNum, ui32 dwRandSeed);
		bool Release();

		void GetValue(Real x, Real * pvValue, i32 nNumValue);
	};
}

#endif