#include "EffectLayerTrail.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "Engine/modules/Anim/SkeletonManager.h"
#include "Engine/modules/Anim/AnimManager.h"
#include "Engine/modules/Anim/Animation.h"
#include "Engine/modules/Anim/AnimState.h"
#include "Engine/modules/Anim/AnimBlender.h"
#include "Engine/modules/Anim/Skeleton.h"
#include "Engine/modules/Anim/Bone.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
	const float EffectLayerTrail::minDistance = 0.002f;

	EffectLayerTrail::EffectLayerTrail()
		: EffectLayer()
		, mOriginalPosition1(0, 0.2f, 0)
		, mOriginalPosition2(0, -0.2f, 0)
		, mDurationTime(500)
		, mFallowParent(false)
		, mEndWithEffectTime(false)
		, mElementNum(0)
		, mUseAnimTrail(false)
		, mAnimMinStep(10)
		, mAnimBeginTime(0)
		, mSkeltonName(StringUtil::BLANK)
		, mAnimationName(StringUtil::BLANK)
		, mAttachBoneName(StringUtil::BLANK)
		, mAttachPosOffset(Vector3::ZERO)
		, mAttachOriOffset(Vector3::ZERO)
		, mSkeleton(nullptr)
		, mAnimation(nullptr)
		, mAnimationState(nullptr)
		, mAnimationBlender(nullptr)
		, mInnerAnimationTime(mAnimBeginTime)
		, mAttachNode(nullptr)
		, mAttachBoneNode(nullptr)
		, mAttachDummyNode(nullptr)
		, mCurrentFrameTime(0)
		, m_elapsedTimecache(0)
	{
		originalDirection = Vector3::UNIT_Z;
		direction = Vector3::UNIT_Z;
		mType = ELT_TRAIL;
	}

	EffectLayerTrail::~EffectLayerTrail()
	{
		mTrailElementList.clear();
	}

	void EffectLayerTrail::prepare()
	{/*
		EffectLayer::prepare();
		if (mUseAnimTrail && mSkeltonName != StringUtil::BLANK && mAnimationName != StringUtil::BLANK && mAttachBoneName != StringUtil::BLANK)
		{
			mSkeleton = SkeletonManager::instance()->createSkeleton(mSkeltonName);
			mSkeleton->prepareLoad();

			// 加载动画数据
			mAnimation = AnimManager::instance()->createAnim(mAnimationName);
			if (!mAnimation->prepareLoad())
			{
				EchoLogError("EffectLayerSkin::Animation[%s] prepare failed", mAnimationName.c_str());
				return;
			}

			mAnimationState = EchoNew(AnimState(mAnimation));
			mAnimationState->setTimePosition(0);

			mAnimationBlender = AnimManager::instance()->createAnimBlender(mSkeleton);
			mAnimationBlender->setBlendMode(AnimBlender::BM_REPLACE);
			mAnimationBlender->blend(mAnimationState, 0);

			Bone* pBone = mSkeleton->getBone(mAttachBoneName);
			if (pBone->isDummy())
			{
				Bone* pParentBone = pBone->getParent();
				mAttachBoneNode = mParentSystem->getEffectSceneNode()->createChild();
				mAttachBoneNode->setLocalOrientation(pParentBone->getRotation());
				mAttachBoneNode->setLocalPosition(pParentBone->getPosition());
				mAttachDummyNode = mAttachBoneNode->createChild();
				mAttachDummyNode->setLocalOrientation(pBone->getRotation());
				mAttachDummyNode->setLocalPosition(pBone->getPosition());
				mAttachNode = mAttachDummyNode->createChild();
				mAttachNode->setLocalPosition(mAttachPosOffset);
				Quaternion ori;
				ori.fromEulerAngle(mAttachOriOffset.x, mAttachOriOffset.y, mAttachOriOffset.z);
				mAttachNode->setLocalOrientation(ori);
			}
			else
			{
				mAttachBoneNode = mParentSystem->getEffectSceneNode()->createChild();
				mAttachBoneNode->setLocalOrientation(pBone->getRotation());
				mAttachBoneNode->setLocalPosition(pBone->getPosition());
				mAttachNode = mAttachBoneNode->createChild();
				mAttachNode->setLocalPosition(mAttachPosOffset);
				Quaternion ori;
				ori.fromEulerAngle(mAttachOriOffset.x, mAttachOriOffset.y, mAttachOriOffset.z);
				mAttachNode->setLocalOrientation(ori);
			}
		}

		*/
	}

	void EffectLayerTrail::unprepare()
	{
		EffectLayer::unprepare();
		/*
		if (mUseAnimTrail)
		{
			if (mAttachNode)
			{
				mAttachNode->getParent()->destroyChild(mAttachNode);
				mAttachNode = nullptr;
			}

			if (mAttachDummyNode)
			{
				mAttachDummyNode->getParent()->destroyChild(mAttachDummyNode);
				mAttachDummyNode = nullptr;
			}

			if (mAttachBoneNode)
			{
				mAttachBoneNode->getParent()->destroyChild(mAttachBoneNode);
				mAttachBoneNode = nullptr;
			}

			AnimManager::instance()->destroyAnimBlenders(&mAnimationBlender, 1);
			EchoSafeDelete(mAnimationState, AnimState);

			if (mAnimation)
			{
				AnimManager::instance()->releaseResource(mAnimation);
				mAnimation = NULL;
			}

			if (mSkeleton)
			{
				SkeletonManager::instance()->releaseResource(mSkeleton);
				mSkeleton = NULL;
			}
		}

		mTrailElementList.clear();
		mElementNum = 0;
		mLastPosition = Vector3::ZERO;
		*/
	}

	int EffectLayerTrail::_update(i32 time)
	{
		int ret  = 0;
		i32 step = mAnimMinStep;
		i32 elapsedTime = time;
		while (elapsedTime > step)
		{
			ret += _updateMinStep(step);
			elapsedTime -= step;
		}
		ret += _updateMinStep(elapsedTime);
		return ret;
	}

	// 根据最小步长更新
	int EffectLayerTrail::_updateMinStep(i32 time)
	{
		if (m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		mCurrentFrameTime = time;

		int ret = EffectLayer::_update(time);
		if (mEndWithEffectTime && !ret)
		{
			return ret;
		}
		//if(mbAffected)
		{
			// test need add vertex.
			Vector3 transed, currentPos;
			if (mFallowParent)
			{
				currentPos = position;
				transed = position - mLastPosition;
			}
			else
			{
				const Matrix4 &mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();
				currentPos = position * mtxWorld;
				transed = currentPos - mLastPosition;
			}

			TrailElementIter it = mTrailElementList.begin();
			mElementNum = 0;
			for (; it != mTrailElementList.end(); ++it)
			{
				TrailElement& element = *it;
				element.currentTime -= time;
				if (element.currentTime <= 0)
				{
					break;
				}
				mElementNum++;
			}
			mTrailElementList.erase(it, mTrailElementList.end());

			// rotate self
			Quaternion rotSelf(1, 0, 0, 0);
			if (Math::Abs(selfRotation) > 1.f)
			{
				rotSelf.fromAxisAngle(direction, selfRotation*Math::DEG2RAD);
			}

			// oritation

			TrailElementList points;
			const auto& worldScale = mFallowParent ? Vector3::ONE : mParentSystem->getEffectSceneNode()->getWorldScaling();
			const auto& worldOrient = mFallowParent ? Quaternion::IDENTITY : mParentSystem->getEffectSceneNode()->getWorldOrientation();
			const auto& worldTranslate = mFallowParent ? Vector3::ZERO : mParentSystem->getEffectSceneNode()->getWorldPosition();

			TrailElement t;
			t.top = mOriginalPosition1*scale;
			t.bottom = mOriginalPosition2*scale;
			t.orient = rotSelf * oritation;
			t.translate = position;
			t.worldScale = worldScale;
			t.worldOrient = worldOrient;
			t.worldTranslate = worldTranslate;
			t.currentTime = mDurationTime;

			if (!mTrailElementList.empty() && !(mUseAnimTrail && mAnimationBlender))
			{
				auto& front = mTrailElementList.front();

				Matrix4 world;
				world.makeScaling(t.worldScale);
				Matrix4 matRot;
				matRot.fromQuan(t.worldOrient);
				world *= matRot;
				world.translate(t.worldTranslate);
				auto currentTop = (t.orient * t.top + t.translate) * world;
				auto currentBottom = (t.orient * t.bottom + t.translate) * world;

				world.identity();
				world.makeScaling(front.worldScale);
				matRot.identity();
				matRot.fromQuan(front.worldOrient);
				world *= matRot;
				world.translate(front.worldTranslate);
				auto lastTop = (front.orient * front.top + front.translate) * world;
				auto lastBottom = (front.orient * front.bottom + front.translate) * world;

				float partition = Math::Max((lastTop - currentTop).len(), (lastBottom - currentBottom).len()) / 0.2;
				for (auto i = 1; i < partition; ++i)
				{
					TrailElement e;
					e.top = front.top - (front.top - t.top) * i / partition;
					e.bottom = front.bottom - (front.bottom - t.bottom) * i / partition;
					Quaternion q;
					Quaternion::Slerp(q, front.orient, t.orient, i / partition, true);
					e.orient = q;
					e.translate = front.translate - (front.translate - t.translate) * i / partition;
					e.worldScale = front.worldScale - (front.worldScale - t.worldScale) * i / partition;
					Quaternion::Slerp(q, front.worldOrient, t.worldOrient, i / partition, true);
					e.worldOrient = q;
					e.worldTranslate = front.worldTranslate - (front.worldTranslate - t.worldTranslate) * i / partition;
					e.currentTime = mDurationTime;
					points.push_back(e);
				}
			}
			points.push_back(t);

			if (mUseAnimTrail && mAnimationBlender)
			{
				if (time == 1 || !ret)
				{
					ret += mElementNum < 2 ? 0 : 1;
					return ret;
				}
				int slerpSize = int(((float)time / (float)mAnimMinStep));

				for (int a = 0; a < slerpSize; ++a)
				{
					mInnerAnimationTime += mAnimMinStep;
					mAnimationState->setTimePosition(mInnerAnimationTime);

					mAnimationBlender->frameMove(mInnerAnimationTime);

					Bone* pbone = mSkeleton->getBone(mAttachBoneName);
					if (pbone->isDummy())
					{
						Bone* pParent = pbone->getParent();
						mAttachBoneNode->setLocalOrientation(mAnimationBlender->getAnimOrientation(pParent->getId()));
						mAttachBoneNode->setLocalPosition(mAnimationBlender->getAnimPosition(pParent->getId()));
					}
					else
					{
						mAttachBoneNode->setLocalOrientation(mAnimationBlender->getAnimOrientation(pbone->getId()));
						mAttachBoneNode->setLocalPosition(mAnimationBlender->getAnimPosition(pbone->getId()));
					}
					mAttachBoneNode->update(true);
					transed = mAttachNode->getWorldPosition() - mLastPosition;
					if (transed.lenSqr() > minDistance)
					{
						for (auto it = points.begin(); it != points.end(); ++it)
						{
							it->currentTime += mAnimMinStep;
							it->worldScale = mAttachNode->getWorldScaling();
							it->worldOrient = mAttachNode->getWorldOrientation();
							it->worldTranslate = mAttachNode->getWorldPosition();
							mTrailElementList.push_front(*it);
							mElementNum++;
						}
						mLastPosition = mAttachNode->getWorldPosition();
					}
				}
			}
			else
			{
				if (transed.lenSqr() > minDistance)
				{
					mLastPosition = currentPos;
					for (auto it = points.begin(); it != points.end(); ++it)
					{
						mTrailElementList.push_front(*it);
						mElementNum++;
					}
				}
			}
		}
		ret += mElementNum < 2 ? 0 : 1;
		return ret;
	}

	void EffectLayerTrail::updateRenderData()
	{
		if ((!mbAffected && mEndWithEffectTime) || (!mbVisible) || mElementNum<2 || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		EchoAssertX(mElementNum == mTrailElementList.size(), "Error: EffectLayerTrail update error");

		mVertexData.resize(mElementNum * 2);

		ui32 colour;
		color = color * mParentSystem->getSystemColor();
		Color tcolor = color;
		size_t elementCount = 0;
		Matrix4 mtxWorld;
		float tl,tt,tr,tb;
		float timeslerp;
		float texslerp, texslerp1;
		float alphaSlerp;
		Vector3 lastbottom, lasttop;
		size_t idx;
		TrailElementIter it = mTrailElementList.begin();

		if(mFallowParent)
		{
			mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();
		}
		else
		{
			mtxWorld.identity();
		}

		tl = mMaterial->getTexAddrLeft();
		tr = mMaterial->getTexAddrRight();
		tt = mMaterial->getTexAddrTop();
		tb = mMaterial->getTexAddrBottom();
		bool swapUV = mMaterial->getSwapUV();

		float tl1 = mMaterial->getTexAddrLeft1();
		float tr1 = mMaterial->getTexAddrRight1();
		float tt1 = mMaterial->getTexAddrTop1();
		float tb1 = mMaterial->getTexAddrBottom1();
		bool swapUV1 = mMaterial->getSwapUV1();
		bool enableUV2 = mMaterial->getUV2Enable();

		size_t elementSize = mTrailElementList.size();
		for(; it!=mTrailElementList.end(); ++it, ++elementCount)
		{
			TrailElement& nextElement = *it;
			timeslerp = (float(nextElement.currentTime)/float(mDurationTime));
			if (mUseAnimTrail)
			{
				alphaSlerp = timeslerp * (1.0f - float(elementCount) / float(elementSize));
				timeslerp = (1.0f - float(elementCount) / float(elementSize));
			}

			if(swapUV)
				texslerp = tb - (tb-tt)*timeslerp;
			else
				texslerp = tr - (tr-tl)*timeslerp;

			if (enableUV2)
			{
				if (swapUV1)
					texslerp1 = tb1 - (tb1 - tt1)*timeslerp;
				else
					texslerp1 = tr1 - (tr1 - tl1)*timeslerp;
			}

			tcolor = color;
			tcolor.a *= timeslerp;//mUseAnimTrail ? alphaSlerp : timeslerp;
			colour = tcolor.getABGR();
			
			Matrix4 world;
			world.makeScaling(it->worldScale);
			Matrix4 matRot;
			matRot.fromQuan(it->worldOrient);
			world *= matRot;
			world.translate(it->worldTranslate);
			lasttop = (it->orient * it->top + it->translate) * world;
			lastbottom = (it->orient * it->bottom + it->translate) * world;
			if(mFallowParent)
			{
				lastbottom = lastbottom * mtxWorld;
				lasttop = lasttop * mtxWorld;
			}
			idx = elementCount*2;
			mVertexData[idx].pos = lastbottom;
			mVertexData[idx].color = colour;
			if(swapUV)
			{
				mVertexData[idx].tx = tl;
				mVertexData[idx].ty = texslerp;
			}
			else
			{
				mVertexData[idx].tx = texslerp;
				mVertexData[idx].ty = tb;
			}

			if (enableUV2)
			{
				if (swapUV1)
				{
					mVertexData[idx].tx1 = tl1;
					mVertexData[idx].ty1 = texslerp1;
				}
				else
				{
					mVertexData[idx].tx1 = texslerp1;
					mVertexData[idx].ty1 = tb1;
				}
			}

			idx = elementCount*2+1;
			mVertexData[idx].pos = lasttop;
			mVertexData[idx].color = colour;
			if(swapUV)
			{
				mVertexData[idx].tx = tr;
				mVertexData[idx].ty = texslerp;
			}
			else
			{
				mVertexData[idx].tx = texslerp;
				mVertexData[idx].ty = tt;
			}

			if (enableUV2)
			{
				if (swapUV1)
				{
					mVertexData[idx].tx1 = tr1;
					mVertexData[idx].ty1 = texslerp1;
				}
				else
				{
					mVertexData[idx].tx1 = texslerp1;
					mVertexData[idx].ty1 = tt1;
				}
			}
		}
	}

	void EffectLayerTrail::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if (/*(!mbAffected) ||*/ (!mbVisible) || mElementNum < 2 || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		if (m_vertexDataMainThread.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		renderables.push_back(pRenderable);

		float _transu = 0.f;
		float _transv = 0.f;
		float _transu1 = 0.f;
		float _transv1 = 0.f;
		float _USpeed, _VSpeed, _USpeed1, _VSpeed1;
		if (mCurrentFrameTime != 0)
		{
			_USpeed = mMaterial->getUSpeed();
			if (_USpeed != 0)
			{
				_transu = _USpeed * (float)(mCurrentFrameTime)* 0.001f;
			}

			_VSpeed = mMaterial->getVSpeed();
			if (_VSpeed != 0)
			{
				_transv = _VSpeed * (float)(mCurrentFrameTime)* 0.001f;
			}

			_USpeed1 = mMaterial->getUSpeed1();
			if (_USpeed1 != 0)
			{
				_transu1 = _USpeed1 * (float)(mCurrentFrameTime)* 0.001f;
			}

			_VSpeed1 = mMaterial->getVSpeed1();
			if (_VSpeed1 != 0)
			{
				_transv1 = _VSpeed1 * (float)(mCurrentFrameTime)* 0.001f;
			}

			mCurrentFrameTime = 0;
		}

		EffectVertexFmt* pVertexPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		if (pRenderable->getLockPtr(mElementNum * 2, 6 * (mElementNum - 1), vertBegin, pVertexPtr, pIdxLockPtr))
		{
			for( size_t i =0;i<mElementNum*2;i++)
			{
				pVertexPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVertexPtr[i].color = m_vertexDataMainThread[i].color;
				pVertexPtr[i].tx = m_vertexDataMainThread[i].tx + _transu;
				pVertexPtr[i].ty = m_vertexDataMainThread[i].ty + _transv;
				pVertexPtr[i].tx1 = m_vertexDataMainThread[i].tx1 + _transu1;
				pVertexPtr[i].ty1 = m_vertexDataMainThread[i].ty1 + _transv1;
			}

			ui16 baseInx, baseVtx;
			for(size_t i=0; i<mElementNum-1; ++i)
			{
				baseInx = i*6;
				baseVtx = vertBegin+i*2;
				pIdxLockPtr[baseInx]   = baseVtx;
				pIdxLockPtr[baseInx+1] = baseVtx+1;
				pIdxLockPtr[baseInx+2] = baseVtx+2;

				pIdxLockPtr[baseInx+3] = baseVtx+2;
				pIdxLockPtr[baseInx+4] = baseVtx+1;
				pIdxLockPtr[baseInx+5] = baseVtx+3;
			}
		}
	}

	void EffectLayerTrail::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mOriginalPosition1, sizeof(float)*3);
		pStream->write(&mOriginalPosition2, sizeof(float)*3);
		pStream->write(&mDurationTime, sizeof(i32));
		pStream->write(&mFallowParent, sizeof(bool));
		pStream->write(&mEndWithEffectTime, sizeof(bool));
		pStream->write(&mUseAnimTrail, sizeof(bool));
		if (mUseAnimTrail)
		{
			ui32 strlength = static_cast<ui32>(mSkeltonName.length());
			pStream->write(&strlength, sizeof(strlength));
			pStream->write(mSkeltonName.c_str(), strlength);
			strlength = static_cast<ui32>(mAnimationName.length());
			pStream->write(&strlength, sizeof(strlength));
			pStream->write(mAnimationName.c_str(), strlength);
			strlength = static_cast<ui32>(mAttachBoneName.length());
			pStream->write(&strlength, sizeof(strlength));
			pStream->write(mAttachBoneName.c_str(), strlength);
			pStream->write(&mAttachPosOffset, sizeof(float)* 3);
			pStream->write(&mAttachOriOffset, sizeof(float)* 3);
			pStream->write(&mAnimBeginTime, sizeof(i32));
			pStream->write(&mAnimMinStep, sizeof(i32));
		}
	}

	void EffectLayerTrail::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mOriginalPosition1, sizeof(float)*3);
		pStream->read(&mOriginalPosition2, sizeof(float)*3);
		pStream->read(&mDurationTime, sizeof(i32));
		pStream->read(&mFallowParent, sizeof(bool));
		if (version > 0x00010012)
		{
			pStream->read(&mEndWithEffectTime, sizeof(bool));
		}

		if (version > 0x00010009)
		{
			pStream->read(&mUseAnimTrail, sizeof(bool));
			if (mUseAnimTrail)
			{
				ui32 strLength;
				char str[128];
				pStream->read(&strLength, sizeof(strLength));
				pStream->read(str, strLength);
				str[strLength] = 0;
				mSkeltonName = str;
				pStream->read(&strLength, sizeof(strLength));
				pStream->read(str, strLength);
				str[strLength] = 0;
				mAnimationName = str;
				pStream->read(&strLength, sizeof(strLength));
				pStream->read(str, strLength);
				str[strLength] = 0;
				mAttachBoneName = str;
				pStream->read(&mAttachPosOffset, sizeof(float)* 3);
				pStream->read(&mAttachOriOffset, sizeof(float)* 3);
				pStream->read(&mAnimBeginTime, sizeof(i32));
				pStream->read(&mAnimMinStep, sizeof(i32));
			}
			
		}
		
	}

	void EffectLayerTrail::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		TrailElementIter it = mTrailElementList.begin();
		for(; it!=mTrailElementList.end(); ++it)
		{
			TrailElement& element = *it;
			// in editor, it is this. dont thank about the fllow parent property.
			aabb.addPoint(element.top);
			aabb.addPoint(element.bottom);
		}
	}

	void EffectLayerTrail::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayerTrail* trail = static_cast<EffectLayerTrail*>(particle);
		trail->mOriginalPosition1 = mOriginalPosition1;
		trail->mOriginalPosition2 = mOriginalPosition2;
		trail->mDurationTime = mDurationTime;
		trail->mFallowParent = mFallowParent;
		trail->mUseAnimTrail = mUseAnimTrail;
		trail->mSkeltonName = mSkeltonName;
		trail->mAnimationName = mAnimationName;
		trail->mAttachBoneName = mAttachBoneName;
		trail->mAttachPosOffset = mAttachPosOffset;
		trail->mAttachOriOffset = mAttachOriOffset;
		trail->mAnimBeginTime = mAnimBeginTime;
		trail->mAnimMinStep = mAnimMinStep;
		trail->mEndWithEffectTime = mEndWithEffectTime;
	}

	void EffectLayerTrail::_notifyReStart()
	{
		EffectLayer::_notifyReStart();
		mInnerAnimationTime = mAnimBeginTime;
		resetElements();
	}

	void EffectLayerTrail::_notifyStart()
	{
		EffectLayer::_notifyStart();
		mInnerAnimationTime = mAnimBeginTime;
		resetElements();
	}

	void EffectLayerTrail::resetElements()
	{
		mTrailElementList.clear();
		mElementNum = 0;
		mLastPosition = Vector3::ZERO;
	}

	void EffectLayerTrail::getPropertyList(PropertyList& list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_TR_POSITION1]);
		list.push_back(token[TOKEN_LY_TR_POSITION2]);
		list.push_back(token[TOKEN_LY_TR_DURATIONTIME]);
		list.push_back(token[TOKEN_LY_TR_FALLOWPARENT]);
		list.push_back(token[TOKEN_LY_TR_ENDTIME]);
		list.push_back(token[TOKEN_LY_TR_ANIM_ENABLE]);
		list.push_back(token[TOKEN_LY_TR_ANIM_SKELNAME]);
		list.push_back(token[TOKEN_LY_TR_ANIM_ANIMNAME]);
		list.push_back(token[TOKEN_LY_TR_ANIM_BONENAME]);
		list.push_back(token[TOKEN_LY_TR_ANIM_POSOFFSET]);
		list.push_back(token[TOKEN_LY_TR_ANIM_ORIOFFSET]);
		list.push_back(token[TOKEN_LY_TR_ANIM_BEGINTIME]);
		list.push_back(token[TOKEN_LY_TR_ANIM_STEP]);
	}

	bool EffectLayerTrail::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_LY_TR_POSITION1])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_LY_TR_POSITION2])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_LY_TR_DURATIONTIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_TR_FALLOWPARENT])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_TR_ENDTIME])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ENABLE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ANIMNAME])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_SKELNAME])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BONENAME])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BEGINTIME])
		{
			type = IElement::PT_UINT; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_STEP])
		{
			type = IElement::PT_UINT; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_POSOFFSET])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ORIOFFSET])
		{
			type = IElement::PT_VECTOR3; return true;
		}

		return false;
	}

	bool EffectLayerTrail::getPropertyValue(const String& name, String& value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_TR_POSITION1])
		{
			value = StringUtil::ToString(mOriginalPosition1); return true;
		}
		else if(name == token[TOKEN_LY_TR_POSITION2])
		{
			value = StringUtil::ToString(mOriginalPosition2); return true;
		}
		else if(name == token[TOKEN_LY_TR_DURATIONTIME])
		{
			value = StringUtil::ToString(mDurationTime); return true;
		}
		else if(name == token[TOKEN_LY_TR_FALLOWPARENT])
		{
			value = StringUtil::ToString(mFallowParent); return true;
		}
		else if (name == token[TOKEN_LY_TR_ENDTIME])
		{
			value = StringUtil::ToString(mEndWithEffectTime); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ENABLE])
		{
			value = StringUtil::ToString(mUseAnimTrail); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ANIMNAME])
		{
			value = mAnimationName; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_SKELNAME])
		{
			value = mSkeltonName; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BONENAME])
		{
			value = mAttachBoneName; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BEGINTIME])
		{
			value = StringUtil::ToString(mAnimBeginTime); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_STEP])
		{
			value = StringUtil::ToString(mAnimMinStep); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_POSOFFSET])
		{
			value = StringUtil::ToString(mAttachPosOffset); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ORIOFFSET])
		{
			value = StringUtil::ToString(mAttachOriOffset); return true;
		}

		return false;
	}

	bool EffectLayerTrail::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_TR_POSITION1])
		{
			mOriginalPosition1 = StringUtil::ParseVec3(value); return true;
		}
		else if(name == token[TOKEN_LY_TR_POSITION2])
		{
			mOriginalPosition2 = StringUtil::ParseVec3(value); return true;
		}
		else if(name == token[TOKEN_LY_TR_DURATIONTIME])
		{
			mDurationTime = StringUtil::ParseInt(value); return true;
		}
		else if(name == token[TOKEN_LY_TR_FALLOWPARENT])
		{
			mFallowParent = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ENDTIME])
		{
			mEndWithEffectTime = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ENABLE])
		{
			mUseAnimTrail = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ANIMNAME])
		{
			mAnimationName = value; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_SKELNAME])
		{
			mSkeltonName = value; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BONENAME])
		{
			mAttachBoneName = value; return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_BEGINTIME])
		{
			mAnimBeginTime = StringUtil::ParseUI32(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_STEP])
		{
			mAnimMinStep = StringUtil::ParseUI32(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_POSOFFSET])
		{
			mAttachPosOffset = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_LY_TR_ANIM_ORIOFFSET])
		{
			mAttachOriOffset = StringUtil::ParseVec3(value); return true;
		}

		return false;
	}



}