#include "EffectSystem.h"
#include "EffectLayer.h"
#include "EffectSystemManager.h"
#include "engine/core/io/DataStream.h"
#include "EffectKeyFrame.h"
#include "Engine/modules/Audio/FMODStudio/FSAudioManager.h" 
#include <algorithm>
#include <iterator>

namespace Echo
{
	const int	EffectLayer::DEFAULT_REPLAY_COUNT = 1;
	const int	EffectLayer::DEFAULT_REPLAY_TIME = 0;
	const bool	EffectLayer::DEFAULT_RESTATE_LOOPEND = false;
	const float EffectLayer::DEFAULT_LY_WIDTH = 1.0f;
	const float EffectLayer::DEFAULT_LY_HEIGHT = 1.0f;
	const bool	EffectLayer::DEFAULT_LY_PERPENDICULAR = false;
	const bool	EffectLayer::DEFAULT_LY_NOSCALEHEIGHT = false;
	const bool	EffectLayer::DEFAULT_LY_NOSCALEWIDTH = false;
	const float EffectLayer::DEFAULT_LY_CENTEROFFSETW = 0.5f;
	const float EffectLayer::DEFAULT_LY_CENTEROFFSETH = 0.5f;
	const float EffectLayer::DEFAULT_LY_OFFSETVIEWPORT = 0.0f;
	const float EffectLayer::DEFAULT_LY_BOUNDALPHA = 0.05f;

	EffectLayer::EffectLayer()
		:m_renderLevel(ERL_Super)
		, mRenderGroup(ERG_GROUPUP)
		, mReplayCount(DEFAULT_REPLAY_COUNT)
		, mReplayTime(DEFAULT_REPLAY_TIME)
		, mbRestateLoopEnd(DEFAULT_RESTATE_LOOPEND)
		, mName(StringUtil::BLANK)
		, mType(ELT_Unknow)
		, mParentSystem(NULL)
		, mVisualbleParticleNum(0)
		, mbVisible(true)
		, mCurrentTime(0)
		, mReplayIdelTime(0)
		, mbReplayIdel(false)
		, mFrameIndex(0)
		, mbPlayEnd(false)
		, mbAffected(false)
		, mpFallowLayer(NULL)
		, mMaterial(NULL)
		, mbParticleSystem(false)
		, mbShowVisualHelp(false)
	{

	}

	EffectLayer::~EffectLayer()
	{
		destroyEffectSound();
		destroyAllKeyFrames();
		mMaterial->unprepare();
		destroyMaterial();
	}

	void EffectLayer::prepare_res()
	{
		if (mMaterial)
			mMaterial->prepare_res();
	}

	void EffectLayer::prepare_io()
	{
		if (!mMaterial)
			assert(0);
		mMaterial->setIsUIEffect( mParentSystem->getIsUIEffect());
		mMaterial->prepare_io();
	}

	void EffectLayer::prepare()
	{
		if (!mMaterial)
			assert(0);

		mMaterial->prepare();

	}

	void EffectLayer::unprepare()
	{
		mMaterial->unprepare();
	}

	String EffectLayer::getName() const
	{
		return mName;
	}

	void EffectLayer::setName(const String& name)
	{
		mName = name;
	}

	void EffectLayer::_notifyReStart()
	{
		mCurrentTime = 0;
		mReplayIdelTime = 0;
		mbReplayIdel = false;
		mFrameIndex = 0;
		mbPlayEnd = false;
		mbAffected = false;

		if (mMaterial)
			mMaterial->_notifyStart();

		if (mbRestateLoopEnd)
		{
			reParticleState();
		}

		EffectKeyFrame* secondKeyframe = NULL;
		if (mKeyFrames.size() > 1)
			secondKeyframe = mKeyFrames[1];
		mKeyFrames[0]->_notifyStart(secondKeyframe);
		resetEffectSoundState();
	}

