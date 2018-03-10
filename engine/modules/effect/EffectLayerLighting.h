#ifndef __ECHO_EFFECT_LAYER_LIGHTING_H__
#define __ECHO_EFFECT_LAYER_LIGHTING_H__

#include "EffectLayer.h"
#include "EffectPerlinNoise.h"

namespace Echo
{

	class EffectLayerLighting : public EffectLayer
	{
	public:
		enum FilterType
		{
			FT_TRIANGLE = 0,
			FT_RECTANGLE,
			FT_SIN,
			FT_TRAPEZIA,
		};

	public:
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		EffectLayerLighting();
		virtual ~EffectLayerLighting();

		virtual int _update(i32 time);
		virtual void updateRenderData();
		
		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		virtual void prepare();
		virtual void unprepare();

		virtual void _notifyStart();
		virtual void _notifyExtenalParam(const Vector3& p1, const Vector3& p2);

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

	protected:
		inline Vector3 calcVertVec(const Vector3& vDir);
		void updatePath();
		void buildPath(const Matrix4& mat);
		inline float filter(int nFilterType, float fDist, float fValue);
		void calcSplineVertex(const int i, 
			const int nRenderCount,
			const int nBase, 
			float fRatio,
			const float fWidthStart,
			const float fWidthEnd,
			const float fWidthMid,
			float& fAlpha, 
			int& nCount,
			const Matrix4& matTran,
			const Vector3& vNormal, 
			const Vector3& vCamPos,
			Vector3& vLast, 
			Vector3& vLastViewUp,
			Vector3& vOutVert1,
			Vector3& vOutVert2,
			Real nodeScale);

	protected:
		Vector3	mStartPos;
		Vector3	mEndPos;
		ui32	mSegment;
		ui32	mLightNum;
		Real	mWaveLen;
		i32     mInterval;
		Real	mWidthStart;
		Real	mWidthEnd;
		Real	mWidthMid;
		Real	mAlphaStart;
		Real	mAlphaEnd;
		Real	mAlphaMid;
		bool	mUseNormal;
		Vector3	mNormal;
		Real	mAmplitude;
		Real	mDeviation;
		i32		mFilterType;
		bool	mExtenalParam;

		/*
		*	0 - portion
		*	1 - Rand Angle
		*	2 - Rand Amplitude
		*/
		float*	mpRandomData[3];
		Vector3	mStartCur;
		Vector3	mEndCur;
		Vector3	mPosProc[2];
		Vector3*	mSplineIntPos;
		Vector3*	mSplineIntDir;
		Vector3*	mSplineIntUp;
		Vector3*	mSplineRandPos;
		float	mCurvedLen;
		bool	mSplineAffected;
		Real	mInnerAmplitude;
		int		mVertPairNum;
		i32		mTimeSpan;

		EffectPerlinNoise1D mPerlinNoise;
	};

}

#endif