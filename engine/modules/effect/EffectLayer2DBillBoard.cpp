#include "EffectLayer2DBillBoard.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	EffectLayer2DBillBoard::EffectLayer2DBillBoard()
		: EffectLayer()
		, mWidth(DEFAULT_LY_WIDTH)
		, mHeight(DEFAULT_LY_HEIGHT)
		, mCenterOffestW(DEFAULT_LY_CENTEROFFSETW)
		, mCenterOffestH(DEFAULT_LY_CENTEROFFSETH)
		, mOffsetViewport(DEFAULT_LY_OFFSETVIEWPORT)
		, mbPerpendicular(false)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mType = ELT_2DBillboard;
	}

	EffectLayer2DBillBoard::~EffectLayer2DBillBoard()
	{

	}

	void EffectLayer2DBillBoard::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		mVertexData.resize(4);

		float leftOff, rightOff, topOff, bottomOff; 
		Vector3 camX, camY;				
		Vector3 camDir;					

		leftOff = -mCenterOffestW;
		rightOff = 1.0f - mCenterOffestW;
		topOff = -mCenterOffestH;
		bottomOff = 1.0f - mCenterOffestH;

		Vector3 tmpBbPos = position;
		Vector3 vOwnOffset[4];
		Vector3 vLeffectOff, vRightOff, vTopOff, vBottomOff;

		camDir = (tmpBbPos - mParentSystem->getCameraPosInEffect()).normalizedCopy();
		if (mParentSystem->getIsUIEffect())
		{
			camDir = mParentSystem->getCurrentCamera()->getDirection();
		}
		if(mbPerpendicular)
		{
			camY = direction;
			camX = camDir.cross(camY);
			camX.normalize();
		}
		else
		{
			camX = mParentSystem->getCameraOriInEffect() * Vector3::UNIT_X;
			camY = mParentSystem->getCameraOriInEffect() * Vector3::UNIT_Y;
		}

		if(mOffsetViewport > 0.0f)
			tmpBbPos -= camDir*(mOffsetViewport);

		Quaternion rotSelf(1,0,0,0);
		if(Math::Abs(selfRotation) > 1.f)
		{
			Vector3 axis = -camDir;
			rotSelf.fromAxisAngle(axis, selfRotation*Math::DEG2RAD);
		}
		camX = rotSelf * camX;
		camY = rotSelf * camY;

		vLeffectOff = camX * (leftOff*mWidth*scale);
		vRightOff = camX * (rightOff*mWidth*scale);
		vTopOff = camY * (bottomOff*mHeight*scale);
		vBottomOff = camY * (topOff*mHeight*scale);

		vOwnOffset[0] = (vLeffectOff  + vBottomOff) + tmpBbPos;
		vOwnOffset[1] = (vLeffectOff  + vTopOff) + tmpBbPos;
		vOwnOffset[2] = (vRightOff + vBottomOff) + tmpBbPos;
		vOwnOffset[3] = (vRightOff + vTopOff) + tmpBbPos;

		const Matrix4& mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();

		color = color * mParentSystem->getSystemColor();
		ui32 colour = color.getABGR();
		float tl,tt,tr,tb;
		tl = mMaterial->getTexAddrLeft();
		tr = mMaterial->getTexAddrRight();
		tt = mMaterial->getTexAddrTop();
		tb = mMaterial->getTexAddrBottom();

		mVertexData[0].pos = vOwnOffset[0] * mtxWorld;
		mVertexData[0].color = colour;
		mVertexData[1].pos = vOwnOffset[1] * mtxWorld;
		mVertexData[1].color = colour;
		mVertexData[2].pos = vOwnOffset[2] * mtxWorld;
		mVertexData[2].color = colour;
		mVertexData[3].pos = vOwnOffset[3] * mtxWorld;
		mVertexData[3].color = colour;
			
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

	//把计算结果同步到Renderable
	void EffectLayer2DBillBoard::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
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

	//-----------------------------------------------------------------------

	void EffectLayer2DBillBoard::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayer2DBillBoard* layerBillboard = static_cast<EffectLayer2DBillBoard*>(particle);
		layerBillboard->mWidth = mWidth;
		layerBillboard->mHeight = mHeight;
		layerBillboard->mCenterOffestW = mCenterOffestW;
		layerBillboard->mCenterOffestH = mCenterOffestH;
		layerBillboard->mOffsetViewport = mOffsetViewport;
		layerBillboard->mbPerpendicular = mbPerpendicular;
	}

	void EffectLayer2DBillBoard::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mWidth, sizeof(float));
		pStream->write(&mHeight, sizeof(float));
		pStream->write(&mCenterOffestW, sizeof(float));
		pStream->write(&mCenterOffestH, sizeof(float));
		pStream->write(&mOffsetViewport, sizeof(float));
		pStream->write(&mbPerpendicular, sizeof(bool));
	}

	void EffectLayer2DBillBoard::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mWidth, sizeof(float));
		pStream->read(&mHeight, sizeof(float));
		pStream->read(&mCenterOffestW, sizeof(float));
		pStream->read(&mCenterOffestH, sizeof(float));
		pStream->read(&mOffsetViewport, sizeof(float));
		pStream->read(&mbPerpendicular, sizeof(bool));
	}

	void EffectLayer2DBillBoard::mergeAligenBox(Box& aabb)
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

	void EffectLayer2DBillBoard::getPropertyList(PropertyList & list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_BB_WIDTH]);
		list.push_back(token[TOKEN_LY_BB_HEIGHT]);
		list.push_back(token[TOKEN_LY_BB_CENTEROFFSETW]);
		list.push_back(token[TOKEN_LY_BB_CENTEROFFSETH]);
		list.push_back(token[TOKEN_LY_BB_OFFSETVIEWPORT]);
		list.push_back(token[TOKEN_LY_BB_PERPENDICULAR]);
	}

	bool EffectLayer2DBillBoard::getPropertyType(const String & name, PropertyType &type)
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
		else if(name == token[TOKEN_LY_BB_OFFSETVIEWPORT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_BB_PERPENDICULAR])
		{
			type = IElement::PT_BOOL; return true;
		}
		return false;
	}

	bool EffectLayer2DBillBoard::getPropertyValue(const String & name, String &value)
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
		else if (name == token[TOKEN_LY_BB_OFFSETVIEWPORT])
		{
			value = StringUtil::ToString(mOffsetViewport); return true;
		}
		else if(name == token[TOKEN_LY_BB_PERPENDICULAR])
		{
			value = StringUtil::ToString(mbPerpendicular); return true;
		}
		return false;
	}

	bool EffectLayer2DBillBoard::setPropertyValue(const String & name,const String & value)
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
		else if (name == token[TOKEN_LY_BB_OFFSETVIEWPORT])
		{
			mOffsetViewport = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_BB_PERPENDICULAR])
		{
			mbPerpendicular = StringUtil::ParseBool(value); return true;
		}
		return false;
	}
}