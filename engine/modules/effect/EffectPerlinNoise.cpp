#include "EffectPerlinNoise.h"

namespace Echo
{
	EffectPerlinNoiseBase::EffectPerlinNoiseBase()
	{
		m_bTurbulence = true;
		m_nActiveOctave = -1;
	}

	EffectPerlinNoiseBase::~EffectPerlinNoiseBase()
	{
	}

	ui32 EffectPerlinNoiseBase::RandInteger()
	{
		m_dwSeed = i32(((i64)16807 * m_dwSeed) % (i64)2147483647);

		return m_dwSeed;
	}

	Real EffectPerlinNoiseBase::RandFloat()
	{
		Real vValue = (i32(RandInteger() % 2001) - 1000) / 1000.0f;

		return vValue;
	}

	bool EffectPerlinNoiseBase::InitParams(Real vAmplitude, i32 nWaveLength, Real vPersistence, i32 nOctaveNum)
	{
		if( nOctaveNum > MAX_OCTAVE )
			return false;

		i32			i;

		m_vBaseAmplitude	= vAmplitude;
		m_nBaseWaveLength   = nWaveLength;
		m_vPersistence		= vPersistence;
		m_nOctaveNum		= nOctaveNum;

		if( m_vBaseAmplitude < 0.0f )
			m_vBaseAmplitude = -m_vBaseAmplitude;
		if( m_nBaseWaveLength < 0 )
			m_nBaseWaveLength = -m_nBaseWaveLength;
		if( m_vPersistence < 0.0f )
			m_vPersistence = -m_vPersistence;

		Real	vTotalAmplitude = 0.0f;
		Real	vThisAmplitude = 1.0f;
		i32		nThisWaveLen = nWaveLength;
		for(i=0; i<m_nOctaveNum; i++)
		{
			vTotalAmplitude += vThisAmplitude;
			m_vAmplitude[i]	= vThisAmplitude;
			m_nWaveLength[i]= nThisWaveLen;
			m_nStartPos[i]	= RandInteger() % 1023;

			vThisAmplitude *= vPersistence;
			nThisWaveLen /= 2;

			if( nThisWaveLen <= 0 )
			{
				m_nOctaveNum = i + 1;
				break;
			}
		}

		for(i=0; i<m_nOctaveNum; i++)
		{
			m_vAmplitude[i] = m_vAmplitude[i] / vTotalAmplitude * m_vBaseAmplitude;	
		}
		return true;
	}


	EffectPerlinNoise1D::EffectPerlinNoise1D() 
	{
		m_pValues = NULL;
		m_nBufferLen = 0;
	}

	EffectPerlinNoise1D::~EffectPerlinNoise1D()
	{
		Release();
	}

	void EffectPerlinNoise1D::GetRandValues(i32 n, Real* pvValues, i32 nNumValue)
	{
		// Clamp to buffer range;
		while(n < 0) n += m_nBufferLen;
		n = n % m_nBufferLen; // this is based on the n must be larger than 0

		if( nNumValue > 3 ) nNumValue = 3;

		for(i32 i=0; i<nNumValue; i++)
		{
			pvValues[i] = m_pValues[n].v[i];
		}
	}

	bool EffectPerlinNoise1D::Init(i32 nBufferLen, Real vAmplitude, i32 nWaveLength, Real vPersistence, i32 nOctaveNum, unsigned int dwRandSeed)
	{
		// First try to release old resource;
		Release();

		i32	i, k;

		m_dwSeed = dwRandSeed;

		// generate loop back smoothed random buffer
		if( nBufferLen <= 0 )
			return false;

		m_nBufferLen = nBufferLen;
		Real* pValues = (float *) EchoMalloc(sizeof(Real) * m_nBufferLen);
		if( NULL == pValues )
			return false;

		m_pValues = (NOISEVALUE *) EchoMalloc(sizeof(NOISEVALUE) * m_nBufferLen);
		if( NULL == m_pValues )
			return false;

		for(k=0; k<3; k++)
		{
			// First create random number buffer;
			for(i=0; i<m_nBufferLen; i++)
				pValues[i] = RandFloat();

			// Now smooth the random number buffer;
			for(i=0; i<m_nBufferLen; i++)
			{
				i32 nLast, nNext;
				nLast = i - 1;
				if( nLast < 0 ) nLast += m_nBufferLen;
				nNext = i + 1;
				if( nNext >= m_nBufferLen ) nNext -= m_nBufferLen;
				m_pValues[i].v[k] = 0.25f * pValues[nLast] + 0.5f * pValues[i] + 0.25f * pValues[nNext];
			}
		}

		EchoSafeFree(pValues);
		return InitParams(vAmplitude, nWaveLength, vPersistence, nOctaveNum);
	}

	bool EffectPerlinNoise1D::Release()
	{
		if( m_pValues )
		{
			EchoSafeFree(m_pValues);
			m_pValues = NULL;
		}

		return true;
	}

	void EffectPerlinNoise1D::GetValue(float x, Real* pvValue, int nNumValue)
	{
		i32		i, k;
		Real		v, s;
		i32		n1, n2;
		Real		value1[3], value2[3];
		Real		value[3];
		Real		vFinal[3];

		memset(vFinal, 0, sizeof(Real) * nNumValue);
		for(i=0; i<m_nOctaveNum; i++)
		{
			v = m_nStartPos[i] + x / m_nWaveLength[i];
			n1 = i32(v);
			s = v - n1;
			n2 = n1 + 1;

			GetRandValues(n1, value1, nNumValue);
			GetRandValues(n2, value2, nNumValue);

			if( m_bTurbulence )
			{
				for(k=0; k<nNumValue; k++)
				{
					value[k] = (Real)fabs(LERP(s, value1[k], value2[k]));
					vFinal[k] += m_vAmplitude[i] * value[k];
				}
			}
			else
			{
				for(k=0; k<nNumValue; k++)
				{
					value[k] = LERP(s, value1[k], value2[k]);
					vFinal[k] += m_vAmplitude[i] * value[k];
				}
			}
		}

		for(k=0; k<nNumValue; k++)
			pvValue[k] = vFinal[k];
	}
}