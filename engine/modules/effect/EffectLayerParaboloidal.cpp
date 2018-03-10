#include "EffectLayerParaboloidal.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{

	const i32 EffectLayerParaboloidal::sParaboloidalLatitude = 8;
	const i32 EffectLayerParaboloidal::sParaboloidalLongitude = 16;

	EffectLayerParaboloidal::EffectLayerParaboloidal()
		: EffectLayer()
		, mParam(1)
		, mHeight(1)
		, mVertices(NULL)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mType = ELT_Paraboloidal;
	}

	EffectLayerParaboloidal::~EffectLayerParaboloidal()
	{
	}

	void EffectLayerParaboloidal::prepare()
	{
		EffectLayer::prepare();

		mVertices = (EffectVertexFmt*)EchoMalloc(sizeof(EffectVertexFmt)*(sParaboloidalLatitude+1)*(sParaboloidalLongitude));

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
		bool uv2Enable = mMaterial->getUV2Enable();

		Vector3 baseAxis,currentAxis;
		float ulength, vlength, ulength1, vlength1;
		float delta = Math::PI_2/sParaboloidalLongitude;
		Matrix4 mtxRotate;
		for(i32 i=0; i<=sParaboloidalLatitude; ++i)
		{
			baseAxis.z = mHeight / sParaboloidalLatitude * i;
			baseAxis.x = 0.0f;
			baseAxis.y = Math::Sqrt(baseAxis.z*mParam);
			
			ulength = (tr-tl)/sParaboloidalLongitude*i;
			vlength = (tb-tt)/sParaboloidalLongitude*i;

			ulength1 = (tr1 - tl1) / sParaboloidalLongitude*i;
			vlength1 = (tb1 - tt1) / sParaboloidalLongitude*i;

			for(i32 j=0; j<sParaboloidalLongitude; ++j)
			{
				mtxRotate.rotateAxisReplace(Vector3::UNIT_Z, delta*j);
				currentAxis = baseAxis * mtxRotate;

				mVertices[i*sParaboloidalLongitude+j].pos = currentAxis;
				if(swapUV)
				{
					mVertices[i*sParaboloidalLongitude+j].tx = tl+(tr-tl)*0.5f + Math::Cos(delta*j)*ulength;
					mVertices[i*sParaboloidalLongitude+j].ty = tt+(tb-tt)*0.5f + Math::Sin(delta*j)*vlength;
				}
				else
				{
					mVertices[i*sParaboloidalLongitude+j].tx = tl+(tr-tl)*0.5f + Math::Sin(delta*j)*ulength;
					mVertices[i*sParaboloidalLongitude+j].ty = tt+(tb-tt)*0.5f - Math::Cos(delta*j)*vlength;
				}

				if (uv2Enable)
				{
					if (swapUV1)
					{
						mVertices[i*sParaboloidalLongitude + j].tx1 = tl1 + (tr1 - tl1)*0.5f + Math::Cos(delta*j)*ulength1;
						mVertices[i*sParaboloidalLongitude + j].ty1 = tt1 + (tb1 - tt1)*0.5f + Math::Sin(delta*j)*vlength1;
					}
					else
					{
						mVertices[i*sParaboloidalLongitude + j].tx1 = tl1 + (tr1 - tl1)*0.5f + Math::Sin(delta*j)*ulength1;
						mVertices[i*sParaboloidalLongitude + j].ty1 = tt1 + (tb1 - tt1)*0.5f - Math::Cos(delta*j)*vlength1;
					}
				}
			}
		}
	}

	void EffectLayerParaboloidal::unprepare()
	{
		EchoSafeFree(mVertices);
		EffectLayer::unprepare();
	}

	void EffectLayerParaboloidal::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || (!mVertices) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		mVertexData.resize((sParaboloidalLatitude + 1)*sParaboloidalLongitude);

		color = color * mParentSystem->getSystemColor();
		ui32 colour = color.getABGR();
		Matrix4 mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();
		Matrix4 mtxLocal;
		mtxLocal.makeScaling(scale, scale, scale);
		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(Vector3::UNIT_Z, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*oritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;
		mtxLocal.translate(position);
		mtxWorld = mtxLocal * mtxWorld;

		i32 i;
		for (i = 0; i < (sParaboloidalLatitude + 1)*sParaboloidalLongitude; ++i)
		{
			mVertexData[i].pos = mVertices[i].pos * mtxWorld;
			mVertexData[i].color = colour;
			mVertexData[i].tx = mVertices[i].tx; 
			mVertexData[i].ty = mVertices[i].ty;
			mVertexData[i].tx1 = mVertices[i].tx1;
			mVertexData[i].ty1 = mVertices[i].ty1;
		}
	}

	void EffectLayerParaboloidal::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if ((!mbAffected) || (!mbVisible) || (!mVertices) || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		if (mVertexData.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		
		renderables.push_back(pRenderable);

		EffectVertexFmt* pVertexPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		if (pRenderable->getLockPtr((sParaboloidalLatitude + 1)*sParaboloidalLongitude, sParaboloidalLongitude*sParaboloidalLatitude * 6, vertBegin, pVertexPtr, pIdxLockPtr))
		{
			i32 i;

			for (int i = 0; i < (sParaboloidalLatitude + 1)*sParaboloidalLongitude; i++)
			{
				pVertexPtr[i].pos = mVertexData[i].pos;
				pVertexPtr[i].color = mVertexData[i].color;
				pVertexPtr[i].tx = mVertexData[i].tx;
				pVertexPtr[i].ty = mVertexData[i].ty;
				pVertexPtr[i].tx1 = mVertexData[i].tx1;
				pVertexPtr[i].ty1 = mVertexData[i].ty1;
			}

			for (i = 0; i < sParaboloidalLatitude; ++i)
			{
				for (i32 j = 0; j < sParaboloidalLongitude; ++j)
				{
					if (j == sParaboloidalLongitude - 1)
						*pIdxLockPtr++ = vertBegin + 1 - sParaboloidalLongitude;
					else
						*pIdxLockPtr++ = vertBegin + 1;
					*pIdxLockPtr++ = vertBegin + sParaboloidalLongitude;
					*pIdxLockPtr++ = vertBegin;

					*pIdxLockPtr++ = vertBegin + sParaboloidalLongitude;
					if (j == sParaboloidalLongitude - 1)
					{
						*pIdxLockPtr++ = vertBegin + 1 - sParaboloidalLongitude;
						*pIdxLockPtr++ = vertBegin + 1;
					}
					else
					{
						*pIdxLockPtr++ = vertBegin + 1;
						*pIdxLockPtr++ = vertBegin + 1 + sParaboloidalLongitude;
					}
					vertBegin += 1;
				}
			}
		}
	}
	void EffectLayerParaboloidal::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayerParaboloidal* paraboloidal = static_cast<EffectLayerParaboloidal*>(particle);
		paraboloidal->mParam = mParam;
		paraboloidal->mHeight = mHeight;
	}

	void EffectLayerParaboloidal::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mParam, sizeof(float));
		pStream->write(&mHeight, sizeof(float));
	}

	void EffectLayerParaboloidal::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mParam, sizeof(float));
		pStream->read(&mHeight, sizeof(float));
	}

	void EffectLayerParaboloidal::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		Matrix4 mtxLocal;
		mtxLocal.makeScaling(scale, scale, scale);
		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(Vector3::UNIT_Z, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*oritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;
		mtxLocal.translate(position);

		for(size_t i=0; i<(sParaboloidalLatitude+1)*(sParaboloidalLongitude); ++i)
		{
			aabb.addPoint(mVertices[i].pos * mtxLocal);
		}
	}

	void EffectLayerParaboloidal::getPropertyList(PropertyList& list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_PB_PARAM]);
		list.push_back(token[TOKEN_LY_PB_HEIGHT]);
	}

	bool EffectLayerParaboloidal::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;
		if (name == token[TOKEN_LY_PB_PARAM])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PB_HEIGHT])
		{
			type = IElement::PT_REAL; return true;
		}
		return false;
	}

	bool EffectLayerParaboloidal::getPropertyValue(const String& name, String& value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_PB_PARAM])
		{
			value = StringUtil::ToString(mParam); return true;
		}
		else if(name == token[TOKEN_LY_PB_HEIGHT])
		{
			value = StringUtil::ToString(mHeight); return true;
		}

		return false;
	}

	bool EffectLayerParaboloidal::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_PB_PARAM])
		{
			mParam = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PB_HEIGHT])
		{
			mHeight = StringUtil::ParseReal(value); return true;
		}

		return false;
	}
}