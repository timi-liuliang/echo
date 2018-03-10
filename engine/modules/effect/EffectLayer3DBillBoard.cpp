#include "EffectLayer3DBillBoard.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{

	EffectLayer3DBillBoard::EffectLayer3DBillBoard()
		: EffectLayer()
		, mWidth(DEFAULT_LY_WIDTH)
		, mHeight(DEFAULT_LY_HEIGHT)
		, mCenterOffestW(DEFAULT_LY_CENTEROFFSETW)
		, mCenterOffestH(DEFAULT_LY_CENTEROFFSETH)
		, mbStretch(false)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mType = ELT_3DBillboard;
		mAutoUV[0] = 0.f;
		mAutoUV[1] = 0.f;
		mAutoUV[2] = 0.f;
		mAutoUV[3] = 0.f;
	}

	EffectLayer3DBillBoard::~EffectLayer3DBillBoard()
	{

	}

	void EffectLayer3DBillBoard::prepare()
	{
		EffectLayer::prepare();

		float leftOff, rightOff, topOff, bottomOff; 

		leftOff = -mCenterOffestW;
		rightOff = 1.0f - mCenterOffestW;
		bottomOff = -mCenterOffestH;
		topOff = 1.0f - mCenterOffestH;

		leftOff = leftOff*mWidth;
		rightOff = rightOff*mWidth;
		topOff = topOff*mHeight;
		bottomOff = bottomOff*mHeight;

		mPosCached[0].x = leftOff;	mPosCached[0].y = bottomOff; mPosCached[0].z = 0;
		mPosCached[1].x = leftOff;	mPosCached[1].y = topOff;	 mPosCached[1].z = 0;
		mPosCached[2].x = rightOff; mPosCached[2].y = bottomOff; mPosCached[2].z = 0;
		mPosCached[3].x = rightOff; mPosCached[3].y = topOff;    mPosCached[3].z = 0;

		mAutoUV[0] = 0.f;
		mAutoUV[1] = 1.f;
		mAutoUV[2] = 0.f;
		mAutoUV[3] = 0.f;
	}

	void EffectLayer3DBillBoard::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		mVertexData.resize(4);

		Vector3 tempPos = position;
		color = color * mParentSystem->getSystemColor();
		ui32 colour = color.getABGR();
		float tl,tt,tr,tb;

		if(!mbStretch)
		{
			tl = mMaterial->getTexAddrLeft();
			tr = mMaterial->getTexAddrRight();
			tt = mMaterial->getTexAddrTop();
			tb = mMaterial->getTexAddrBottom();
			Quaternion rotSelf;
			if(Math::Abs(selfRotation) > 1.f)
			{
				Vector3 axis(0, 0, 1);
				rotSelf.fromAxisAngle(axis, selfRotation*Math::DEG2RAD);
			}
			rotSelf = rotSelf * oritation;

			Matrix4 mtxLocal;
			mtxLocal.makeScaling(scale, scale, scale);
			Matrix4 mtxRotate;
			mtxRotate.fromQuan(rotSelf);
			mtxLocal = mtxLocal * mtxRotate;
			mtxLocal.translate(tempPos);
			Matrix4 mtxWolrd = mParentSystem->getEffectSceneNode()->getWorldMatrix();
			mtxWolrd = mtxLocal*mtxWolrd;

			mVertexData[0].pos = mPosCached[0] * mtxWolrd;
			mVertexData[0].color = colour;
			mVertexData[1].pos = mPosCached[1] * mtxWolrd;
			mVertexData[1].color = colour;
			mVertexData[2].pos = mPosCached[2] * mtxWolrd;
			mVertexData[2].color = colour;
			mVertexData[3].pos = mPosCached[3] * mtxWolrd;
			mVertexData[3].color = colour;
		}
		else
		{
			tl = mAutoUV[0];
			tr = mAutoUV[1];
			tt = mAutoUV[3];
			tb = mAutoUV[2];

			mVertexData[0].pos = mPosCached[0];
			mVertexData[0].color = colour;
			mVertexData[1].pos = mPosCached[1];
			mVertexData[1].color = colour;
			mVertexData[2].pos = mPosCached[2];
			mVertexData[2].color = colour;
			mVertexData[3].pos = mPosCached[3];
			mVertexData[3].color = colour;
		}
			
		if(mMaterial->getSwapUV())
		{
			mVertexData[0].tx = tr; mVertexData[0].ty = tb;  //lb
			mVertexData[1].tx = tl; mVertexData[1].ty = tb;  //lt
			mVertexData[2].tx = tr; mVertexData[2].ty = tt;  //rb
			mVertexData[3].tx = tl; mVertexData[3].ty = tt;  //rt
		}
		else
		{
			mVertexData[0].tx = tl; mVertexData[0].ty = tb;
			mVertexData[1].tx = tl; mVertexData[1].ty = tt;
			mVertexData[2].tx = tr; mVertexData[2].ty = tb;
			mVertexData[3].tx = tr; mVertexData[3].ty = tt;
		}

		if (mMaterial->getUV2Enable())
		{
			tl = mMaterial->getTexAddrLeft1();
			tr = mMaterial->getTexAddrRight1();
			tt = mMaterial->getTexAddrTop1();
			tb = mMaterial->getTexAddrBottom1();

			if (mMaterial->getSwapUV1())
			{
				mVertexData[0].tx1 = tr; mVertexData[0].ty1 = tb;  //lb
				mVertexData[1].tx1 = tl; mVertexData[1].ty1 = tb;  //lt
				mVertexData[2].tx1 = tr; mVertexData[2].ty1 = tt;  //rb
				mVertexData[3].tx1 = tl; mVertexData[3].ty1 = tt;  //rt
			}
			else
			{
				mVertexData[0].tx1 = tl; mVertexData[0].ty1 = tb;
				mVertexData[1].tx1 = tl; mVertexData[1].ty1 = tt;
				mVertexData[2].tx1 = tr; mVertexData[2].ty1 = tb;
				mVertexData[3].tx1 = tr; mVertexData[3].ty1 = tt;
			}
		}
	}

	void EffectLayer3DBillBoard::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		if (m_vertexDataMainThread.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		
		renderables.push_back(pRenderable);

		EffectVertexFmt* pVetLockPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		if (pRenderable->getLockPtr(4, 6, vertBegin, pVetLockPtr, pIdxLockPtr))
		{
			for (int i = 0; i < 4; i++)
			{
				pVetLockPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVetLockPtr[i].color = m_vertexDataMainThread[i].color;
				pVetLockPtr[i].tx = m_vertexDataMainThread[i].tx;
				pVetLockPtr[i].ty = m_vertexDataMainThread[i].ty;
				pVetLockPtr[i].tx1 = m_vertexDataMainThread[i].tx1;
				pVetLockPtr[i].ty1 = m_vertexDataMainThread[i].ty1;
			}

			pIdxLockPtr[0] = vertBegin;
			pIdxLockPtr[1] = vertBegin + 1;
			pIdxLockPtr[2] = vertBegin + 2;

			pIdxLockPtr[3] = vertBegin + 2;
			pIdxLockPtr[4] = vertBegin + 1;
			pIdxLockPtr[5] = vertBegin + 3;
		}
	}

	// 属性拷贝
	void EffectLayer3DBillBoard::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayer3DBillBoard* layerBillboard = static_cast<EffectLayer3DBillBoard*>(particle);
		layerBillboard->mWidth = mWidth;
		layerBillboard->mHeight = mHeight;
		layerBillboard->mCenterOffestW = mCenterOffestW;
		layerBillboard->mCenterOffestH = mCenterOffestH;
	}

	void EffectLayer3DBillBoard::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mWidth, sizeof(float));
		pStream->write(&mHeight, sizeof(float));
		pStream->write(&mCenterOffestW, sizeof(float));
		pStream->write(&mCenterOffestH, sizeof(float));
	}

	void EffectLayer3DBillBoard::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mWidth, sizeof(float));
		pStream->read(&mHeight, sizeof(float));
		pStream->read(&mCenterOffestW, sizeof(float));
		pStream->read(&mCenterOffestH, sizeof(float));
	}

	void EffectLayer3DBillBoard::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		float fwidth = mWidth*scale;
		float fheight = mHeight*scale;
		float adjust  = Math::Max(fwidth/2.f, fheight/2.f);
		Vector3  vecAdjust(adjust, adjust, adjust);
		aabb.addPoint(position + vecAdjust);
		aabb.addPoint(position - vecAdjust);
	}

	void EffectLayer3DBillBoard::getPropertyList(PropertyList & list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_BB_WIDTH]);
		list.push_back(token[TOKEN_LY_BB_HEIGHT]);
		list.push_back(token[TOKEN_LY_BB_CENTEROFFSETW]);
		list.push_back(token[TOKEN_LY_BB_CENTEROFFSETH]);
	}

	bool EffectLayer3DBillBoard::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;
		if (name == token[TOKEN_LY_BB_WIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_BB_HEIGHT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_BB_CENTEROFFSETW])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_BB_CENTEROFFSETH])
		{
			type = IElement::PT_REAL; return true;
		}
		return false;
	}

	bool EffectLayer3DBillBoard::getPropertyValue(const String & name, String &value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_BB_WIDTH])
		{
			value = StringUtil::ToString(mWidth); return true;
		}
		else if (name == token[TOKEN_LY_BB_HEIGHT])
		{
			value = StringUtil::ToString(mHeight); return true;
		}
		else if (name == token[TOKEN_LY_BB_CENTEROFFSETW])
		{
			value = StringUtil::ToString(mCenterOffestW); return true;
		}
		else if (name == token[TOKEN_LY_BB_CENTEROFFSETH])
		{
			value = StringUtil::ToString(mCenterOffestH); return true;
		}
		return false;
	}

	bool EffectLayer3DBillBoard::setPropertyValue(const String & name,const String & value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_BB_WIDTH])
		{
			mWidth = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_LY_BB_HEIGHT])
		{
			mHeight = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_LY_BB_CENTEROFFSETW])
		{
			mCenterOffestW = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_LY_BB_CENTEROFFSETH])
		{
			mCenterOffestH = StringUtil::ParseReal(value); return true;
		}
		return false;
	}

	void EffectLayer3DBillBoard::_notifyExtenalParam( const Vector3& p1, const Vector3& p2 )
	{
		mStartPoint = p1;
		mEndPoint	= p2;

		// 设置起点终点，就开启了拉伸
		mbStretch = true;

		if(mStartPoint == mEndPoint)
		{
			mEndPoint.x += 0.01f;
		}

		// 重新计算 mPosCached
		Vector3 _len = mEndPoint - mStartPoint;
		Vector3 _dir = _len;
		_dir.normalize();

		Vector3 _right = _dir.cross(Vector3::UNIT_Y);
		_right.normalize();

		mPosCached[0] = - _right * mWidth * 0.5f + mStartPoint;
		mPosCached[1] =   mPosCached[0] + _len;
		mPosCached[2] =   _right * mWidth * 0.5f + mStartPoint;
		mPosCached[3] =   mPosCached[2] + _len;

		mAutoUV[2] = mAutoUV[3] + _len.len() / mHeight;
	}

	int EffectLayer3DBillBoard::_update(i32 time)
	{
		if (m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		int ret = EffectLayer::_update(time);

		if (mbStretch)
		{
			float USpeed = mMaterial->getUSpeed();
			if (USpeed != 0)
			{
				/*float transu = mUSpped*(float)(mTimer)/1000.f;
				mTexAddrLeftRealtime = mTexAddrLeft+transu;
				mTexAddrRightRealtime = mTexAddrRight+transu;*/
				float transu = USpeed * (float)(time)* 0.001f;
				mAutoUV[0] += transu;
				mAutoUV[1] += transu;
			}

			float VSpeed = mMaterial->getVSpeed();
			if (VSpeed != 0)
			{
				//VSpeed = -VSpeed;
				/*float transv = mVSpeed*(float)(mTimer)/1000.f;
				mTexAddrTopRealtime = mTexAddrTop + transv;
				mTexAddrBottomRealtime = mTexAddrBottom + transv;*/
				float transv = VSpeed * (float)(time)* 0.001f;
				mAutoUV[2] += transv;
				mAutoUV[3] += transv;
			}
		}

		return ret;
	}

}