	void EffectLayer::_notifyStart()
	{
		mCurrentTime = 0;
		mReplayIdelTime = 0;
		mbReplayIdel = false;
		mbPlayEnd = false;
		mbAffected = false;
		mFrameIndex = 0;
		mInerReplayCount = mReplayCount;

		if (mMaterial)
			mMaterial->_notifyStart();

		EffectKeyFrame* secondKeyframe = NULL;
		if (mKeyFrames.size() > 1)
			secondKeyframe = mKeyFrames[1];
		mKeyFrames[0]->_notifyStart(secondKeyframe);
	}

	void EffectLayer::_notifyStop()
	{
		stopEffectSound();

		mVertexData.clear();
	}

	void EffectLayer::_notifyPause()
	{

	}

	void EffectLayer::_notifyResume()
	{

	}

	void EffectLayer::_notifyAffected()
	{
		_update(1);
	}

	void EffectLayer::_notifyUnAffected()
	{

	}

	void EffectLayer::_notifyExtenalParam(const Vector3& p1, const Vector3& p2)
	{
		// nothing to do.
	}

	void EffectLayer::_notifyShowVisualHelp(bool show)
	{
		mbShowVisualHelp = show;
	}

	void EffectLayer::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer* pLayer = (EffectLayer*)particle;
		pLayer->destroyAllKeyFrames();
		pLayer->destroyMaterial();

		EffectParticle::copyAttributesTo(pLayer);

		// copy property
		pLayer->mbRestateLoopEnd = pLayer->mbRestateLoopEnd;
		pLayer->mName = mName;
		pLayer->mbVisible = mbVisible;
		pLayer->m_renderLevel = m_renderLevel;
		pLayer->mRenderGroup = mRenderGroup;
		pLayer->mReplayCount = mReplayCount;
		pLayer->mReplayTime = mReplayTime;

		pLayer->mFrameIndex = mFrameIndex;
		pLayer->mReplayIdelTime = mReplayIdelTime;
		pLayer->mbReplayIdel = mbReplayIdel;
		pLayer->mCurrentTime = mCurrentTime;
		pLayer->mbPlayEnd = mbPlayEnd;
		pLayer->mInerReplayCount = mInerReplayCount;

		// copy componment
		EffectKeyFrame* cloneKeyFrame = 0;
		for (size_t i = 0; i < getNumKeyFrames(); ++i)
		{
			cloneKeyFrame = EffectSystemManager::instance()->cloneKeyFrame(getKeyFrame(i));
			pLayer->addKeyFrame(cloneKeyFrame, i == 0 ? -1 : i);
		}

