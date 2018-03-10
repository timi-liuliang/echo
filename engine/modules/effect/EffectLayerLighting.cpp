#include "EffectLayerLighting.h"
#include "EffectSystemManager.h"
#include "Engine/core/Geom/SimpleSpline.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	EffectLayerLighting::EffectLayerLighting()
		: EffectLayer()
		, mStartPos(0, 2.0f, 0)
		, mEndPos(0, 0, 0)
		, mSegment(10)
		, mLightNum(1)
		, mWaveLen(1.f)
		, mInterval(100)
		, mWidthStart(0.1f)
		, mWidthEnd(0.1f)
		, mWidthMid(0.8f)
		, mAlphaStart(0.1f)
		, mAlphaEnd(0.1f)
		, mAlphaMid(1.0f)
		, mUseNormal(false)
		, mNormal(0, 1, 0)
		, mAmplitude(0.2f)
		, mDeviation(0.25f)
		, mFilterType(1)			
		, mExtenalParam(false)
		, mSplineIntPos(NULL)
		, mSplineIntDir(NULL)
		, mSplineIntUp(NULL)
		, mSplineRandPos(NULL)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mpRandomData[0] = NULL;
		mpRandomData[1] = NULL;
		mpRandomData[2] = NULL;

		mType = ELT_Lighting;
	}

	EffectLayerLighting::~EffectLayerLighting()
	{

	}

	inline Vector3 EffectLayerLighting::calcVertVec(const Vector3& vDir)
	{
		Vector3 v = fabs(vDir.y) > .99f ? vDir.cross(Vector3::UNIT_X) 
			: Vector3::UNIT_Y.cross(vDir);
		v.normalize();
		return v;
	}

	void EffectLayerLighting::prepare()
	{
		EffectLayer::prepare();

		mPerlinNoise.Init(256, 2.0f, 1, mDeviation, 5, 2343);
		if(mSegment < 2) mSegment = 2;
		if(mLightNum < 1) mLightNum = 1;

		assert(mKeyFrames.size());
		EffectKeyFrame* pKeyFrame = mKeyFrames[0];
		SimpleSpline* pSpline = pKeyFrame->getSplinePtr();

		if(pSpline && pKeyFrame->getInterpolationType()==EffectKeyFrame::IT_SPLINE)
		{
			mSplineAffected = true;
			mCurvedLen = 0;
			Vector3 pos0, pos1;
			pos0 = pSpline->interpolate(0.0f);
			for(int i=0; i<=40; ++i)
			{
				pos1 = pSpline->interpolate(float(i)/40.0f);
				mCurvedLen += (pos1 - pos0).len();
				pos0 = pos1;
			}
			mLightNum = 1;
		}
		else
			mSplineAffected = false;

		mVertPairNum = mSegment * mLightNum + 1;
		for (int i = 0; i < sizeof(mpRandomData) / sizeof(float*); i++)
		{
            EchoSafeFree( mpRandomData[i] );
			mpRandomData[i] = (float*)EchoMalloc(sizeof(float)*mSegment*mLightNum); 
		}

		int nCount = 0;
		float fSegs = (float)mSegment;

		for(ui32 i = 0; i < mLightNum; i++)
		{
			for(ui32 j = 1; j < mSegment; j++)
			{
				if (i % 2 == 0) mpRandomData[0][nCount] = j / fSegs;
				else mpRandomData[0][nCount] = (mSegment - j) / fSegs;
				nCount++;
			}
		}

		if (mSplineAffected)
		{
			mSplineIntPos = (Vector3*)EchoMalloc(sizeof(Vector3)*(mSegment+1));
			mSplineIntDir = (Vector3*)EchoMalloc(sizeof(Vector3)*(mSegment+1));
			mSplineIntUp = (Vector3*)EchoMalloc(sizeof(Vector3)*(mSegment+1));
			mSplineRandPos = (Vector3*)EchoMalloc(sizeof(Vector3)*(mSegment));

			for (ui32 i = 0; i <= mSegment; i++)
			{
				mSplineIntPos[i] = pSpline->interpolate(float(i)/fSegs);

				if (i > 0)
				{
					mSplineIntDir[i-1] = (mSplineIntPos[i] - mSplineIntPos[i-1]);
					mSplineIntDir[i-1].normalize();
					mSplineIntUp[i-1] = calcVertVec(mSplineIntDir[i-1]);
				}
			}

			mSplineIntDir[mSegment] = mSplineIntDir[mSegment-1];
			mSplineIntUp[mSegment] = calcVertVec(mSplineIntDir[mSegment]);
		}
	}

	void EffectLayerLighting::unprepare()
	{
		EffectLayer::unprepare();

		EchoSafeFree(mSplineIntPos);
		EchoSafeFree(mSplineIntDir);
		EchoSafeFree(mSplineIntUp);
		EchoSafeFree(mSplineRandPos);

		for (int i = 0; i < sizeof(mpRandomData) / sizeof(float*); i++)
		{
			EchoSafeFree(mpRandomData[i]);
		}
	}

	int EffectLayerLighting::_update(i32 time)
	{
		if (m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		int ret = EffectLayer::_update(time);

		if(mbAffected)
		{
			mTimeSpan += time;
			if (mTimeSpan >= mInterval)
			{
				mTimeSpan -=  mInterval;
				updatePath();
			}
		}

		return ret;
	}

	void EffectLayerLighting::updatePath()
	{
		mInnerAmplitude = Math::IntervalRandom(0.0f, mAmplitude);

		Matrix4 transMatrix = Matrix4::IDENTITY;

		buildPath(transMatrix);
	}

	void EffectLayerLighting::buildPath(const Matrix4& mat)
	{
		float fDist;

		if (mSplineAffected)
			fDist = mCurvedLen;
		else
		{
			Vector3 vDir = mEndPos - mStartPos;
			fDist = vDir.normalizeLen();

			float randAngle1(Math::UnitRandom());
			randAngle1 *= Math::PI_2;
			float randAngle2(Math::UnitRandom());

			Quaternion randomOri;
			randomOri.fromAxisAngle(vDir, randAngle1);
			Vector3 vUp = randomOri * calcVertVec(vDir);
			randomOri.fromAxisAngle(vUp, randAngle2*mDeviation);
			Vector3 vNewDir = randomOri * vDir;

			mStartCur = mStartPos;
			mEndCur = mStartCur + vNewDir * fDist;
			Vector3 vStart(mStartCur * mat);
			Vector3 vEnd(mEndCur * mat);
			fDist = (vStart - vEnd).len();
		}

		float fWaves = fDist / mWaveLen;
		float fSeg = fWaves / mSegment;
		float fStart = (float)Math::IntervalRandom(0, 1022);
		int nCount = mSegment * mLightNum;
		float fRand[2];
		float fSegs = (float)mSegment;

		for (int i = 0; i < nCount; i++)
		{
			mPerlinNoise.GetValue(
				fStart + i * fSeg,
				fRand,
				2);

			mpRandomData[1][i] = fRand[0] * Math::PI_2;
			float fInv = i / fSegs;
			mpRandomData[2][i] = filter(mFilterType, fInv, fRand[1]);

			if (mSplineAffected)
			{
				Quaternion ori;
				ori.fromAxisAngle(mSplineIntDir[i+1], mpRandomData[1][i]);
				Vector3 vRandUp = ori * mSplineIntUp[i];
				mSplineRandPos[i] = mSplineIntPos[i+1] + vRandUp * (mpRandomData[2][i] * mInnerAmplitude);
			}
		}	
	}

	inline float EffectLayerLighting::filter(int nFilterType, float fDist, float fValue)
	{
		if (fDist < 0.f)
			return fValue;

		if (fDist > 1.f)
			fDist = (fDist - (int)fDist);

		switch(nFilterType)
		{
		case FT_TRIANGLE:
			return 2 * (fDist > 0.5 ? 1 - fDist : fDist);
		case FT_RECTANGLE:
			return fValue;
		case FT_TRAPEZIA:
			return fValue * (fDist < 0.2 ? fDist * 5 : fDist > 0.8 ? (1 - fDist) * 5 : 1);
		case FT_SIN:
			return fValue * Math::Sin(fDist * Math::PI_2);
		}
		return fValue;
	}


	void EffectLayerLighting::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		mVertexData.resize((mSegment + 1) * 2 * mLightNum);
		EffectVertexFmt* verticsPtr = &mVertexData[0];
		Matrix4 matTran = mParentSystem->getEffectSceneNode()->getWorldMatrix();
		if(!mSplineAffected)
		{
			if(mExtenalParam)
			{
				matTran = Matrix4::IDENTITY;
			}

			Matrix4 mtxLocal;
			mtxLocal.makeScaling(scale, scale, scale);

			Matrix4 mtxRot;
			Quaternion quanRot;
			quanRot.fromAxisAngle(originalDirection, selfRotation*Math::DEG2RAD);
			quanRot = quanRot*oritation;
			quanRot.toMat4(mtxRot);
			mtxLocal = mtxLocal * mtxRot;

			mtxLocal.translate(position);

			matTran = mtxLocal * matTran;
		}

		//Real nodeScale = mParentSystem->getDerivedScale().x;
		Real nodeScale = mParentSystem->getEffectSceneNode()->getWorldScaling().x;

		Vector3 vStart, vEnd, vDir, vUp;
		float fDist;

		if (!mSplineAffected)
		{
			vStart = mStartCur * matTran;
			vEnd =  mEndPos * matTran;
			vDir = vEnd - vStart;
			fDist = vDir.normalizeLen();
			vUp = calcVertVec(vDir);
		}

		Vector3 vViewUp, vLastViewUp, vLast(0.f,0.f,0.f);
		Vector3 vCamPos = mParentSystem->getCurrentCamera()->getPosition();

		float fWidthStart = mWidthStart * scale;
		float fWidthEnd = mWidthEnd * scale;
		float fWidthMid = mWidthMid * scale;
		float fAmplitude = mAmplitude;

		int nStep = mSegment * 2;
		int nCount = 0;
		float fSegs = (float)mSegment;

		color = color * mParentSystem->getSystemColor();
		Color lightningColor = color;

		Vector3 vNormal;

		if (mUseNormal)
		{
			vNormal = mParentSystem->getEffectSceneNode()->getWorldOrientation() * mNormal;
		}

		int nRenderCount = mVertPairNum;
		float fInvRenderCountMinus1 = 1.0f / float(nRenderCount - 1);
		float tl = mMaterial->getTexAddrLeft();
		float tr = mMaterial->getTexAddrRight();
		float tt = mMaterial->getTexAddrTop();
		float tb = mMaterial->getTexAddrBottom();
		bool swapUV = mMaterial->getSwapUV();
		float tl1 = mMaterial->getTexAddrLeft1();
		float tr1 = mMaterial->getTexAddrRight1();
		float tt1 = mMaterial->getTexAddrTop1();
		float tb1 = mMaterial->getTexAddrBottom1();
		bool swapUV1 = mMaterial->getSwapUV1();

		for (int i = 0; i < nRenderCount; i++)
		{
			int nBase = i * 2;
			float fRatio = i * fInvRenderCountMinus1;
			Vector3 vVert[2];
			float fAlpha = 1.0f;
			float tv = 0.0f;
			int nMod = i % nStep;
			int nRemain = i / mSegment;
			int nModSeg = i % mSegment;

			tv = i*fInvRenderCountMinus1;

			if (mSplineAffected)
			{
				calcSplineVertex(
					i, 
					nRenderCount, 
					nBase, 
					fRatio, 
					fWidthStart, 
					fWidthEnd, 
					fWidthMid,
					fAlpha, 
					nCount, 
					matTran, 
					vNormal, 
					vCamPos, 
					vLast, 
					vLastViewUp, 
					vVert[0],
					vVert[1],
					nodeScale);
			}
			else
			{
				if (nMod == 0)
				{
					Vector3 vView(vStart - vCamPos);
					vViewUp = vDir.cross(vView);
					vViewUp.normalize();
					vViewUp *= fWidthStart;

					vVert[0] = (vStart - vViewUp*nodeScale);
					vVert[1] = (vStart + vViewUp*nodeScale);

					fAlpha = mAlphaStart;
				}
				else if (nMod == mSegment)
				{
					Vector3 vView = vEnd - vCamPos;
					vViewUp = vDir.cross(vView);
					vViewUp.normalize();
					vViewUp *= fWidthEnd;

					vVert[0] = (vEnd - vViewUp*nodeScale);
					vVert[1] = (vEnd + vViewUp*nodeScale);

					fAlpha = mAlphaEnd;
				}
				else
				{
					Quaternion q;
					q.fromAxisAngle(vDir, mpRandomData[1][nCount]);
					Vector3 vRandUp = q * vUp;
					Vector3 vPos(vStart + vDir * (mpRandomData[0][nCount] * fDist) + vRandUp * (mpRandomData[2][nCount] * fAmplitude));
					Vector3  vView(vPos - vCamPos);
					vViewUp = vDir.cross(vView);
					vViewUp.normalize();
					fRatio = ((nRemain % 2) ? (mSegment - nModSeg) : nModSeg) / fSegs;

					if (fRatio < .5f)
					{
						fRatio *= 2.0f;
						vViewUp *= fWidthStart * (1.f - fRatio) + fWidthMid * fRatio;
						fAlpha = mAlphaStart * (1.f - fRatio) + mAlphaMid * fRatio;
					}
					else
					{
						fRatio = (fRatio - .5f) * 2.0f;
						vViewUp *= fWidthMid * (1.f - fRatio) + fWidthEnd * fRatio;
						fAlpha = mAlphaMid * (1.f - fRatio) + mAlphaEnd * fRatio;
					}

					vVert[0] = (vPos - vViewUp*nodeScale);
					vVert[1] = (vPos + vViewUp*nodeScale);

					nCount++;
				}
			}
			ui32 dcolor = (lightningColor*fAlpha).getABGR();
			verticsPtr[0].pos = vVert[0];
			verticsPtr[0].color = dcolor;
			verticsPtr[1].pos = vVert[1];
			verticsPtr[1].color = dcolor;
			if(swapUV)
			{
				tv = tl + tv*(tr-tl);
				verticsPtr[0].tx = tv;
				verticsPtr[0].ty = tb;
				verticsPtr[1].tx = tv;
				verticsPtr[1].ty = tt;
			}
			else
			{
				tv = tt + tv*(tb-tt);
				verticsPtr[0].tx = tl;
				verticsPtr[0].ty = tv;
				verticsPtr[1].tx = tr;
				verticsPtr[1].ty = tv;
			}

			if (mMaterial->getUV2Enable())
			{
				if (swapUV1)
				{
					tv = tl1 + tv*(tr1 - tl1);
					verticsPtr[0].tx1 = tv;
					verticsPtr[0].ty1 = tb1;
					verticsPtr[1].tx1 = tv;
					verticsPtr[1].ty1 = tt1;
				}
				else
				{
					tv = tt1 + tv*(tb1 - tt1);
					verticsPtr[0].tx1 = tl1;
					verticsPtr[0].ty1 = tv;
					verticsPtr[1].tx1 = tr1;
					verticsPtr[1].ty1 = tv;
				}
			}

			verticsPtr += 2;
		}
	}

	void EffectLayerLighting::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		if (m_vertexDataMainThread.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		
		renderables.push_back(pRenderable);

		EffectVertexFmt* pVertexPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		if (pRenderable->getLockPtr((mSegment + 1) * 2 * mLightNum, mSegment * 6 * mLightNum, vertBegin, pVertexPtr, pIdxLockPtr))
		{

			for (ui32 i = 0; i < (mSegment + 1) * 2 * mLightNum; i++)
			{
				pVertexPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVertexPtr[i].color = m_vertexDataMainThread[i].color;
				pVertexPtr[i].tx = m_vertexDataMainThread[i].tx;
				pVertexPtr[i].ty = m_vertexDataMainThread[i].ty;
				pVertexPtr[i].tx1 = m_vertexDataMainThread[i].tx1;
				pVertexPtr[i].ty1 = m_vertexDataMainThread[i].ty1;
			}

			for (i32 i = 0; i < mVertPairNum - 1; ++i)
			{
				*pIdxLockPtr++ = vertBegin + 2;
				*pIdxLockPtr++ = vertBegin;
				*pIdxLockPtr++ = vertBegin + 1;

				*pIdxLockPtr++ = vertBegin + 3;
				*pIdxLockPtr++ = vertBegin + 2;
				*pIdxLockPtr++ = vertBegin + 1;

				vertBegin += 2;
			}
		}
	}

	void EffectLayerLighting::calcSplineVertex(const int i, 
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
		Real nodeScale)
	{
		if (i == 0)
		{
			Vector3 vIntStart(mSplineIntPos[0] * matTran);
			Vector3 vDir = oritation * mSplineIntDir[0];
			//vIntStart *= mSystemScale;
			vLast = vIntStart;

			Vector3 vViewUp = vDir.cross(mUseNormal ? vNormal : vIntStart - vCamPos);
			vViewUp.normalize();
			vLastViewUp = vViewUp;
			vViewUp *= fWidthStart;

			vOutVert1 = vIntStart - vViewUp*nodeScale;
			vOutVert2 = vIntStart + vViewUp*nodeScale;
			fAlpha = mAlphaStart;
		}
		else if (i + 1 == nRenderCount)
		{
			Vector3 vIntEnd(mSplineIntPos[i] * matTran);
			Vector3 vDir(vIntEnd - vLast);
			Vector3 vViewUp = vDir.cross(mUseNormal ? vNormal : vIntEnd - vCamPos);
			//vIntEnd *= mSystemScale;
			vViewUp.normalize();

			if (vViewUp.dot(vLastViewUp) < 0)
				vViewUp *= -1;

			vViewUp *= fWidthEnd;
			vOutVert1 = vIntEnd - vViewUp*nodeScale;
			vOutVert2 = vIntEnd + vViewUp*nodeScale;
			fAlpha = mAlphaEnd;
		}
		else
		{
			Vector3 vPos(mSplineRandPos[nCount] * matTran);
			Vector3 vDir(vPos - vLast);
			//vPos *= mSystemScale;
			vLast = vPos;

			Vector3 vViewUp = vDir.cross(mUseNormal ? vNormal : vPos - vCamPos);
			vViewUp.normalize();

			if (vViewUp.dot(vLastViewUp) < 0)
				vViewUp *= -1;
			vLastViewUp = vViewUp;

			if (fRatio < .5f)
			{
				fRatio *= 2.0f;
				vViewUp *= fWidthStart * (1.f - fRatio) + fWidthMid * fRatio;
				fAlpha = mAlphaStart * (1.f - fRatio) + mAlphaMid * fRatio;
			}
			else
			{
				fRatio = (fRatio - .5f) * 2.0f;
				vViewUp *= fWidthMid * (1.f - fRatio) + fWidthEnd * fRatio;
				fAlpha = mAlphaMid * (1.f - fRatio) + mAlphaEnd * fRatio;
			}

			vOutVert1 = vPos - vViewUp*nodeScale;
			vOutVert2 = vPos + vViewUp*nodeScale;
			nCount++;
		}
	}

	void EffectLayerLighting::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mStartPos, sizeof(Vector3));
		pStream->read(&mEndPos, sizeof(Vector3));
		pStream->read(&mSegment, sizeof(ui32));
		pStream->read(&mLightNum, sizeof(ui32));
		pStream->read(&mWaveLen, sizeof(Real));
		pStream->read(&mInterval, sizeof(i32));
		pStream->read(&mWidthStart, sizeof(Real));
		pStream->read(&mWidthEnd, sizeof(Real));
		pStream->read(&mWidthMid, sizeof(Real));
		pStream->read(&mAlphaStart, sizeof(Real));
		pStream->read(&mAlphaEnd, sizeof(Real));
		pStream->read(&mAlphaMid, sizeof(Real));
		pStream->read(&mUseNormal, sizeof(bool));
		pStream->read(&mNormal, sizeof(Vector3));
		pStream->read(&mAmplitude, sizeof(Real));
		pStream->read(&mDeviation, sizeof(Real));
		pStream->read(&mFilterType, sizeof(i32));
		pStream->read(&mExtenalParam, sizeof(bool));
	}

	void EffectLayerLighting::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mStartPos, sizeof(Vector3));
		pStream->write(&mEndPos, sizeof(Vector3));
		pStream->write(&mSegment, sizeof(ui32));
		pStream->write(&mLightNum, sizeof(ui32));
		pStream->write(&mWaveLen, sizeof(Real));
		pStream->write(&mInterval, sizeof(i32));
		pStream->write(&mWidthStart, sizeof(Real));
		pStream->write(&mWidthEnd, sizeof(Real));
		pStream->write(&mWidthMid, sizeof(Real));
		pStream->write(&mAlphaStart, sizeof(Real));
		pStream->write(&mAlphaEnd, sizeof(Real));
		pStream->write(&mAlphaMid, sizeof(Real));
		pStream->write(&mUseNormal, sizeof(bool));
		pStream->write(&mNormal, sizeof(Vector3));
		pStream->write(&mAmplitude, sizeof(Real));
		pStream->write(&mDeviation, sizeof(Real));
		pStream->write(&mFilterType, sizeof(i32));
		pStream->write(&mExtenalParam, sizeof(bool));
	}

	void EffectLayerLighting::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		Vector3 startNoise =Vector3(mWidthStart, mWidthStart, mWidthStart);
		Vector3 endNoise = Vector3(mWidthEnd, mWidthEnd, mWidthEnd);
		Vector3 midNoise = Vector3(mWidthMid, mWidthMid, mWidthMid);

		if(mSplineAffected)
		{
			for(ui32 i=0; i<mSegment+1; ++i)
			{
				if(i==0)
				{
					aabb.addPoint(mSplineIntPos[i]+startNoise);
					aabb.addPoint(mSplineIntPos[i]-startNoise);
				}
				else if( i==mSegment)
				{
					aabb.addPoint(mSplineIntPos[i]+endNoise);
					aabb.addPoint(mSplineIntPos[i]-endNoise);
				}
				else if(i == mSegment/2)
				{
					aabb.addPoint(mSplineIntPos[i]+midNoise);
					aabb.addPoint(mSplineIntPos[i]-midNoise);
				}
				else
				{
					aabb.addPoint(mSplineIntPos[i]);
				}
			}
		}
		else
		{
			Vector3 midPos = (mStartPos+mEndPos)/2.0f;
			aabb.addPoint(mStartPos);
			aabb.addPoint(mEndPos);
			aabb.addPoint(mStartPos + startNoise);
			aabb.addPoint(mStartPos - startNoise);
			aabb.addPoint(mEndPos + endNoise);
			aabb.addPoint(mEndPos - endNoise);
			aabb.addPoint(midPos + midNoise);
			aabb.addPoint(midPos - midNoise);
		}	
	}

	void EffectLayerLighting::copyAttributesTo(EffectParticle* layer)
	{
		EffectLayer::copyAttributesTo(layer);
		EffectLayerLighting* lightning = static_cast<EffectLayerLighting*>(layer);
		lightning->mStartPos = mStartPos;
		lightning->mEndPos = mEndPos;
		lightning->mSegment = mSegment;
		lightning->mLightNum = mLightNum;
		lightning->mWaveLen = mWaveLen;
		lightning->mInterval = mInterval;
		lightning->mWidthStart = mWidthStart;
		lightning->mWidthEnd = mWidthEnd;
		lightning->mWidthMid = mWidthMid;
		lightning->mAlphaStart = mAlphaStart;
		lightning->mAlphaEnd = mAlphaEnd;
		lightning->mAlphaMid = mAlphaMid;
		lightning->mUseNormal = mUseNormal;
		lightning->mNormal = mNormal;
		lightning->mAmplitude = mAmplitude;
		lightning->mDeviation = mDeviation;
		lightning->mFilterType = mFilterType;
		lightning->mExtenalParam = mExtenalParam;
	}

	void EffectLayerLighting::_notifyStart()
	{
		EffectLayer::_notifyStart();
		mTimeSpan = mInterval;
	}

	void EffectLayerLighting::_notifyExtenalParam(const Vector3& p1, const Vector3& p2)
	{
		if(mExtenalParam)
		{
			mStartPos = p1;
			mEndPos = p2;
		}
	}

	void EffectLayerLighting::getPropertyList(PropertyList& list)
	{
		EffectLayer::getPropertyList(list);

		list.push_back(token[TOKEN_LY_LN_BEGINPOS]);
		list.push_back(token[TOKEN_LY_LN_ENDPOS]);
		list.push_back(token[TOKEN_LY_LN_SEGMENT]);
		list.push_back(token[TOKEN_LY_LN_NUM]);
		list.push_back(token[TOKEN_LY_LN_WAVELEN]);
		list.push_back(token[TOKEN_LY_LN_TIMESPIN]);
		list.push_back(token[TOKEN_LY_LN_STARTWIDTH]);
		list.push_back(token[TOKEN_LY_LN_ENDWIDTH]);
		list.push_back(token[TOKEN_LY_LN_MIDDERWIDTH]);
		list.push_back(token[TOKEN_LY_LN_STARTALPHA]);
		list.push_back(token[TOKEN_LY_LN_ENDALPHA]);
		list.push_back(token[TOKEN_LY_LN_MIDDERALPHA]);
		list.push_back(token[TOKEN_LY_LN_USENORMAL]);
		list.push_back(token[TOKEN_LY_LN_NORMAL]);
		list.push_back(token[TOKEN_LY_LN_CURVEDLEN]);
		list.push_back(token[TOKEN_LY_LN_DEVIATION]);
		list.push_back(token[TOKEN_LY_LN_FILTER]);
		list.push_back(token[TOKEN_LY_LN_EXTENALPARAM]);
	}

	bool EffectLayerLighting::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;
		if (name == token[TOKEN_LY_LN_BEGINPOS])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDPOS])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_LY_LN_SEGMENT])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_LN_NUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_LN_WAVELEN])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_TIMESPIN])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTWIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDWIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERWIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTALPHA])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDALPHA])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERALPHA])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_USENORMAL])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_LN_NORMAL])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_LY_LN_CURVEDLEN])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_DEVIATION])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_LN_FILTER])
		{
			type = IElement::PT_LIGHTNING_FILTER_MODE; return true;
		}
		else if(name == token[TOKEN_LY_LN_EXTENALPARAM])
		{
			type = IElement::PT_BOOL; return true;
		}
		return false;
	}

	bool EffectLayerLighting::getPropertyValue(const String& name, String&value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_LN_BEGINPOS])
		{
			value = StringUtil::ToString(mStartPos); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDPOS])
		{
			value = StringUtil::ToString(mEndPos); return true;
		}
		else if(name == token[TOKEN_LY_LN_SEGMENT])
		{
			value = StringUtil::ToString(mSegment); return true;
		}
		else if(name == token[TOKEN_LY_LN_NUM])
		{
			value = StringUtil::ToString(mLightNum); return true;
		}
		else if(name == token[TOKEN_LY_LN_WAVELEN])
		{
			value = StringUtil::ToString(mWaveLen); return true;
		}
		else if(name == token[TOKEN_LY_LN_TIMESPIN])
		{
			value = StringUtil::ToString(mInterval); return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTWIDTH])
		{
			value = StringUtil::ToString(mWidthStart); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDWIDTH])
		{
			value = StringUtil::ToString(mWidthEnd); return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERWIDTH])
		{
			value = StringUtil::ToString(mWidthMid); return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTALPHA])
		{
			value = StringUtil::ToString(mAlphaStart); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDALPHA])
		{
			value = StringUtil::ToString(mAlphaEnd); return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERALPHA])
		{
			value = StringUtil::ToString(mAlphaMid); return true;
		}
		else if(name == token[TOKEN_LY_LN_USENORMAL])
		{
			value = StringUtil::ToString(mUseNormal); return true;
		}
		else if(name == token[TOKEN_LY_LN_NORMAL])
		{
			value = StringUtil::ToString(mNormal); return true;
		}
		else if(name == token[TOKEN_LY_LN_CURVEDLEN])
		{
			value = StringUtil::ToString(mAmplitude); return true;
		}
		else if(name == token[TOKEN_LY_LN_DEVIATION])
		{
			value = StringUtil::ToString(mDeviation); return true;
		}
		else if(name == token[TOKEN_LY_LN_FILTER])
		{
			value = StringUtil::ToString(mFilterType); return true;
		}
		else if(name == token[TOKEN_LY_LN_EXTENALPARAM])
		{
			value = StringUtil::ToString(mExtenalParam); return true;
		}
		return false;
	}

	bool EffectLayerLighting::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_LN_BEGINPOS])
		{
			mStartPos = StringUtil::ParseVec3(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDPOS])
		{
			mEndPos = StringUtil::ParseVec3(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_SEGMENT])
		{
			mSegment = StringUtil::ParseI32(value); 
			if(mSegment < 2) mSegment = 2; 
			return true;
		}
		else if(name == token[TOKEN_LY_LN_NUM])
		{
			mLightNum = StringUtil::ParseI32(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_WAVELEN])
		{
			mWaveLen = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_TIMESPIN])
		{
			mInterval = StringUtil::ParseI32(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTWIDTH])
		{
			mWidthStart = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDWIDTH])
		{
			mWidthEnd = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERWIDTH])
		{
			mWidthMid = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_STARTALPHA])
		{
			mAlphaStart = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_ENDALPHA])
		{
			mAlphaEnd = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_MIDDERALPHA])
		{
			mAlphaMid = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_USENORMAL])
		{
			mUseNormal = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_NORMAL])
		{
			mNormal = StringUtil::ParseVec3(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_CURVEDLEN])
		{
			mAmplitude = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_DEVIATION])
		{
			mDeviation = StringUtil::ParseFloat(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_FILTER])
		{
			mFilterType = StringUtil::ParseI32(value); return true;
		}
		else if(name == token[TOKEN_LY_LN_EXTENALPARAM])
		{
			mExtenalParam = StringUtil::ParseBool(value); return true;
		}

		return false;
	}
}