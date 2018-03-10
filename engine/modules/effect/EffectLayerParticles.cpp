#include "EffectLayerParticles.h"
#include "EffectSystemManager.h"
#include "engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{

	const i32		EffectLayerParticles::DEFAULT_PT_QUOTA = -1;	
	const bool		EffectLayerParticles::DEFAULT_PT_3DBILLBOARD = false;
	const bool		EffectLayerParticles::DEFAULT_PT_NOTRANSSCALE = false;
	const i32		EffectLayerParticles::DEFAULT_PT_EMITTERRATE = 10;
	const float		EffectLayerParticles::DEFAULT_PT_EMITTERANGLE = 0;
	const float		EffectLayerParticles::DEFAULT_PT_PTSPEED = 0;
	const float		EffectLayerParticles::DEFAULT_PT_MAXPTSPEED = 0;
	const float		EffectLayerParticles::DEFAULT_PT_PTADDSPEED = 0;
	const i32		EffectLayerParticles::DEFAULT_PT_PTLEAVETIME = 1000;
	const i32		EffectLayerParticles::DEFAULT_PT_MAXPTLEAVETIME = 1000;
	const Color		EffectLayerParticles::DEFAULT_PT_MINCOLOR(1,1,1,1);
	const Color		EffectLayerParticles::DEFAULT_PT_MAXCOLOR(1,1,1,1);
	const float		EffectLayerParticles::DEFAULT_PT_MINSCALE = 1;
	const float		EffectLayerParticles::DEFAULT_PT_MAXSCALE = 1;
	const float		EffectLayerParticles::DEFAULT_PT_MINANGLE = 0;
	const float		EffectLayerParticles::DEFAULT_PT_MAXANGLE = 0;
	const bool		EffectLayerParticles::DEFAULT_PT_FALLOWPARENT = true;
	const bool		EffectLayerParticles::DEFAULT_PT_EMITTERINSURFACE = false;
	const i32		EffectLayerParticles::DEFAULT_PT_ANIMROWNUM = 0;
	const i32		EffectLayerParticles::DEFAULT_PT_ANIMCOLUMNNUM = 0;
	const i32		EffectLayerParticles::DEFAULT_PT_ANIMTIMEGAP = 100;
	const i32       EffectLayerParticles::DEFAULT_PT_ANIMSTARTINDEX = 0;
	const i32		EffectLayerParticles::DEFAULT_PT_ANIMENDINDEX = 0;
	const bool		EffectLayerParticles::DEFAULT_PT_ANIMTILED = false;

	EffectLayerParticles::EffectLayerParticles()
		:EffectLayer()
		, mParticleQuota(DEFAULT_PT_QUOTA)
		, mWidth(DEFAULT_LY_WIDTH)
		, mHeight(DEFAULT_LY_HEIGHT)
		, mb3dBillboard(DEFAULT_PT_3DBILLBOARD)
		, mbPerpendicular(DEFAULT_LY_PERPENDICULAR)
		, mb3DBillboardTrans(false)
		, mbNoTransScale(DEFAULT_PT_NOTRANSSCALE)
		, mbNoScaleWidth(DEFAULT_LY_NOSCALEWIDTH)
		, mbNoScaleHeight(DEFAULT_LY_NOSCALEHEIGHT)
		, mCenterOffsetW(DEFAULT_LY_CENTEROFFSETW)
		, mCenterOffsetH(DEFAULT_LY_CENTEROFFSETH)
		, mEmitterRate(DEFAULT_PT_EMITTERRATE)
		, mEmitterAngle(DEFAULT_PT_EMITTERANGLE)
		, mParticleSpeed(DEFAULT_PT_PTSPEED)
		, mParticleMaxSpeed(DEFAULT_PT_MAXPTSPEED)
		, mParticleAddSpeed(DEFAULT_PT_PTADDSPEED)
		, mParticleLeaveTime(DEFAULT_PT_PTLEAVETIME)
		, mParticleMaxLeaveTime(DEFAULT_PT_MAXPTLEAVETIME)
		, mMinColor(DEFAULT_PT_MINCOLOR)
		, mMaxColor(DEFAULT_PT_MAXCOLOR)
		, mMinScale(DEFAULT_PT_MINSCALE)
		, mMaxScale(DEFAULT_PT_MAXSCALE)
		, mMin2dAngle(DEFAULT_PT_MINANGLE)
		, mMax2dAngle(DEFAULT_PT_MAXANGLE)
		, mFallowParent(DEFAULT_PT_FALLOWPARENT)
		, mEmitterInSurface(DEFAULT_PT_EMITTERINSURFACE)
		, mAnimRownum(DEFAULT_PT_ANIMROWNUM)
		, mAnimColumnnum(DEFAULT_PT_ANIMCOLUMNNUM)
		, mAnimTimeGap(DEFAULT_PT_ANIMTIMEGAP)
		, mAnimStartIndex(DEFAULT_PT_ANIMSTARTINDEX)
		, mAnimEndIndex(DEFAULT_PT_ANIMENDINDEX)
		, mbAnimTiled(DEFAULT_PT_ANIMTILED)
		, mParticleControllerSet(0)
		, m_particlePoolChunk(NULL)
		, mChildEffectName("")
		, mChildEffectLoop(false)
		, mChildEffectSpeed(1.0f)
		, mPrewarm(false)
	{
		direction = Vector3::UNIT_Z;
		mbParticleSystem = true;
		createParticleControllerSet();
	}

	EffectLayerParticles::~EffectLayerParticles()
	{
		destroyParticleControllerSet();
	}

	EffectParticleControllerSet* EffectLayerParticles::createParticleControllerSet()
	{
		EffectParticleControllerSet* particleControllerSet = 
			EffectSystemManager::instance()->createParticleControllerSet();

		setParticleControllerSet(particleControllerSet);
		return particleControllerSet;
	}

	void EffectLayerParticles::setParticleControllerSet(EffectParticleControllerSet* particleControllerSet)
	{
		mParticleControllerSet = particleControllerSet;
	}

	EffectParticleControllerSet* EffectLayerParticles::getParticleControllerSet() const
	{
		return mParticleControllerSet;
	}

	void EffectLayerParticles::destroyParticleControllerSet()
	{
		if(mParticleControllerSet)
		{
			EffectSystemManager::instance()->destroyParticleControllerSet(mParticleControllerSet);
			mParticleControllerSet = NULL;
		}
	}

	void EffectLayerParticles::prepare()
	{
		EffectLayer::prepare();

		size_t poolSize = size_t(mEmitterRate * (float(mParticleMaxLeaveTime)/1000.0f)) + 1;

		mEmitterTimeStep = 1000/mEmitterRate;

		//add by ajohn 
		if((i32)poolSize > mParticleQuota && mParticleQuota > 0)
			poolSize = mParticleQuota + 1;
		clearPool();
		createPool(poolSize);
	}

	void EffectLayerParticles::unprepare()
	{
		EffectLayer::unprepare();

		clearPool();
	}

	void EffectLayerParticles::_notifyStart()
	{
		EffectLayer::_notifyStart();

		//mParticlePoolFree.splice(mParticlePoolFree.end(), mParticlePoolInUse);
		mEmitterTime = 0;
		mInnerParticleQuota = mParticleQuota;

		assert(mParticleControllerSet);
		mParticleControllerSet->_notifyStart();
		if (mPrewarm)
		{
			int prewarm = mParticleMaxLeaveTime;
			while (prewarm > 0)
			{
				_update(30);
				prewarm -= 30;
			}
		}
	}

	void EffectLayerParticles::_notifyReStart()
	{
		EffectLayer::_notifyReStart();

		//mParticlePoolFree.splice(mParticlePoolFree.end(), mParticlePoolInUse);
		mEmitterTime = 0;
		mInnerParticleQuota = mParticleQuota;

		assert(mParticleControllerSet);
		mParticleControllerSet->_notifyStart();
	}

	void EffectLayerParticles::_notifyShowVisualHelp(bool show)
	{
		EffectLayer::_notifyShowVisualHelp(show);
	}

	void EffectLayerParticles::emitterParticle(EffectParticle* particle)
	{
		Vector3 v1 = Vector3::UNIT_Z;
		Vector3 v2 = Vector3::UNIT_Z;

		if(mEmitterAngle != DEFAULT_PT_EMITTERANGLE)
		{
			Quaternion q1, q2;
			q1.fromAxisAngle(Vector3::UNIT_X, Math::IntervalRandom(-mEmitterAngle, mEmitterAngle)*Math::DEG2RAD);
			q2.fromAxisAngle(Vector3::UNIT_Y, Math::IntervalRandom(-mEmitterAngle, mEmitterAngle)*Math::DEG2RAD);
			v1 = q1 * v1;
			v2 = q2 * v2;
		}

		if(mFallowParent)
		{
			particle->direction = particle->originalDirection = mb3DBillboardTrans?Vector3::UNIT_Y:Vector3::UNIT_Z;
			particle->oritation = particle->originalOritation = Quaternion::IDENTITY;
			particle->movementDirection = ((v1+v2).normalizedCopy());
		}
		else 
		{
			particle->direction = particle->originalDirection = direction;
			particle->oritation = particle->originalOritation = oritation;
			particle->movementDirection = ((mParentSystem->getEffectSceneNode()->getWorldOrientation()) * oritation) * ((v1+v2).normalizedCopy());
		}
		particle->mTime = 0;

		if(mParticleLeaveTime != mParticleMaxLeaveTime)
			particle->mLeaveTime = (i32)Math::IntervalRandom((float)mParticleLeaveTime, (float)mParticleMaxLeaveTime);
		else
			particle->mLeaveTime = mParticleLeaveTime;

		if(mParticleSpeed != mParticleMaxSpeed)
			particle->originalSpeed = Math::IntervalRandom(mParticleSpeed, mParticleMaxSpeed);
		else
			particle->originalSpeed = mParticleSpeed;

		particle->mInController = false;

		if(mMinColor!=mMaxColor)
		{
			particle->originalColor.r = Math::IntervalRandom(mMinColor.r, mMaxColor.r);
			particle->originalColor.g = Math::IntervalRandom(mMinColor.g, mMaxColor.g);
			particle->originalColor.b = Math::IntervalRandom(mMinColor.b, mMaxColor.b);
			particle->originalColor.a = Math::IntervalRandom(mMinColor.a, mMaxColor.a);
		}
		else
			particle->originalColor = mMinColor;


		if(mMinScale != mMaxScale)
			particle->originalScale = Math::IntervalRandom(mMinScale, mMaxScale);
		else
			particle->originalScale = mMinScale;

		if(mMin2dAngle != mMax2dAngle)
			particle->originalSelfRotation = Math::IntervalRandom(mMin2dAngle, mMax2dAngle);
		else
			particle->originalSelfRotation = mMin2dAngle;
	}

	int EffectLayerParticles::_update(i32 time)
	{
		if (m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		int currentParticlesNum = 0;

		if(EffectLayer::_update(time))
		{
			if(mbAffected)
			{
				/// 1 test emitter particles.
				mEmitterTime += time;
				while(mEmitterTime > mEmitterTimeStep && mInnerParticleQuota != 0)
				{
					--mInnerParticleQuota;
					mEmitterTime -= mEmitterTimeStep;

					// emitter a particle
					EffectParticle* particle = m_particlePoolChunk->New();
					if ( !particle)
						break;

					// add by huangteng for support childeffect particle
					if (!mChildEffectName.empty())
					{
						addChildEffect(particle);
					}
					

					mParticlePoolInUse.push_back(particle);
					emitterParticle(particle);

					particle->startPlayTime = Time::instance()->getMilliseconds();
					particle->mRandomAnimIndex = (int)(Echo::Math::IntervalRandom(mAnimStartIndex*1.0f,mAnimEndIndex*1.0f));
					if(mFallowParent)
					{
						particle->mlastPosition = particle->position;
					}
					else
					{
						Node* parentNode = mParentSystem->getEffectSceneNode();
						const Vector3& pscale = parentNode->getWorldScaling();
						const Quaternion& porita = parentNode->getWorldOrientation();
						const Vector3& ptrans = parentNode->getWorldPosition();
						particle->position = particle->position * pscale;
						particle->position = (porita * originalOritation) * particle->position;
						particle->originalPosition = particle->position;

						//particle->mEmitOritation = porita * oritation;
						particle->scenePosition = ptrans + porita*position*pscale;
						particle->originalScale = particle->scale = pscale.x * particle->originalScale;

						particle->originalOritation = particle->oritation = porita * particle->oritation;

						//particle->position = mSystemOrientation * originalOritation * particle->position;
						if (!mbNoTransScale)
						{
							particle->position *= scale;
						}
						particle->position += particle->scenePosition;
						particle->mlastPosition = particle->position;
					}
				}

				color = color * mParentSystem->getSystemColor();

				/// 2 run all particles by the controllers.
				ParticlePoolItor Iter = mParticlePoolInUse.begin();
				for(; Iter!=mParticlePoolInUse.end(); )
				{
					EffectParticle* particle = *Iter;

					--particle->mReDirection;
					particle->mTime += time;
					if(particle->mTime > particle->mLeaveTime)
					{
						// add by huangteng for support childeffect particle
						if (!mChildEffectName.empty())
						{
							removeChildEffect(particle);
						}
						//drop the died particles.
						mParticlePoolInUse.erase(Iter++);
						//mParticlePoolFree.push_back(particle);
						m_particlePoolChunk->Delete(particle);
						continue;
					}

					/// a controller test.
					particle->reParticleState();
					mParticleControllerSet->_updateParticle(particle);

					/// b slef movement.
					if(particle->originalSpeed != 0)
					{
						float particleTime = float(particle->mTime)*0.001f;
						float ds = particle->originalSpeed*particleTime + 0.5f*mParticleAddSpeed*particleTime*particleTime;
						particle->position += particle->movementDirection*ds;
					}

					/// c trans by parent layer.
					if(mFallowParent)
					{
						if (mbPerpendicular)
						{
							Vector3 deltaPos = particle->position - particle->mlastPosition;
							if (deltaPos.lenSqr() > 0.000001f)
							{
								particle->direction = deltaPos.normalizedCopy();
							}
							else
							{
								particle->direction = particle->originalDirection;
							}
							particle->mlastPosition = particle->position;
						}

						particle->position = oritation * particle->position;
						particle->oritation = oritation*particle->oritation;

						if (!mbPerpendicular)
							particle->direction = particle->oritation*particle->originalDirection;
						else
							particle->direction = oritation * particle->direction;

						if (!mbNoTransScale)
						{
							particle->position *= scale;
							particle->position += position;
						}
						else
						{
							particle->position += position;
						}
					}
					else  // world space.
					{
						if(!mbNoTransScale)
						{
							particle->position *= scale;
						}
						particle->position = particle->originalOritation * particle->position;
						particle->position += particle->scenePosition;

						if (mbPerpendicular)
						{
							Vector3 deltaPos = particle->position - particle->mlastPosition;
							if (deltaPos.lenSqr() > 0.000001f)
							{
								particle->direction = (particle->position - particle->mlastPosition).normalizedCopy();
							}
							else
							{
								particle->direction = particle->originalDirection;
							}
							particle->mlastPosition = particle->position;
						}
					}

					particle->color = color*particle->color;

					++currentParticlesNum;
					++Iter;
				}

				// add by huangteng for support childeffect particle
				if (!mChildEffectName.empty())
				{
					updateChildEffect(time);
				}
			}
			return currentParticlesNum+1;
		}
		else return 0;
	}

	void EffectLayerParticles::clearPool()
	{
		ParticlePoolItor it = mParticlePoolInUse.begin();
		for(; it!=mParticlePoolInUse.end(); ++it)
		{
			m_particlePoolChunk->Delete(*it);
		}

		mParticlePoolInUse.clear();
		EffectSystemManager::instance()->getParticlePool()->ReleaseChunk(m_particlePoolChunk);
		m_particlePoolChunk = NULL;
		// add by huangteng for support childeffect particle
		if (!mChildEffectName.empty())
		{
			clearChildEffect();
		}
	}

	// 创建粒子池
	void EffectLayerParticles::createPool(size_t size)
	{
		EchoAssert(!m_particlePoolChunk);
		m_particlePoolChunk = EffectSystemManager::instance()->getParticlePool()->NewChunk(size);
	}

	void EffectLayerParticles::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold() || !mChildEffectName.empty())
			return;

		size_t particleNum = mParticlePoolInUse.size();

		mVertexData.resize(4 * particleNum);
		EffectVertexFmt* verticsPtr = NULL;
		if (particleNum != 0)
		{
			verticsPtr = &mVertexData[0];
		}

		particleUsed = 0;

		ParticlePoolItor Iter = mParticlePoolInUse.begin();
		for (; Iter != mParticlePoolInUse.end(); ++Iter)
		{
			EffectParticle* particle = *Iter;
			if (particle->mReDirection < 1)
			{
				renderBillboard(particle, verticsPtr);
				particleUsed++;
				verticsPtr += 4;
			}
		}
	}

	void EffectLayerParticles::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		// add by huangteng for support childeffect particle
		if (!mChildEffectName.empty())
		{
			childEffectsSubmitToRQ();
			return;
		}

		if (m_vertexDataMainThread.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		
		renderables.push_back(pRenderable);

		EffectVertexFmt* pVetLockPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		size_t particleNum = m_vertexDataMainThread.size()/4;

		if (pRenderable->getLockPtr(4 * particleNum, 6 * particleNum, vertBegin, pVetLockPtr, pIdxLockPtr))
		{
			for (size_t i = 0; i < 4 * particleNum; i++)
			{
				pVetLockPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVetLockPtr[i].color = m_vertexDataMainThread[i].color;
				pVetLockPtr[i].tx = m_vertexDataMainThread[i].tx;
				pVetLockPtr[i].ty = m_vertexDataMainThread[i].ty;
				pVetLockPtr[i].tx1 = m_vertexDataMainThread[i].tx1;
				pVetLockPtr[i].ty1 = m_vertexDataMainThread[i].ty1;
			}
			// reflash index.
			for (size_t i = 0; i<particleNum; ++i)
			{
				ui16 innerbegin = vertBegin + i * 4;
				*pIdxLockPtr++ = innerbegin;
				*pIdxLockPtr++ = innerbegin + 2;
				*pIdxLockPtr++ = innerbegin + 1;
				*pIdxLockPtr++ = innerbegin + 1;
				*pIdxLockPtr++ = innerbegin + 2;
				*pIdxLockPtr++ = innerbegin + 3;
			}

			//pRenderable->reversePtr(4*(particleNum-particleUsed), 6*(particleNum-particleUsed));
		}
	}

	void EffectLayerParticles::renderBillboard(const EffectParticle* billboard, EffectVertexFmt* pVertexPtr)
	{
		float leftOff = -mCenterOffsetW;
		float rightOff = 1.0f - mCenterOffsetW;
		float topOff = -mCenterOffsetH;
		float bottomOff = 1.0f - mCenterOffsetH;
		Vector3 tmpBbPos = billboard->position;
		Vector3 vOwnOffset[4];
		float bwidth = mbNoScaleWidth?mWidth*scale:(mWidth*scale*billboard->scale);
		float bheight = mbNoScaleHeight?mHeight*scale:(mHeight*scale*billboard->scale);

		float tl = mMaterial->getTexAddrLeft();
		float tr = mMaterial->getTexAddrRight();
		float tt = mMaterial->getTexAddrTop();
		float tb = mMaterial->getTexAddrBottom();

		float tl1 = mMaterial->getTexAddrLeft1();
		float tr1 = mMaterial->getTexAddrRight1();
		float tt1 = mMaterial->getTexAddrTop1();
		float tb1 = mMaterial->getTexAddrBottom1();

		if (mAnimColumnnum > 0 && mAnimColumnnum > 0 && mAnimTimeGap > 0)
		{
			float perFrameU = (tr - tl) / mAnimColumnnum;
			float perFrameV = (tb - tt) / mAnimRownum;
			if (mbAnimTiled) //如果开启平铺
			{
				tl = tl + perFrameU * (billboard->mRandomAnimIndex % (mAnimRownum * mAnimColumnnum));
				tr = tl + perFrameU;
				tt = perFrameV * (billboard->mRandomAnimIndex / (mAnimRownum * mAnimColumnnum));
				tb = tt + perFrameV;
			}
			else
			{
				unsigned long curTime = Time::instance()->getMilliseconds();
				int curindex = (curTime - billboard->startPlayTime) / 25 / mAnimTimeGap;
				curindex = mAnimStartIndex + curindex % (mAnimEndIndex - mAnimStartIndex + 1);
				//curindex = curindex %(mAnimRownum * mAnimColumnnum);

				tl = tl + perFrameU * (curindex % (mAnimColumnnum));
				tr = tl + perFrameU;
				tt = perFrameV * (curindex / (mAnimColumnnum));
				tb = tt + perFrameV;
			}

		}

		bool swapUV = mMaterial->getSwapUV();
		bool swapUV1 = mMaterial->getSwapUV1();
		ui32 colour = billboard->color.getABGR();

		if(mb3dBillboard && !mbPerpendicular)
		{
			leftOff = leftOff * bwidth;
			rightOff = rightOff * bwidth;
			topOff = topOff * bheight;
			bottomOff = bottomOff * bheight;

			if(mb3DBillboardTrans)
			{
				vOwnOffset[0] = Vector3(leftOff, 0, topOff);
				vOwnOffset[1] = Vector3(rightOff, 0, topOff);
				vOwnOffset[2] = Vector3(leftOff, 0, bottomOff);
				vOwnOffset[3] = Vector3(rightOff, 0, bottomOff);
			}
			else
			{
				vOwnOffset[0] = Vector3(leftOff, topOff, 0);
				vOwnOffset[1] = Vector3(rightOff, topOff, 0);
				vOwnOffset[2] = Vector3(leftOff, bottomOff, 0);
				vOwnOffset[3] = Vector3(rightOff, bottomOff, 0);
			}

			Quaternion selfOritation;
			if(Math::Abs(billboard->selfRotation)>1.f)
				selfOritation.fromAxisAngle(billboard->direction, billboard->selfRotation*Math::DEG2RAD);
			Matrix4 localMatrix;
			selfOritation = selfOritation * billboard->oritation;
			localMatrix.fromQuan(selfOritation);
			localMatrix.translate(tmpBbPos);
			if(mFallowParent)
			{
				localMatrix = localMatrix * mParentSystem->getEffectSceneNode()->getWorldMatrix();
			}

			vOwnOffset[0] = vOwnOffset[0] * localMatrix;
			vOwnOffset[1] = vOwnOffset[1] * localMatrix;
			vOwnOffset[2] = vOwnOffset[2] * localMatrix;
			vOwnOffset[3] = vOwnOffset[3] * localMatrix;
		}
		else
		{
			Vector3 camDir = (tmpBbPos - mParentSystem->getCameraPosInEffect()).normalizedCopy();
			Vector3 camX, camY;
			Quaternion camQ;
			if(mFallowParent)
				camQ = mParentSystem->getCameraOriInEffect();
			else
				camQ = mParentSystem->getCurrentCamera()->getOritation();
			if(mb3dBillboard)
			{
				camY = billboard->direction;
				//camX = camDir.cross(Vector3::UNIT_X);
				camX = camDir.cross(camY);
				camX.normalize();
			}
			else
			{
				camX = camQ * Vector3::UNIT_X;
				camY = camQ * Vector3::UNIT_Y;
			}
			Vector3 vLeftOff, vRightOff, vTopOff, vBottomOff;
			vLeftOff   = camX * ( leftOff   * bwidth );
			vRightOff  = camX * ( rightOff  * bwidth );
			vTopOff    = camY * ( topOff   * bheight );
			vBottomOff = camY * ( bottomOff * bheight );

			// Make final offsets to vertex positions
			vOwnOffset[0] = vLeftOff  + vTopOff;
			vOwnOffset[1] = vRightOff + vTopOff;
			vOwnOffset[2] = vLeftOff  + vBottomOff;
			vOwnOffset[3] = vRightOff + vBottomOff;

			Vector3 axis = (vOwnOffset[3] - vOwnOffset[0]).cross(vOwnOffset[2] - vOwnOffset[1]).normalizedCopy();

			if(Math::Abs(billboard->selfRotation) > 1.f)
			{
				Quaternion rotateSelf;
				rotateSelf.fromAxisAngle(axis, billboard->selfRotation*Math::DEG2RAD);

				vOwnOffset[0] = rotateSelf * vOwnOffset[0];
				vOwnOffset[1] = rotateSelf * vOwnOffset[1];
				vOwnOffset[2] = rotateSelf * vOwnOffset[2];
				vOwnOffset[3] = rotateSelf * vOwnOffset[3];
			}

			vOwnOffset[0] = vOwnOffset[0] + tmpBbPos;
			vOwnOffset[1] = vOwnOffset[1] + tmpBbPos;
			vOwnOffset[2] = vOwnOffset[2] + tmpBbPos;
			vOwnOffset[3] = vOwnOffset[3] + tmpBbPos;

			if(mFallowParent)
			{
				Matrix4 mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();
				vOwnOffset[0] = vOwnOffset[0] * mtxWorld;
				vOwnOffset[1] = vOwnOffset[1] * mtxWorld;
				vOwnOffset[2] = vOwnOffset[2] * mtxWorld;
				vOwnOffset[3] = vOwnOffset[3] * mtxWorld;
			}		
		}

		pVertexPtr[0].pos = vOwnOffset[0];
		pVertexPtr[0].color = colour;
		pVertexPtr[1].pos = vOwnOffset[1];
		pVertexPtr[1].color = colour;
		pVertexPtr[2].pos = vOwnOffset[2];
		pVertexPtr[2].color = colour;
		pVertexPtr[3].pos = vOwnOffset[3];
		pVertexPtr[3].color = colour;

		if(swapUV)
		{
			pVertexPtr[0].tx = tr;
			pVertexPtr[0].ty = tb;
			pVertexPtr[1].tx = tr;
			pVertexPtr[1].ty = tt;
			pVertexPtr[2].tx = tl;
			pVertexPtr[2].ty = tb;
			pVertexPtr[3].tx = tl;
			pVertexPtr[3].ty = tt;
		}
		else
		{
			pVertexPtr[0].tx = tl;
			pVertexPtr[0].ty = tb;
			pVertexPtr[1].tx = tr;
			pVertexPtr[1].ty = tb;
			pVertexPtr[2].tx = tl;
			pVertexPtr[2].ty = tt;
			pVertexPtr[3].tx = tr;
			pVertexPtr[3].ty = tt;
		}

		if (mMaterial->getUV2Enable())
		{
			if (swapUV1)
			{
				pVertexPtr[0].tx1 = tr1;
				pVertexPtr[0].ty1 = tb1;
				pVertexPtr[1].tx1 = tr1;
				pVertexPtr[1].ty1 = tt1;
				pVertexPtr[2].tx1 = tl1;
				pVertexPtr[2].ty1 = tb1;
				pVertexPtr[3].tx1 = tl1;
				pVertexPtr[3].ty1 = tt1;
			}
			else
			{
				pVertexPtr[0].tx1 = tl1;
				pVertexPtr[0].ty1 = tb1;
				pVertexPtr[1].tx1 = tr1;
				pVertexPtr[1].ty1 = tb1;
				pVertexPtr[2].tx1 = tl1;
				pVertexPtr[2].ty1 = tt1;
				pVertexPtr[3].tx1 = tr1;
				pVertexPtr[3].ty1 = tt1;
			}
		}
		
	}

	void EffectLayerParticles::copyAttributesTo(EffectParticle* layer)
	{
		EffectLayer::copyAttributesTo(layer);
		EffectLayerParticles* Particles = static_cast<EffectLayerParticles*>(layer);
		Particles->destroyParticleControllerSet();

		Particles->mParticleQuota = mParticleQuota;
		Particles->mb3dBillboard = mb3dBillboard;
		Particles->mbPerpendicular = mbPerpendicular;
		Particles->mb3DBillboardTrans = mb3DBillboardTrans;
		Particles->mbNoTransScale = mbNoTransScale;
		Particles->mEmitterRate = mEmitterRate;
		Particles->mEmitterAngle = mEmitterAngle;
		Particles->mParticleSpeed = mParticleSpeed;
		Particles->mParticleMaxSpeed = mParticleMaxSpeed;
		Particles->mParticleAddSpeed = mParticleAddSpeed;
		Particles->mParticleLeaveTime = mParticleLeaveTime;
		Particles->mParticleMaxLeaveTime = mParticleMaxLeaveTime;
		Particles->mMinColor = mMinColor;
		Particles->mMaxColor = mMaxColor;
		Particles->mMinScale = mMinScale;
		Particles->mMaxScale = mMaxScale;
		Particles->mMin2dAngle = mMin2dAngle;
		Particles->mMax2dAngle = mMax2dAngle;
		Particles->mFallowParent = mFallowParent;
		Particles->mEmitterInSurface = mEmitterInSurface;
		Particles->mWidth = mWidth;
		Particles->mHeight = mHeight;
		Particles->mbNoScaleWidth = mbNoScaleWidth;
		Particles->mbNoScaleHeight = mbNoScaleHeight;
		Particles->mCenterOffsetW = mCenterOffsetW;
		Particles->mCenterOffsetH = mCenterOffsetH;
		Particles->mAnimRownum = mAnimRownum;
		Particles->mAnimColumnnum = mAnimColumnnum;
		Particles->mAnimTimeGap = mAnimTimeGap;
		Particles->mAnimStartIndex = mAnimStartIndex;
		Particles->mAnimEndIndex = mAnimEndIndex;
		Particles->mbAnimTiled = mbAnimTiled;
		Particles->mChildEffectName = mChildEffectName;
		Particles->mChildEffectLoop = mChildEffectLoop;
		Particles->mChildEffectSpeed = mChildEffectSpeed;
		Particles->mPrewarm = mPrewarm;

		Particles->setParticleControllerSet(EffectSystemManager::instance()->cloneParticleControllerSet(mParticleControllerSet));
	}

	void EffectLayerParticles::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		mParticleControllerSet->exportData(pStream, version);
		pStream->write(&mParticleQuota, sizeof(i32));
		pStream->write(&mb3dBillboard, sizeof(bool));
		pStream->write(&mbPerpendicular, sizeof(bool));
		pStream->write(&mb3DBillboardTrans, sizeof(bool));
		pStream->write(&mbNoTransScale, sizeof(bool));
		pStream->write(&mEmitterRate, sizeof(i32));
		pStream->write(&mEmitterAngle, sizeof(float));
		pStream->write(&mParticleSpeed, sizeof(float));
		pStream->write(&mParticleMaxSpeed,sizeof(float));
		pStream->write(&mParticleAddSpeed, sizeof(float));
		pStream->write(&mParticleLeaveTime, sizeof(i32));
		pStream->write(&mParticleMaxLeaveTime, sizeof(i32));
		pStream->write(&mMinColor, sizeof(Color));
		pStream->write(&mMaxColor, sizeof(Color));
		pStream->write(&mMinScale, sizeof(float));
		pStream->write(&mMaxScale, sizeof(float));
		pStream->write(&mMin2dAngle, sizeof(float));
		pStream->write(&mMax2dAngle, sizeof(float));
		pStream->write(&mFallowParent, sizeof(bool));
		pStream->write(&mEmitterInSurface, sizeof(bool));
		pStream->write(&mWidth, sizeof(float));
		pStream->write(&mHeight, sizeof(float));
		pStream->write(&mbNoScaleWidth, sizeof(bool));
		pStream->write(&mbNoScaleHeight, sizeof(bool));
		pStream->write(&mCenterOffsetW, sizeof(float));
		pStream->write(&mCenterOffsetH, sizeof(float));
		if (version > 0x00010003)
		{
			pStream->write(&mAnimRownum, sizeof(i32));
			pStream->write(&mAnimColumnnum, sizeof(i32));
			pStream->write(&mAnimTimeGap, sizeof(i32));
			pStream->write(&mAnimStartIndex, sizeof(i32));
			pStream->write(&mAnimEndIndex, sizeof(i32));
			pStream->write(&mbAnimTiled, sizeof(bool));
		}
		ui32 strlength = mChildEffectName.length();
		pStream->write(&strlength, sizeof(ui32));
		pStream->write(mChildEffectName.c_str(), strlength);
		pStream->write(&mChildEffectLoop, sizeof(bool));
		pStream->write(&mChildEffectSpeed, sizeof(float));

		i32 prewarm = mPrewarm ? 1 : 0;
		pStream->write(&prewarm, sizeof(i32));
	}

	void EffectLayerParticles::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		mParticleControllerSet->importData(pStream, version);
		pStream->read(&mParticleQuota, sizeof(i32));
		pStream->read(&mb3dBillboard, sizeof(bool));
		pStream->read(&mbPerpendicular, sizeof(bool));
		pStream->read(&mb3DBillboardTrans, sizeof(bool));
		pStream->read(&mbNoTransScale, sizeof(bool));
		pStream->read(&mEmitterRate, sizeof(i32));
		pStream->read(&mEmitterAngle, sizeof(float));
		pStream->read(&mParticleSpeed, sizeof(float));
		if(version > 0x00010001)
			pStream->read(&mParticleMaxSpeed,sizeof(float));
		pStream->read(&mParticleAddSpeed, sizeof(float));
		pStream->read(&mParticleLeaveTime, sizeof(i32));
		if(version > 0x00010001)
			pStream->read(&mParticleMaxLeaveTime,sizeof(i32));
		pStream->read(&mMinColor, sizeof(Color));
		pStream->read(&mMaxColor, sizeof(Color));
		pStream->read(&mMinScale, sizeof(float));
		pStream->read(&mMaxScale, sizeof(float));
		pStream->read(&mMin2dAngle, sizeof(float));
		pStream->read(&mMax2dAngle, sizeof(float));
		pStream->read(&mFallowParent, sizeof(bool));
		pStream->read(&mEmitterInSurface, sizeof(bool));
		pStream->read(&mWidth, sizeof(float));
		pStream->read(&mHeight, sizeof(float));
		pStream->read(&mbNoScaleWidth, sizeof(bool));
		pStream->read(&mbNoScaleHeight, sizeof(bool));
		pStream->read(&mCenterOffsetW, sizeof(float));
		pStream->read(&mCenterOffsetH, sizeof(float));

		if (version > 0x00010003)
		{
			pStream->read(&mAnimRownum, sizeof(i32));
			pStream->read(&mAnimColumnnum, sizeof(i32));
			pStream->read(&mAnimTimeGap, sizeof(i32));
			pStream->read(&mAnimStartIndex, sizeof(i32));
			pStream->read(&mAnimEndIndex, sizeof(i32));
			pStream->read(&mbAnimTiled, sizeof(bool));
		}

		if (version > 0x00010007)
		{
			ui32 strLength;
			pStream->read(&strLength, sizeof(ui32));
			char str[128];
			pStream->read(str, strLength);
			str[strLength] = 0;
			mChildEffectName = str;
			pStream->read(&mChildEffectLoop, sizeof(bool));
			pStream->read(&mChildEffectSpeed, sizeof(float));
		}

		if (version > 0x00010015)
		{
			i32 prewarm;
			pStream->read(&prewarm, sizeof(i32));
			mPrewarm = prewarm == 0 ? false : true;
		}

	}

	void EffectLayerParticles::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		ParticlePoolItor Iter = mParticlePoolInUse.begin();
		for(; Iter!=mParticlePoolInUse.end(); ++Iter)
		{
			EffectParticle* particle = *Iter;
			if(particle->mReDirection<1)
			{
				float fWidth = mbNoScaleWidth ? (mWidth*scale) : (mWidth*scale*particle->scale);
				float fHeight = mbNoScaleHeight ? (mHeight*scale) : (mHeight*scale*particle->scale);
				float fAdjust = Math::Max(fWidth/2.f, fHeight/2.f);
				Vector3 vecAdjust(fAdjust, fAdjust, fAdjust);
				aabb.addPoint(particle->position - vecAdjust);
				aabb.addPoint(particle->position + vecAdjust);
			}
		}
	}

	void EffectLayerParticles::getPropertyList(PropertyList& list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_PT_PREWARM]);
		list.push_back(token[TOKEN_LY_PT_QUOTA]);
		list.push_back(token[TOKEN_LY_PT_WIDTH]);
		list.push_back(token[TOKEN_LY_PT_HEIGHT]);
		list.push_back(token[TOKEN_LY_PT_3DBILLBOARD]);
		list.push_back(token[TOKEN_LY_PT_3DTRANS]);
		list.push_back(token[TOKEN_LY_PT_PERPENDICULAR]);
		list.push_back(token[TOKEN_LY_PT_NOTRANSSCALE]);
		list.push_back(token[TOKEN_LY_PT_NOSCALEWIDTH]);
		list.push_back(token[TOKEN_LY_PT_NOSCALEHEIGHT]);
		list.push_back(token[TOKEN_LY_PT_CENTEROFFSETW]);
		list.push_back(token[TOKEN_LY_PT_CENTEROFFSETH]);
		list.push_back(token[TOKEN_LY_PT_EMITTERRATE]);
		list.push_back(token[TOKEN_LY_PT_EMITTERANGLE]);
		list.push_back(token[TOKEN_LY_PT_SPEED]);
		list.push_back(token[TOKEN_LY_PT_MAXSPEED]);
		list.push_back(token[TOKEN_LY_PT_ADDSPEED]);
		list.push_back(token[TOKEN_LY_PT_LEAVETIME]);
		list.push_back(token[TOKEN_LY_PT_MAXLEAVETIME]);
		list.push_back(token[TOKEN_LY_PT_MINCOLOR]);
		list.push_back(token[TOKEN_LY_PT_MAXCOLOR]);
		list.push_back(token[TOKEN_LY_PT_MINSCALE]);
		list.push_back(token[TOKEN_LY_PT_MAXSCALE]);
		list.push_back(token[TOKEN_LY_PT_MIN2DANGLE]);
		list.push_back(token[TOKEN_LY_PT_MAX2DANGLE]);
		list.push_back(token[TOKEN_LY_PT_FALLOWPARENT]);
		list.push_back(token[TOKEN_LY_PT_EMITTERINSURFACE]);
		list.push_back(token[TOKEN_LY_PT_ANIMROWNUM]);
		list.push_back(token[TOKEN_LY_PT_ANIMCOLUMNNUM]);
		list.push_back(token[TOKEN_LY_PT_ANIMTIMEGAP]);
		list.push_back(token[TOKEN_LY_PT_ANIMSTARTINDEX]);
		list.push_back(token[TOKEN_LY_PT_ANIMENDINDEX]);
		list.push_back(token[TOKEN_LY_PT_ANIMTILED]);
		list.push_back(token[TOKEN_LY_PT_CHILDEFFECT_NAME]);
		list.push_back(token[TOKEN_LY_PT_CHILDEFFECT_LOOP]);
		list.push_back(token[TOKEN_LY_PT_CHILDEFFECT_SPEED]);
	}

	bool EffectLayerParticles::getPropertyType(const String& name, PropertyType& type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;
		if (name == token[TOKEN_LY_PT_QUOTA])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_PREWARM])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_WIDTH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_HEIGHT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_3DBILLBOARD])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_PERPENDICULAR])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_3DTRANS])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_NOTRANSSCALE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEWIDTH])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEHEIGHT])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETW])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERRATE])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERANGLE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_SPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_ADDSPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_LEAVETIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXLEAVETIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_PT_MINCOLOR])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXCOLOR])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_LY_PT_MINSCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_MIN2DANGLE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_MAX2DANGLE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_PT_FALLOWPARENT])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERINSURFACE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMROWNUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMCOLUMNNUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTIMEGAP])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMSTARTINDEX])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMENDINDEX])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTILED])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_NAME])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_LOOP])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_SPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		return false;
	}

	bool EffectLayerParticles::getPropertyValue(const String& name, String& value)
	{
		if (EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_PT_QUOTA])
		{
			value = StringUtil::ToString(mParticleQuota); return true;
		}
		else if (name == token[TOKEN_LY_PT_PREWARM])
		{
			value = StringUtil::ToString(mPrewarm); return true;
		}
		else if(name == token[TOKEN_LY_PT_WIDTH])
		{
			value = StringUtil::ToString(mWidth); return true;
		}
		else if(name == token[TOKEN_LY_PT_HEIGHT])
		{
			value = StringUtil::ToString(mHeight); return true;
		}
		else if(name == token[TOKEN_LY_PT_3DBILLBOARD])
		{
			value = StringUtil::ToString(mb3dBillboard); return true;
		}
		else if(name == token[TOKEN_LY_PT_PERPENDICULAR])
		{
			value = StringUtil::ToString(mbPerpendicular); return true;
		}
		else if(name == token[TOKEN_LY_PT_3DTRANS])
		{
			value = StringUtil::ToString(mb3DBillboardTrans); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOTRANSSCALE])
		{
			value = StringUtil::ToString(mbNoTransScale); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEWIDTH])
		{
			value = StringUtil::ToString(mbNoScaleWidth); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEHEIGHT])
		{
			value = StringUtil::ToString(mbNoScaleHeight); return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETW])
		{
			value = StringUtil::ToString(mCenterOffsetW); return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETH])
		{
			value = StringUtil::ToString(mCenterOffsetH); return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERRATE])
		{
			value = StringUtil::ToString(mEmitterRate); return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERANGLE])
		{
			value = StringUtil::ToString(mEmitterAngle); return true;
		}
		else if(name == token[TOKEN_LY_PT_SPEED])
		{
			value = StringUtil::ToString(mParticleSpeed); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSPEED])
		{
			value = StringUtil::ToString(mParticleMaxSpeed); return true;
		}
		else if(name == token[TOKEN_LY_PT_ADDSPEED])
		{
			value = StringUtil::ToString(mParticleAddSpeed); return true;
		}
		else if(name == token[TOKEN_LY_PT_LEAVETIME])
		{
			value = StringUtil::ToString(mParticleLeaveTime); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXLEAVETIME])
		{
			value = StringUtil::ToString(mParticleMaxLeaveTime); return true;
		}
		else if(name == token[TOKEN_LY_PT_MINCOLOR])
		{
			value = StringUtil::ToString(mMinColor); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXCOLOR])
		{
			value = StringUtil::ToString(mMaxColor);  return true;
		}
		else if(name == token[TOKEN_LY_PT_MINSCALE])
		{
			value = StringUtil::ToString(mMinScale); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSCALE])
		{
			value = StringUtil::ToString(mMaxScale); return true;
		}
		else if(name == token[TOKEN_LY_PT_MIN2DANGLE])
		{
			value = StringUtil::ToString(mMin2dAngle); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAX2DANGLE])
		{
			value = StringUtil::ToString(mMax2dAngle); return true;
		}
		else if(name == token[TOKEN_LY_PT_FALLOWPARENT])
		{
			value = StringUtil::ToString(mFallowParent); return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERINSURFACE])
		{
			value = StringUtil::ToString(mEmitterInSurface); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMROWNUM])
		{
			value = StringUtil::ToString(mAnimRownum); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMCOLUMNNUM])
		{
			value = StringUtil::ToString(mAnimColumnnum); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTIMEGAP])
		{
			value = StringUtil::ToString(mAnimTimeGap); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMSTARTINDEX])
		{
			value = StringUtil::ToString(mAnimStartIndex); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMENDINDEX])
		{
			value = StringUtil::ToString(mAnimEndIndex); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTILED])
		{
			value = StringUtil::ToString(mbAnimTiled); return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_NAME])
		{
			value = mChildEffectName; return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_LOOP])
		{
			value = StringUtil::ToString(mChildEffectLoop); return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_SPEED])
		{
			value = StringUtil::ToString(mChildEffectSpeed); return true;
		}
		return false;
	}

	bool EffectLayerParticles::setPropertyValue(const String& name,const String& value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;
		if (name == token[TOKEN_LY_PT_QUOTA])
		{
			mParticleQuota = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_PREWARM])
		{
			mPrewarm = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_WIDTH])
		{
			mWidth = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_HEIGHT])
		{
			mHeight = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_3DBILLBOARD])
		{
			mb3dBillboard = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_PERPENDICULAR])
		{
			mbPerpendicular = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_3DTRANS])
		{
			mb3DBillboardTrans = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOTRANSSCALE])
		{
			mbNoTransScale = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEWIDTH])
		{
			mbNoScaleWidth = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_NOSCALEHEIGHT])
		{
			mbNoScaleHeight = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETW])
		{
			mCenterOffsetW = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_CENTEROFFSETH])
		{
			mCenterOffsetH = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERRATE])
		{
			mEmitterRate = StringUtil::ParseInt(value); 
			if(mEmitterRate < 1)
			{
				mEmitterRate = 1;
			}
			return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERANGLE])
		{
			mEmitterAngle = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_SPEED])
		{
			mParticleSpeed = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSPEED])
		{
			mParticleMaxSpeed = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_ADDSPEED])
		{
			mParticleAddSpeed = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_LEAVETIME])
		{
			mParticleLeaveTime = StringUtil::ParseInt(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXLEAVETIME])
		{
			mParticleMaxLeaveTime = StringUtil::ParseInt(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MINCOLOR])
		{
			mMinColor = StringUtil::ParseColor(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXCOLOR])
		{
			mMaxColor = StringUtil::ParseColor(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MINSCALE])
		{
			mMinScale = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAXSCALE])
		{
			mMaxScale = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MIN2DANGLE])
		{
			mMin2dAngle = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_MAX2DANGLE])
		{
			mMax2dAngle = StringUtil::ParseReal(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_FALLOWPARENT])
		{
			mFallowParent = StringUtil::ParseBool(value); return true;
		}
		else if(name == token[TOKEN_LY_PT_EMITTERINSURFACE])
		{
			mEmitterInSurface = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMROWNUM])
		{
			mAnimRownum = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMCOLUMNNUM])
		{
			mAnimColumnnum = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTIMEGAP])
		{
			mAnimTimeGap = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMSTARTINDEX])
		{
			mAnimStartIndex = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMENDINDEX])
		{
			mAnimEndIndex = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_ANIMTILED])
		{
			mbAnimTiled = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_NAME])
		{
			mChildEffectName = value; return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_LOOP])
		{
			mChildEffectLoop = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_LY_PT_CHILDEFFECT_SPEED])
		{
			mChildEffectSpeed = StringUtil::ParseFloat(value); return true;
		}
		return false;
	}

	void EffectLayerParticles::addChildEffect(EffectParticle* particle)
	{/*
		static ui32 childCounter = 0;
		bool isUIEffect = mParentSystem->getIsUIEffect();
		ChildEffect* pChildEffect = EchoNew(ChildEffect);
		pChildEffect->particle = particle;
		pChildEffect->effectID = EffectSystemManager::instance()->createEffectSystem(Echo::String("child") + Echo::StringUtil::ToString(++childCounter), mChildEffectName, isUIEffect);
		pChildEffect->sceneNode = mParentSystem->getEffectSceneNode()->createChild();
		EffectSystem* pEffect = EffectSystemManager::instance()->getEffectSystem(pChildEffect->effectID);
		if (pEffect)
		{
			pEffect->setParentSceneNode(pChildEffect->sceneNode);
			pEffect->prepare_sys(false);
			pEffect->setCurrentCamera(SceneManager::instance()->getMainCamera());
			pEffect->start();
		}
		mChildEffects.push_back(pChildEffect);
		*/
	}

	void EffectLayerParticles::removeChildEffect(EffectParticle* particle)
	{/*
		list<ChildEffect*>::type::iterator itChileEffect = mChildEffects.begin();
		for (; itChileEffect != mChildEffects.end(); ++itChileEffect)
		{
			if ((*itChileEffect)->particle == particle)
			{
				ChildEffect* pChildEffect = *itChileEffect;

				EffectSystem* pEffect = EffectSystemManager::instance()->getEffectSystem(pChildEffect->effectID);
				if (pEffect)
					pEffect->unprepareLayer();
				EffectSystemManager::instance()->destroyEffectSystem(pChildEffect->effectID);

				node* paraentNode = mParentSystem->getEffectSceneNode();
				if (paraentNode)
					paraentNode->destroyChild(pChildEffect->sceneNode);
				else
					SceneManager::instance()->destroySceneNodes(&pChildEffect->sceneNode, 1);

				pChildEffect->effectID = -1;
				pChildEffect->particle = NULL;
				pChildEffect->sceneNode = NULL;

				EchoSafeDelete(pChildEffect, ChildEffect);
				mChildEffects.erase(itChileEffect);

				break;
			}
		}*/
	}

	void EffectLayerParticles::updateChildEffect(ui32 time)
	{
		list<ChildEffect*>::type::iterator itChileEffect = mChildEffects.begin();
		for (; itChileEffect != mChildEffects.end(); ++itChileEffect)
		{
			EffectSystem* pEffect = EffectSystemManager::instance()->getEffectSystem((*itChileEffect)->effectID);
			if (pEffect && !pEffect->isPlaying() && mChildEffectLoop)
			{
				pEffect->stop();
				pEffect->start();
			}

			Quaternion oritation = (*itChileEffect)->particle->oritation;
			if (mbPerpendicular)
			{
				Vector3 camDir = ((*itChileEffect)->particle->position - mParentSystem->getCameraPosInEffect()).normalizedCopy();
				Vector3 camX, camY;
				Quaternion camQ;
			
				camY = (*itChileEffect)->particle->direction;
				camDir.normalize();
				camX = camY.cross(camDir);
				camX.normalize();
				camY = camDir.cross(camX);

				camQ.fromAxes(camX,camY,camDir);
				oritation *= camQ;
			}

			(*itChileEffect)->sceneNode->setLocalPosition((*itChileEffect)->particle->position);
			(*itChileEffect)->sceneNode->setLocalOrientation(oritation);
			float _scale = (*itChileEffect)->particle->scale * scale;
			(*itChileEffect)->sceneNode->setLocalScalingXYZ(_scale, _scale, _scale);
			(*itChileEffect)->sceneNode->update();
			pEffect->setSystemColor((*itChileEffect)->particle->color);

			pEffect->_update_sys(ui32(time * mChildEffectSpeed));
		}
	}

	void EffectLayerParticles::clearChildEffect()
	{/*
		list<ChildEffect*>::type::iterator itChileEffect = mChildEffects.begin();
		for (; itChileEffect != mChildEffects.end(); ++itChileEffect)
		{
			ChildEffect* pChildEffect = *itChileEffect;

			EffectSystem* pEffect = EffectSystemManager::instance()->getEffectSystem(pChildEffect->effectID);
			if (pEffect)
				pEffect->unprepareLayer();
			EffectSystemManager::instance()->destroyEffectSystem(pChildEffect->effectID);

			node* paraentNode = mParentSystem->getEffectSceneNode();
			if (paraentNode)
				paraentNode->destroyChild(pChildEffect->sceneNode);
			else
				SceneManager::instance()->destroySceneNodes(&pChildEffect->sceneNode, 1);

			pChildEffect->effectID = -1;
			pChildEffect->particle = NULL;
			pChildEffect->sceneNode = NULL;

			EchoSafeDelete(pChildEffect, ChildEffect);
		}
		mChildEffects.clear();*/
	}

	void EffectLayerParticles::childEffectsSubmitToRQ()
	{
		list<ChildEffect*>::type::iterator itChileEffect = mChildEffects.begin();
		for (; itChileEffect != mChildEffects.end(); ++itChileEffect)
		{
			EffectSystem* pEffect = EffectSystemManager::instance()->getEffectSystem((*itChileEffect)->effectID);
			if (pEffect)
			{
				pEffect->submitToRenderQueue();
			}
		}
	}

}