		pLayer->setEffectMaterial(EffectSystemManager::instance()->cloneMaterial(mMaterial));
	}

	void EffectLayer::setRenderQueueGroup(EFFECT_RENDER_GROUP rg)
	{
		mRenderGroup = rg;
	}

	EFFECT_RENDER_GROUP EffectLayer::getRenderQueueGroup() const
	{
		return mRenderGroup;
	}

	void EffectLayer::updateRenderData()
	{

	}

	bool EffectLayer::getVisible()const
	{
		return mbVisible;
	}

	void EffectLayer::setVisible(bool visible)
	{
		mbVisible = visible;
	}

	EffectMaterial* EffectLayer::createMaterial()
	{
		EffectMaterial* material = EffectSystemManager::instance()->createMaterial();
		setEffectMaterial(material);
		return material;
	}

	void EffectLayer::setEffectMaterial(EffectMaterial* material)
	{
		mMaterial = material;
	}

	void EffectLayer::destroyMaterial()
	{
		EffectSystemManager::instance()->destroyMaterial(mMaterial);
		mMaterial = NULL;
	}

	EffectKeyFrame* EffectLayer::createkeyFrame(size_t index /* = -1 */)
	{
		EffectKeyFrame* keyFrame = EffectSystemManager::instance()->createKeyFrame();
		addKeyFrame(keyFrame, index);
		return keyFrame;
	}

	void EffectLayer::addKeyFrame(EffectKeyFrame* keyFrame, size_t index)
	{
		if (index == -1)
		{
			mKeyFrames.push_back(keyFrame);
			mSoundMap[0] = keyFrame->getEffectSound();
		}
		else
		{
			mKeyFrames.push_back(keyFrame);
			EffectKeyFrame* temp;
			for (size_t tindex = mKeyFrames.size() - 1; tindex > index + 1; tindex--)
			{
				temp = mKeyFrames[tindex];
				mKeyFrames[tindex] = mKeyFrames[tindex - 1];
				mKeyFrames[tindex - 1] = temp;
			}
			mSoundMap[index + 1] = keyFrame->getEffectSound();
		}
		keyFrame->setParentLayer(this);
	}

	void EffectLayer::removeKeyFrame(EffectKeyFrame* keyFrame)
	{
		assert(keyFrame && "KeyFrame is null!");
		EffectKeyFrameIterator it;
		for (it = mKeyFrames.begin(); it != mKeyFrames.end(); ++it)
		{
			if ((*it) == keyFrame)
			{
				mKeyFrames.erase(it);
				break;
			}
		}

		EffectSoundMap::iterator it2;
		for (it2 = mSoundMap.begin(); it2 != mSoundMap.end(); ++it2)
		{
			if (it2->second == keyFrame->getEffectSound())
			{
				mSoundMap.erase(it2);
				break;
			}
		}

		keyFrame->setParentLayer(0);
	}

	EffectKeyFrame* EffectLayer::getKeyFrame(size_t index) const
	{
		assert(index < mKeyFrames.size() && "KeyFrame index out of bounds!");
		return mKeyFrames[index];
	}

	size_t EffectLayer::getNumKeyFrames() const
	{
		return mKeyFrames.size();
	}

	void EffectLayer::destroyKeyFrame(size_t index)
	{
		destroyKeyFrame(getKeyFrame(index));
	}

	void EffectLayer::destroyKeyFrame(EffectKeyFrame* keyFrame)
	{
		assert(keyFrame && "KeyFrame is null!");
		EffectKeyFrameIterator it;
		for (it = mKeyFrames.begin(); it != mKeyFrames.end(); ++it)
		{
			if ((*it) == keyFrame)
			{
				EffectSystemManager::instance()->destroyKeyFrame(*it);
				mKeyFrames.erase(it);
				break;
			}
		}
	}

	void EffectLayer::destroyAllKeyFrames()
	{
		EffectKeyFrameIterator it;
		for (it = mKeyFrames.begin(); it != mKeyFrames.end(); ++it)
		{
			EffectSystemManager::instance()->destroyKeyFrame(*it);
		}
		mKeyFrames.clear();
	}

	int EffectLayer::_update(i32 time)
	{
		if (mbPlayEnd || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return 0;

		mCurrentTime += time;

		if (mbReplayIdel)
		{
			mReplayIdelTime += time;
			if (mReplayIdelTime >= mReplayTime)
			{
				mInerReplayCount--;
				mbReplayIdel = false;
				if (mInerReplayCount == 0)
				{
					_notifyStop();
					mbPlayEnd = true;
				}
				else
				{
					_notifyReStart();
					return 1;
				}
			}
			else
			{
				return 1;
			}
		}
		else if (!mbAffected)
		{
			i32 frameTime = mCurrentTime - mKeyFrames[mFrameIndex]->getBeginTime();
			if (frameTime < 0)
			{
				return 1;
			}
			else
			{
				mbAffected = true;
				_notifyAffected();
				_updateKeyFrame(frameTime);
			}
		}
		else
		{
			_updateKeyFrame(time);
		}

		if (mbPlayEnd)
			return 0;
		return 1;
	}

	// 更新关键帧
	void EffectLayer::_updateKeyFrame(i32 time)
	{
		assert(mbAffected);

		if (mMaterial)
		{
			mMaterial->_update(time);
		}

		i32 remainderTime = mKeyFrames[mFrameIndex]->_update_sys(time);
		if (mKeyFrames[mFrameIndex]->needPlaySound())
		{
			if (mParentSystem && mParentSystem->getIsRealPlaying() && mSoundMap.find(mFrameIndex) != mSoundMap.end())
			{
				playEffectSound(mFrameIndex);
			}
		}


		if (remainderTime > 0)
		{
			size_t frameCount = mKeyFrames.size();
			mFrameIndex++;
			if (mFrameIndex >= frameCount)
			{
				mbAffected = false;
				_notifyUnAffected();
				if (mReplayTime > 0)
				{
					mReplayIdelTime = remainderTime;
					mbReplayIdel = true;
				}
				else
				{
					mInerReplayCount--;
					mbReplayIdel = false;
					if (mInerReplayCount == 0)
					{
						_notifyStop();
						mbPlayEnd = true;
					}
					else
					{
						_notifyReStart();
						_update(remainderTime);
					}
				}
			}
			else
			{
				EffectKeyFrame* secondKeyframe = NULL;
				if (mKeyFrames.size() > mFrameIndex + 1)
					secondKeyframe = mKeyFrames[mFrameIndex + 1];
				mKeyFrames[mFrameIndex]->_notifyContinue(secondKeyframe);
				_updateKeyFrame(remainderTime);
			}
		}
	}

	Matrix4 EffectLayer::getTransMatrix()
	{
		Quaternion selfOritation;
		selfOritation.fromAxisAngle(originalDirection, selfRotation);
		Matrix4 selfRotateMatrix(selfOritation);
		Matrix4 rotateMatrix;
		rotateMatrix.fromQuan(oritation);
		Matrix4 transMatrix = Matrix4::IDENTITY;
		transMatrix.translate(position);
		Matrix4 sclaeMatrix = Matrix4::IDENTITY;
		sclaeMatrix.scale(scale, scale, scale);
		transMatrix = transMatrix * rotateMatrix * sclaeMatrix * selfRotateMatrix;
		return transMatrix;
	}

	float EffectLayer::getLayerTimeLength() const
	{
		unsigned int keyFrameTime = mKeyFrames[0]->getBeginTime();
		EffectKeyFrameConstIterator it;

		for (it = mKeyFrames.begin(); it != mKeyFrames.end(); ++it)
		{
			if ((*it)->getAffectTime() < 0)
				return -1.0f;
			keyFrameTime += (*it)->getAffectTime();
		}

		keyFrameTime += mReplayTime;
		keyFrameTime *= mReplayCount;

		return float(keyFrameTime) / 1000.0f;
	}

	void EffectLayer::setFallowLayer(EffectLayer* layer)
	{
		mpFallowLayer = layer;
	}

	EffectLayer* EffectLayer::getFallowLayer() const
	{
		return mpFallowLayer;
	}

	void EffectLayer::fallowMovement()
	{
		if (mpFallowLayer)
		{
			position = mpFallowLayer->position;
			oritation = mpFallowLayer->oritation;
			direction = mpFallowLayer->direction;
			selfRotation = mpFallowLayer->selfRotation;
		}
	}

	// 更新渲染数据到主线程
	void EffectLayer::updateRenderDataToMainThread()
	{
		m_vertexDataMainThread = mVertexData;
	}

	//播放特效挂载的音效
	void EffectLayer::playEffectSound(int index)
	{
		EffectSound* sound = mSoundMap[index];
		if (!sound || !sound->enableSound || sound->isPlaying)
		{
			return;
		}

		sound->isPlaying = true;
		sound->soundID = 0;

		float rate = Math::IntervalRandom(0.0f, 1.0f);
		if (sound->probability < rate)
			return;

		int soundFileNum = 0;
		if (!sound->randomsound1.empty())
		{
			soundFileNum++;
			if (!sound->randomsound2.empty())
			{
				soundFileNum++;
			}
		}

		if (soundFileNum == 0)
			return;

		// 随机选个声音文件播
		int randInt = (int)Math::IntervalRandom(1.0f, soundFileNum + 0.999f);

		String soundNamePlay;
		switch (randInt)
		{
		case 1:
			soundNamePlay = sound->randomsound1; break;
		case 2:
			soundNamePlay = sound->randomsound2; break;
		default:
			return;
		}

		AudioSource::Cinfo cinfo;
		{
			cinfo.m_name = soundNamePlay;
			cinfo.m_pos = getParentSystem()->getEffectSceneNode()->getWorldPosition();//Echo::Vector3::ZERO;
			cinfo.m_volume = sound->volume;
			cinfo.m_isOneShoot = !sound->loop;
			cinfo.m_maxDistance = sound->maxDistance;
			cinfo.m_minDistane = sound->minDistance;
			cinfo.m_is3DMode = sound->is3D;
		}

		sound->soundID = FSAudioManager::instance()->createAudioSource(cinfo);
		sound->isPlaying = true;
	}

	//重置特效音效的状态，主要是为了做到声音随着特效系统循环
	void EffectLayer::resetEffectSoundState()
	{
		EffectSoundMap::iterator it;
		for (it = mSoundMap.begin(); it != mSoundMap.end(); it++)
		{
			it->second->isPlaying = false;
		}
	}

	//当特效停止时候,情况音效的播放状态
	void EffectLayer::stopEffectSound()
	{
		EffectSoundMap::iterator it;
		for (it = mSoundMap.begin(); it != mSoundMap.end(); it++)
		{
			if (it->second->loop)
			{
				FSAudioManager::instance()->getAudioSource(it->second->soundID)->stop();
			}
			it->second->isPlaying = false;
		}
	}

	//销毁所有正在播放或者持有的音效
	void EffectLayer::destroyEffectSound()
	{
		EffectSoundMap::iterator it;
		for (it = mSoundMap.begin(); it != mSoundMap.end(); it++)
		{
			it->second->isPlaying = false;
			FSAudioManager::instance()->destroyAudioSources(&(it->second->soundID), 1);
		}
	}

	void EffectLayer::exportData(DataStream* pStream, int version)
	{
		if (version > 0x00010002)
		{
			i32 renderLevel = (EFFECT_RENDER_LEVEL)m_renderLevel;
			pStream->write(&renderLevel, sizeof(i32));
		}

		// property base
		i32 rg = (ui32)mRenderGroup;
		pStream->write(&rg, sizeof(i32));
		pStream->write(&mReplayCount, sizeof(i32));
		pStream->write(&mReplayTime, sizeof(i32));
		pStream->write(&mbRestateLoopEnd, sizeof(bool));
		ui32 strlength = mName.length();
		pStream->write(&strlength, sizeof(ui32));
		pStream->write(mName.c_str(), strlength);

		// material
		mMaterial->exportData(pStream, version);

		// keyframe
		ui32 keyFrameNum = mKeyFrames.size();
		pStream->write(&keyFrameNum, sizeof(ui32));
		EffectKeyFrame* pKeyFrame = NULL;
		for (size_t i = 0; i<keyFrameNum; ++i)
		{
			pKeyFrame = mKeyFrames[i];
			pKeyFrame->exportData(pStream, version);
		}
	}

	void EffectLayer::importData(DataStream* pStream, int version)
	{
		if (version>0x00010002)
		{
			i32 renderLevel = (i32)ERL_High;
			pStream->read(&renderLevel, sizeof(i32));
			m_renderLevel = (EFFECT_RENDER_LEVEL)renderLevel;
		}

		// property base
		i32 rg;
		pStream->read(&rg, sizeof(i32));
		mRenderGroup = (EFFECT_RENDER_GROUP)rg;
		pStream->read(&mReplayCount, sizeof(i32));
		pStream->read(&mReplayTime, sizeof(i32));
		pStream->read(&mbRestateLoopEnd, sizeof(bool));
		ui32 strlength;
		pStream->read(&strlength, sizeof(ui32));
		char str[128];
		pStream->read(str, strlength);
		str[strlength] = 0;
		mName = str;

		// material
		EchoAssertX(mMaterial == NULL, "Error: EffectLayer::importData error occurse in material");
		mMaterial = EffectSystemManager::instance()->createMaterial();
		mMaterial->importData(pStream, version);

		// keyframe
		EchoAssertX(mKeyFrames.empty(), "Error: EffectLayer::importData error occurse in Keyframes");
		ui32 keyFrameNum;
		pStream->read(&keyFrameNum, sizeof(ui32));
		EffectKeyFrame* pKeyFrame = NULL;
		for (ui32 i = 0; i < keyFrameNum; ++i)
		{
			pKeyFrame = createkeyFrame(i == 0 ? -1 : i);
			pKeyFrame->importData(pStream, version);
		}
	}

	void EffectLayer::getPropertyList(PropertyList & list)
	{
		list.push_back(token[TOKEN_LY_RENDERLEVEL]);
		list.push_back(token[TOKEN_LY_RENDERGROUP]);
		list.push_back(token[TOKEN_LY_REPLAYCOUNT]);
		list.push_back(token[TOKEN_LY_REPLAYTIME]);
		list.push_back(token[TOKEN_LY_RESTATELOOP]);
	}

	bool EffectLayer::getPropertyType(const String & name, PropertyType &type)
	{
		if (name == token[TOKEN_LY_RENDERLEVEL])
		{
			type = IElement::PT_RENDERLEVEL; return true;
		}
		else if (name == token[TOKEN_LY_RENDERGROUP])
		{
			type = IElement::PT_RENDERQUEUE; return true;
		}
		else if (name == token[TOKEN_LY_REPLAYCOUNT])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_REPLAYTIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_LY_RESTATELOOP])
		{
			type = IElement::PT_BOOL; return true;
		}

		return false;
	}

	bool EffectLayer::getPropertyValue(const String & name, String &value)
	{
		if (name == token[TOKEN_LY_RENDERLEVEL])
		{
			value = StringUtil::ToString(m_renderLevel); return true;
		}
		else if (name == token[TOKEN_LY_RENDERGROUP])
		{
			value = StringUtil::ToString(mRenderGroup); return true;
		}
		else if (name == token[TOKEN_LY_REPLAYCOUNT])
		{
			value = StringUtil::ToString(mReplayCount); return true;
		}
		else if (name == token[TOKEN_LY_REPLAYTIME])
		{
			value = StringUtil::ToString(mReplayTime); return true;
		}
		else if (name == token[TOKEN_LY_RESTATELOOP])
		{
			value = StringUtil::ToString(mbRestateLoopEnd); return true;
		}

		return false;
	}

	bool EffectLayer::setPropertyValue(const String & name, const String & value)
	{
		if (name == token[TOKEN_LY_RENDERLEVEL])
		{
			m_renderLevel = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_RENDERGROUP])
		{
			mRenderGroup = (EFFECT_RENDER_GROUP)StringUtil::ParseInt(value);
			return true;
		}
		else if (name == token[TOKEN_LY_REPLAYCOUNT])
		{
			mReplayCount = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_REPLAYTIME])
		{
			mReplayTime = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_LY_RESTATELOOP])
		{
			mbRestateLoopEnd = StringUtil::ParseBool(value); return true;
		}

		return false;
	}

	std::vector<std::pair<String, String> >	EffectLayer::getAllProperties()
	{
		PropertyList list;
		std::vector<std::pair<String, String> > res;
		//材质属性
		EffectMaterial *pMaterial = getMaterial();
		if (pMaterial)
		{
			pMaterial->getPropertyList(list);
			for (PropertyList::iterator it = list.begin(); it != list.end(); it++)
			{
				String str;
				pMaterial->getPropertyValue(*it, str);
				res.push_back(std::make_pair(*it, str));
			}
		}
		//图层属性
		list.clear();
		getPropertyList(list);
		for (PropertyList::iterator it = list.begin(); it != list.end(); it++)
		{
			String str;
			getPropertyValue(*it, str);
			res.push_back(std::make_pair(*it, str));
		}
		return res;
	}
}