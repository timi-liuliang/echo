#include "EffectLayerSpriteAnimation.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{

	EffectLayerSpriteAnimation::EffectLayerSpriteAnimation()
		: EffectLayer()
		, muSegment(4)
		, mvSegment(4)
		, mWidth(1)
		, mHeight(1)
		, mSpeedTime(50)
		, mInnerTime(0)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mType = ELT_SpriteAnimation;
	}

	EffectLayerSpriteAnimation::~EffectLayerSpriteAnimation()
	{

	}

	void EffectLayerSpriteAnimation::_notifyAffected()
	{
		mInnerTime = 0;
	}

	void EffectLayerSpriteAnimation::unprepare()
	{
		EffectLayer::unprepare();
	}

	int EffectLayerSpriteAnimation::_update(i32 time)
	{
		if (m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		int ret = EffectLayer::_update(time);
		mInnerTime += time;
		return ret;
	}

	void EffectLayerSpriteAnimation::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		mVertexData.resize(4);

		color = color * mParentSystem->getSystemColor();
		ui32 colour = color.getABGR();
		Matrix4 mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();

		Matrix4 mtxLocal;
		Vector3 vScale(scale, scale, scale);
		mtxLocal.makeScaling(vScale);

		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(direction, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*oritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;
		mtxLocal.translate(position);

		mtxWorld = mtxLocal * mtxWorld;
			
		Vector3 pos0(-mWidth/2.f, mHeight/2.f, 0);
		Vector3 pos1(-mWidth/2.f, -mHeight/2.f, 0);
		Vector3 pos2(mWidth/2.f, mHeight/2.f, 0);
		Vector3 pos3(mWidth/2.f, -mHeight/2.f, 0);

		float tl = mMaterial->getTexAddrLeft();
		float tr = mMaterial->getTexAddrRight();
		float tt = mMaterial->getTexAddrTop();
		float tb = mMaterial->getTexAddrBottom();

		// sprite no need to swap uv.

		float use = (tr-tl)/muSegment;
		float vse = (tb-tl)/mvSegment;
		i32 texId = mInnerTime/mSpeedTime;
		texId = texId%(mvSegment*muSegment);
		i32 ubegin = texId%muSegment;
		i32 vbegin = texId/muSegment;
			
		mVertexData[0].pos = pos0 * mtxWorld;
		mVertexData[0].color = colour;
		mVertexData[0].tx = tl + ubegin*use;
		mVertexData[0].ty = tt + vbegin*vse;
		mVertexData[1].pos = pos1 * mtxWorld;
		mVertexData[1].color = colour;
		mVertexData[1].tx = tl + ubegin*use;
		mVertexData[1].ty = tt + (vbegin+1)*vse;
		mVertexData[2].pos = pos2 * mtxWorld;
		mVertexData[2].color = colour;
		mVertexData[2].tx = tl + (ubegin+1)*use;
		mVertexData[2].ty = tt + vbegin*vse;
		mVertexData[3].pos = pos3 * mtxWorld;
		mVertexData[3].color = colour;
		mVertexData[3].tx = tl + (ubegin+1)*use;
		mVertexData[3].ty = tt + (vbegin+1)*vse;

		if (mMaterial->getUV2Enable())
		{
			float tl1 = mMaterial->getTexAddrLeft1();
			float tr1 = mMaterial->getTexAddrRight1();
			float tt1 = mMaterial->getTexAddrTop1();
			float tb1 = mMaterial->getTexAddrBottom1();

			float use1 = (tr1 - tl1) / muSegment;
			float vse1 = (tb1 - tl1) / mvSegment;

			mVertexData[0].tx1 = tl1 + ubegin*use1;
			mVertexData[0].ty1 = tt1 + vbegin*vse1;
			mVertexData[1].tx1 = tl1 + ubegin*use1;
			mVertexData[1].ty1 = tt1 + (vbegin + 1)*vse1;
			mVertexData[2].tx1 = tl1 + (ubegin + 1)*use1;
			mVertexData[2].ty1 = tt1 + vbegin*vse1;
			mVertexData[3].tx1 = tl1 + (ubegin + 1)*use1;
			mVertexData[3].ty1 = tt1 + (vbegin + 1)*vse1;
		}
	}

	void EffectLayerSpriteAnimation::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
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
		if (pRenderable->getLockPtr(4, 6, vertBegin, pVertexPtr, pIdxLockPtr))
		{
			for (int i = 0; i < 4; i++)
			{
				pVertexPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVertexPtr[i].color = m_vertexDataMainThread[i].color;
				pVertexPtr[i].tx = m_vertexDataMainThread[i].tx;
				pVertexPtr[i].ty = m_vertexDataMainThread[i].ty;
				pVertexPtr[i].tx1 = m_vertexDataMainThread[i].tx1;
				pVertexPtr[i].ty1 = m_vertexDataMainThread[i].ty1;
			}

			pIdxLockPtr[0] = vertBegin;
			pIdxLockPtr[1] = vertBegin + 1;
			pIdxLockPtr[2] = vertBegin + 2;

			pIdxLockPtr[3] = vertBegin + 2;
			pIdxLockPtr[4] = vertBegin + 1;
			pIdxLockPtr[5] = vertBegin + 3;
		}
	}

	void EffectLayerSpriteAnimation::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayerSpriteAnimation* sprite = static_cast<EffectLayerSpriteAnimation*>(particle);
		sprite->muSegment = muSegment;
		sprite->mvSegment = mvSegment;
		sprite->mWidth = mWidth;
		sprite->mHeight = mHeight;
		sprite->mSpeedTime = mSpeedTime;
	}

	void EffectLayerSpriteAnimation::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&muSegment, sizeof(i32));
		pStream->write(&mvSegment, sizeof(i32));
		pStream->write(&mWidth, sizeof(float));
		pStream->write(&mHeight, sizeof(float));
		pStream->write(&mSpeedTime, sizeof(i32));
	}

	void EffectLayerSpriteAnimation::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&muSegment, sizeof(i32));
		pStream->read(&mvSegment, sizeof(i32));
		pStream->read(&mWidth, sizeof(float));
		pStream->read(&mHeight, sizeof(float));
		pStream->read(&mSpeedTime, sizeof(i32));
	}

	void EffectLayerSpriteAnimation::mergeAligenBox(Box& aabb)
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

	void EffectLayerSpriteAnimation::getPropertyList(PropertyList & list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_SA_USEG]);
		list.push_back(token[TOKEN_LY_SA_VSEG]);
		list.push_back(token[TOKEN_LY_SA_WIDTH]);
		list.push_back(token[TOKEN_LY_SA_HEIGHT]);
		list.push_back(token[TOKEN_LY_SA_SPEED]);
	}

	bool EffectLayerSpriteAnimation::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;
		if (name == token[TOKEN_LY_SA_USEG])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_SA_VSEG])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_SA_WIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_SA_HEIGHT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_SA_SPEED])
		{
			type = IElement::PT_INT; return true;
		}
		return false;
	}

	bool EffectLayerSpriteAnimation::getPropertyValue(const String & name, String &value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_SA_USEG])
		{
			value = StringUtil::ToString(muSegment); return true;
		}
		else if(name == token[TOKEN_LY_SA_VSEG])
		{
			value = StringUtil::ToString(mvSegment); return true;
		}
		else if(name == token[TOKEN_LY_SA_WIDTH])
		{
			value = StringUtil::ToString(mWidth); return true;
		}
		else if(name == token[TOKEN_LY_SA_HEIGHT])
		{
			value = StringUtil::ToString(mHeight); return true;
		}
		else if(name == token[TOKEN_LY_SA_SPEED])
		{
			value = StringUtil::ToString(mSpeedTime); return true;
		}

		return false;
	}

	bool EffectLayerSpriteAnimation::setPropertyValue(const String & name,const String & value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_SA_USEG])
		{
			muSegment = StringUtil::ParseInt(value); return true;
		}
		else if(name == token[TOKEN_LY_SA_VSEG])
		{
			mvSegment = StringUtil::ParseInt(value); return true;
		}
		else if(name == token[TOKEN_LY_SA_WIDTH])
		{
			mWidth = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_SA_HEIGHT])
		{
			mHeight = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_SA_SPEED])
		{
			mSpeedTime = StringUtil::ParseInt(value); return true;
		}

		return false;
	}
}