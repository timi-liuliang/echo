#include "EffectSystemManager.h"
#include "EffectKeyFrame.h"
#include "EffectLayer.h" 
#include "Engine/core/Geom/SimpleSpline.h"
#include "engine/core/resource/DataStream.h"
#include "engine/modules/Audio/FMODStudio/FSAudioManager.h"

namespace Echo
{

	EffectKeyFrame::EffectKeyFrame()
		:mPosition(0, 0, 0)
		, mOritation(1, 0, 0, 0)
		, mSelfRotate(0)
		, mColor(1, 1, 1, 1)
		, mScale(1)
		, mBeginTime(0)
		, mAffectTime(2000)
		, mInterpolationType(IT_CONTROLLER)
		, mAutoOritationForSpline(false)
		, mSpline(NULL)
		, mSplineOldPos(Vector3::ZERO)
		, mPitch(0)
		, mYaw(0)
		, mRoll(0)
		, mParentLayer(0)
		, mNextKeyFrame(NULL)
		, mCurrentTime(0)
	{
		mEffectSound = EchoNew(EffectSound);
		{
			mEffectSound->enableSound = false;
			mEffectSound->randomsound1 = "";
			mEffectSound->randomsound2 = "";
			mEffectSound->volume = 1;
			mEffectSound->probability = 1;
			mEffectSound->loop = false;
			mEffectSound->is3D = false;
			mEffectSound->minDistance = 10;
			mEffectSound->maxDistance = 50;
			mEffectSound->soundID = 0;
			mEffectSound->isPlaying = false;
		};

	}

	EffectKeyFrame::~EffectKeyFrame()
	{
		destroyAllControllers();
		destroyEffectSound();

		EchoSafeDelete(mSpline, SimpleSpline);
	}

	void EffectKeyFrame::_notifyStart(EffectKeyFrame* nextFrame)
	{
		mCurrentTime = 0;

		mParentLayer->originalPosition = mParentLayer->position = mPosition;
		mParentLayer->originalScale = mParentLayer->scale = 1.0f;
		mParentLayer->originalColor = mParentLayer->color = mColor;
		mParentLayer->originalScale = mParentLayer->scale = mScale;
		mParentLayer->originalOritation = mParentLayer->oritation = mOritation;
		mParentLayer->direction = mOritation * mParentLayer->originalDirection;
		mParentLayer->originalSelfRotation = mParentLayer->selfRotation = mSelfRotate;

		mParentLayer->mTime = 0;
		if (mSpline && mInterpolationType == IT_SPLINE)
		{
			mSplineOldPos = mSpline->getPoint(0);
			Vector3 prePos = mSpline->interpolate(0.01f);
			prePos -= mSplineOldPos;
			prePos.normalize();
			mParentLayer->originalDirection = mParentLayer->direction = prePos;
		}

		for (size_t i = 0; i<mControllers.size(); ++i)
		{
			mControllers[i]->_notifyStart(mParentLayer);
		}

		mNextKeyFrame = nextFrame;
	}

	void EffectKeyFrame::_notifyContinue(EffectKeyFrame* nextFrame)
	{
		mCurrentTime = 0;

		mParentLayer->_notifyKeyframeEnd();

		if (mInterpolationType == IT_CONTROLLER)
		{
			mParentLayer->forwardParticleState();
			mOritation = mParentLayer->oritation;
			mColor = mParentLayer->color;
			mPosition = mParentLayer->position;
			mScale = mParentLayer->scale;
			mSelfRotate = mParentLayer->selfRotation;
			for (size_t i = 0; i<mControllers.size(); ++i)
			{
				mControllers[i]->_notifyStart(mParentLayer);
			}
		}

		mNextKeyFrame = nextFrame;
	}

	i32 EffectKeyFrame::_update_sys(i32 time)
	{
		mCurrentTime += time;
		i32 remainedTime = 0;
		if (mAffectTime>0 && mCurrentTime>mAffectTime)
		{
			remainedTime = mCurrentTime - mAffectTime;
			mCurrentTime = mAffectTime;
		}

		mParentLayer->reParticleState();

		if (mInterpolationType == IT_CONTROLLER)
		{
			for (size_t i = 0; i<mControllers.size(); ++i)
			{
				if (mParentLayer->getFallowLayer() != NULL)
				{
					if (!mControllers[i]->isControllerMovement())
						mControllers[i]->controll(mCurrentTime, mParentLayer);
				}
				else
					mControllers[i]->controll(mCurrentTime, mParentLayer);
			}

			if (mParentLayer->getFallowLayer() != NULL)
				mParentLayer->fallowMovement();
		}
		else if (mInterpolationType == IT_SPLINE)
		{
			for (size_t i = 0; i<mControllers.size(); ++i)
			{
				if (!mControllers[i]->isControllerMovement())
					mControllers[i]->controll(mCurrentTime, mParentLayer);
			}
			if (mSpline)
			{
				float interpolateRate = float(mCurrentTime) / float(mAffectTime);
				if (interpolateRate < 0.0f)
					interpolateRate = 0.0f;

				Vector3 pos = mSpline->interpolate(interpolateRate);
				mParentLayer->position = pos;
				Vector3 dir = (pos - mSplineOldPos).normalizedCopy();
				mParentLayer->direction = dir;
				mSplineOldPos = pos;

				if (mAutoOritationForSpline)
				{
					Vector3 dirXZ = Vector3(dir.x, 0, dir.z);

					Quaternion ori, ori2, oriInverse;
					ori = Vector3::UNIT_Z.getRotationTo(dirXZ);

					oriInverse = ori;
					oriInverse.inverse();
					dirXZ = oriInverse*dirXZ;
					dir = oriInverse*dir;
					ori2 = Vector3::UNIT_Z.getRotationTo(dir);

					mParentLayer->oritation = ori*ori2;
				}
			}
		}
		else
		{
			for (size_t i = 0; i<mControllers.size(); ++i)
			{
				if (!mControllers[i]->isControllerMovement())
					mControllers[i]->controll(mCurrentTime, mParentLayer);
			}

			if (mNextKeyFrame)
			{
				const Vector3& pos1 = mPosition;
				float rot = mSelfRotate;
				const Quaternion& ori1 = mOritation;
				const Color& color1 = mColor;
				float sc1 = mScale;
				const Vector3& pos2 = mNextKeyFrame->getPosition();
				const Quaternion& ori2 = mNextKeyFrame->getOritation();
				float rot2 = mNextKeyFrame->getSelfRotate();
				const Color& color2 = mNextKeyFrame->getColor();
				float sc2 = mNextKeyFrame->getScale();

				float interpolateRate = float(mCurrentTime) / float(mAffectTime);
				mParentLayer->position = pos1 + (pos2 - pos1) * interpolateRate;
				Quaternion::Slerp(mParentLayer->oritation, ori1, ori2, interpolateRate);
				mParentLayer->selfRotation = rot + (rot2 - rot)*interpolateRate;
				mParentLayer->color = color1 + (color2 - color1)*interpolateRate;
				mParentLayer->scale = sc1 + (sc2 - sc1)*interpolateRate;
			}
		}

		return remainedTime;
	}

	String EffectKeyFrame::getInterpolationTypeString() const
	{
		String typeString = "Controller";
		if (mInterpolationType == IT_SPLINE)
			typeString = "Spline";
		else if (mInterpolationType == IT_SLERP)
			typeString = "Slerp";

		return typeString;
	}

	void EffectKeyFrame::copyAttributesTo(EffectKeyFrame* keyFrame)
	{
		keyFrame->destroyAllControllers();

		keyFrame->mPosition = mPosition;
		keyFrame->mSelfRotate = mSelfRotate;
		keyFrame->mScale = mScale;
		keyFrame->mOritation = mOritation;
		keyFrame->mPitch = mPitch;
		keyFrame->mYaw = mYaw;
		keyFrame->mRoll = mRoll;
		keyFrame->mColor = mColor;
		keyFrame->mBeginTime = mBeginTime;
		keyFrame->mAffectTime = mAffectTime;
		keyFrame->mInterpolationType = mInterpolationType;
		keyFrame->mAutoOritationForSpline = mAutoOritationForSpline;

		keyFrame->mEffectSound->enableSound = mEffectSound->enableSound;
		keyFrame->mEffectSound->randomsound1 = mEffectSound->randomsound1;
		keyFrame->mEffectSound->randomsound2 = mEffectSound->randomsound2;
		keyFrame->mEffectSound->volume = mEffectSound->volume;
		keyFrame->mEffectSound->probability = mEffectSound->probability;
		keyFrame->mEffectSound->is3D = mEffectSound->is3D;
		keyFrame->mEffectSound->loop = mEffectSound->loop;
		keyFrame->mEffectSound->minDistance = mEffectSound->minDistance;
		keyFrame->mEffectSound->maxDistance = mEffectSound->maxDistance;

		EffectController* cloneController = 0;
		for (size_t i = 0; i<mControllers.size(); ++i)
		{
			cloneController = EffectSystemManager::instance()->cloneController(getController(i));
			keyFrame->addController(cloneController);
		}

		if (mSpline != NULL)
		{
			keyFrame->mSpline = EchoNew(SimpleSpline);
			for (size_t i = 0; i<mSpline->getNumPoints(); ++i)
			{
				keyFrame->mSpline->addPoint(mSpline->getPoint(i));
			}
		}
	}

	EffectController* EffectKeyFrame::createController(EFFECT_CONTROL_TYPE controlType)
	{
		EffectController* controller = EffectSystemManager::instance()->createController(controlType);
		addController(controller);
		return controller;
	}

	void EffectKeyFrame::addController(EffectController* controller)
	{
		assert(controller && "Controller is null!");
		mControllers.push_back(controller);
		EffectController* swapTemp;
		for (size_t i = mControllers.size() - 1; i>0; --i)
		{
			if (mControllers[i]->getPriority() < mControllers[i - 1]->getPriority())
			{
				swapTemp = mControllers[i];
				mControllers[i] = mControllers[i - 1];
				mControllers[i - 1] = swapTemp;
			}
			else
			{
				break;
			}
		}
	}

	void EffectKeyFrame::removeController(EffectController* controller)
	{
		EchoAssert(controller && "Controller is null!");
		EffectControllerItor it;
		EffectControllerItor itEnd = mControllers.end();
		for (it = mControllers.begin(); it != itEnd; ++it)
		{
			if (*it == controller)
			{
				// Remove it
				mControllers.erase(it);
				break;
			}
		}
	}

	EffectController* EffectKeyFrame::getController(size_t index) const
	{
		assert(index < mControllers.size() && "Controller index out of bounds!");
		return mControllers[index];
	}

	size_t EffectKeyFrame::getNumControllers() const
	{
		return mControllers.size();
	}

	void EffectKeyFrame::destroyController(size_t index)
	{
		destroyController(getController(index));
	}

	void EffectKeyFrame::destroyController(EffectController* controller)
	{
		EchoAssert(controller && "Controller is null!");
		EffectControllerItor it;
		EffectControllerItor itEnd = mControllers.end();
		for (it = mControllers.begin(); it != itEnd; ++it)
		{
			if (*it == controller)
			{
				EffectSystemManager::instance()->destroyController(*it);
				mControllers.erase(it);
				break;
			}
		}
	}

	void EffectKeyFrame::destroyAllControllers()
	{
		EffectControllerItor it;
		EffectControllerItor itEnd = mControllers.end();
		for (it = mControllers.begin(); it != itEnd; ++it)
		{
			EffectSystemManager::instance()->destroyController(*it);
		}
		mControllers.clear();
	}

	//音效随着关键帧销毁而销毁
	void EffectKeyFrame::destroyEffectSound()
	{
		FSAudioManager::instance()->destroyAudioSources(&mEffectSound->soundID, 1);
		EchoSafeDelete(mEffectSound, EffectSound);
	}

	void EffectKeyFrame::mirrorXy()
	{
		mPosition.z = -mPosition.z;
	}

	void EffectKeyFrame::mirrorYz()
	{
		mPosition.x = -mPosition.x;
	}

	void EffectKeyFrame::mirrorXz()
	{
		mPosition.y = -mPosition.y;
	}

	void EffectKeyFrame::reCaculateOritation()
	{
		mOritation.fromEulerAngle(mPitch, mYaw, mRoll);
	}

	void EffectKeyFrame::reCaculateAngle()
	{
		mOritation.toEulerAngle(mPitch, mYaw, mRoll);
	}


	void EffectKeyFrame::createSpline()
	{
		assert(mSpline == NULL);
		mSpline = EchoNew(SimpleSpline);
	}

	SimpleSpline* EffectKeyFrame::getSplinePtr() const
	{
		return mSpline;
	}

	void EffectKeyFrame::destroySpline()
	{
		EchoSafeDelete(mSpline, SimpleSpline);
	}

	void EffectKeyFrame::setSpline(const String& str)
	{
		String trackPoints;
		trackPoints = StringUtil::Replace(str, "_", " ");
		StringArray points = StringUtil::Split(trackPoints, "|");

		if (points.size())
		{
			EchoSafeDelete(mSpline, SimpleSpline);
			mSpline = EchoNew(SimpleSpline);

			for (int indexPoint = 0; indexPoint < (int)points.size(); indexPoint++)
			{
				mSpline->addPoint(StringUtil::ParseVec3(points[indexPoint]));
			}
		}
	}

	String EffectKeyFrame::getSpline() const
	{
		if (mSpline == NULL)
			return StringUtil::BLANK;

		String result = StringUtil::BLANK;

		for (int index = 0; index < mSpline->getNumPoints(); index++)
		{
			String strPoint = StringUtil::BLANK;
			if (index != 0)
				strPoint = "|";

			Vector3 point = mSpline->getPoint(index);

			strPoint +=
				StringUtil::ToString(point.x) + "_" +
				StringUtil::ToString(point.y) + "_" +
				StringUtil::ToString(point.z);

			result += strPoint;
		}

		return result;
	}

	void EffectKeyFrame::exportData(DataStream* pStream, int version)
	{
		/// base property
		pStream->write(&mPosition, sizeof(Vector3));
		pStream->write(&mSelfRotate, sizeof(float));
		pStream->write(&mScale, sizeof(float));
		pStream->write(&mOritation, sizeof(Quaternion));
		pStream->write(&mColor, sizeof(Color));
		pStream->write(&mBeginTime, sizeof(i32));
		pStream->write(&mAffectTime, sizeof(i32));
		pStream->write(&mInterpolationType, sizeof(INTERPOLATION_TYPE));
		pStream->write(&mAutoOritationForSpline, sizeof(bool));

		//音效部分
		if (version > 0x00010006)
		{
			pStream->write(&mEffectSound->enableSound, sizeof(bool));

			ui32 strlength = mEffectSound->randomsound1.length();
			pStream->write(&strlength, sizeof(ui32));
			pStream->write(mEffectSound->randomsound1.c_str(), strlength);

			strlength = mEffectSound->randomsound2.length();
			pStream->write(&strlength, sizeof(ui32));
			pStream->write(mEffectSound->randomsound2.c_str(), strlength);

			pStream->write(&mEffectSound->volume, sizeof(float));
			pStream->write(&mEffectSound->probability, sizeof(float));
			pStream->write(&mEffectSound->loop, sizeof(bool));
			pStream->write(&mEffectSound->is3D, sizeof(bool));
			pStream->write(&mEffectSound->minDistance, sizeof(float));
			pStream->write(&mEffectSound->maxDistance, sizeof(float));
		}
		// spline
		ui32 splinePointNum = 0;
		if (mSpline)
		{
			splinePointNum = mSpline->getNumPoints();
			pStream->write(&splinePointNum, sizeof(ui32));
			for (size_t i = 0; i<splinePointNum; ++i)
			{
				Vector3 splinePoint = mSpline->getPoint(i);
				pStream->write(&splinePoint, sizeof(Vector3));
			}
		}
		else
		{
			pStream->write(&splinePointNum, sizeof(ui32));
		}

		// controllers
		ui32 controllerNum = mControllers.size();
		pStream->write(&controllerNum, sizeof(ui32));
		EFFECT_CONTROL_TYPE type;
		EffectController* pController = NULL;
		for (size_t j = 0; j<controllerNum; ++j)
		{
			pController = mControllers[j];
			type = pController->getControllerType();
			pStream->write(&type, sizeof(EFFECT_CONTROL_TYPE));
			pController->exportData(pStream, version);
		}
	}

	void EffectKeyFrame::importData(DataStream* pStream, int version)
	{
		/// base property
		pStream->read(&mPosition, sizeof(Vector3));
		pStream->read(&mSelfRotate, sizeof(float));
		pStream->read(&mScale, sizeof(float));
		pStream->read(&mOritation, sizeof(Quaternion));
		pStream->read(&mColor, sizeof(Color));
		pStream->read(&mBeginTime, sizeof(i32));
		pStream->read(&mAffectTime, sizeof(i32));
		pStream->read(&mInterpolationType, sizeof(INTERPOLATION_TYPE));
		pStream->read(&mAutoOritationForSpline, sizeof(bool));

		//音效部分
		if (version > 0x00010006)
		{
			pStream->read(&(mEffectSound->enableSound), sizeof(bool));

			ui32 strLength;
			char str[128];
			pStream->read(&strLength, sizeof(ui32));
			pStream->read(str, sizeof(char)*strLength);
			str[strLength] = 0;
			mEffectSound->randomsound1 = str;

			pStream->read(&strLength, sizeof(ui32));
			pStream->read(str, sizeof(char)*strLength);
			str[strLength] = 0;
			mEffectSound->randomsound2 = str;

			pStream->read(&(mEffectSound->volume), sizeof(float));
			pStream->read(&(mEffectSound->probability), sizeof(float));
			pStream->read(&(mEffectSound->loop), sizeof(bool));
			pStream->read(&(mEffectSound->is3D), sizeof(bool));
			pStream->read(&(mEffectSound->minDistance), sizeof(float));
			pStream->read(&(mEffectSound->maxDistance), sizeof(float));
		}

		// spline
		EchoAssertX(mSpline == NULL, "Error: EffectKeyFrame::importData error occurse in spline.");
		ui32 splinePointNum = 0;
		pStream->read(&splinePointNum, sizeof(ui32));
		if (splinePointNum > 0)
		{
			createSpline();
			Vector3 splinePoint;
			for (ui32 i = 0; i<splinePointNum; ++i)
			{
				pStream->read(&splinePoint, sizeof(Vector3));
				mSpline->addPoint(splinePoint);
			}
		}

		// controllers
		EchoAssertX(mControllers.empty(), "Error: EffectKeyFrame::importData error occurse in controllers.");
		ui32 controllerNum;
		pStream->read(&controllerNum, sizeof(ui32));
		EFFECT_CONTROL_TYPE type;
		EffectController* pController = NULL;
		for (ui32 j = 0; j<controllerNum; ++j)
		{
			pStream->read(&type, sizeof(EFFECT_CONTROL_TYPE));
			pController = createController(type);
			pController->importData(pStream, version);
		}
	}

	void EffectKeyFrame::getPropertyList(PropertyList & list)
	{
		list.push_back(token[TOKEN_KF_POSITION]);
		list.push_back(token[TOKEN_KF_YAW]);
		list.push_back(token[TOKEN_KF_PITCH]);
		list.push_back(token[TOKEN_KF_ROLL]);
		list.push_back(token[TOKEN_KF_SELFROTATE]);
		list.push_back(token[TOKEN_KF_SCALE]);
		list.push_back(token[TOKEN_KF_COLOR]);
		list.push_back(token[TOKEN_KF_AFFECTTIME]);
		list.push_back(token[TOKEN_KF_INTERPOLATION]);
		getAudioPropertyList(list);
	}

	void EffectKeyFrame::getAudioPropertyList(PropertyList & list)
	{
		list.push_back(token[TOKEN_KF_AUDIO_ENABLE]);
		list.push_back(token[TOKEN_KF_AUDIO_RANDOMSOUND1]);
		list.push_back(token[TOKEN_KF_AUDIO_RANDOMSOUND2]);
		list.push_back(token[TOKEN_KF_AUDIO_VOLUME]);
		list.push_back(token[TOKEN_KF_AUDIO_PROBABILITY]);
		list.push_back(token[TOKEN_KF_AUDIO_LOOP]);
		list.push_back(token[TOKEN_KF_AUDIO_3DMODE]);
		list.push_back(token[TOKEN_KF_AUDIO_MINDISTANCE]);
		list.push_back(token[TOKEN_KF_AUDIO_MAXDISTANCE]);
	}

	bool EffectKeyFrame::getPropertyType(const String & name, PropertyType &type)
	{
		if (name == token[TOKEN_KF_POSITION])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if (name == token[TOKEN_KF_PITCH])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_KF_YAW])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_KF_ROLL])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_KF_SELFROTATE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_KF_SCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_KF_COLOR])
		{
			type = IElement::PT_COLOR; return true;
		}
		/*else if (name == token[TOKEN_KF_BEGINTIME])
		{
		type = IElement::PT_INT; return true;
		}*/ // 该属性由编辑器KeyFrameUI控制
		else if (name == token[TOKEN_KF_AFFECTTIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_KF_INTERPOLATION])
		{
			type = IElement::PT_KEYFRAME_SLERP_MODE; return true;
		}
		//音效部分
		else if (name == token[TOKEN_KF_AUDIO_ENABLE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND1])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND2])
		{
			type = IElement::PT_STRING; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_VOLUME])
		{
			type = IElement::PT_FLOAT; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_PROBABILITY])
		{
			type = IElement::PT_FLOAT; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_LOOP])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_3DMODE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MINDISTANCE])
		{
			type = IElement::PT_FLOAT; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MAXDISTANCE])
		{
			type = IElement::PT_FLOAT; return true;
		}
		return false;
	}

	bool EffectKeyFrame::getPropertyValue(const String & name, String &value)
	{
		if (name == token[TOKEN_KF_POSITION])
		{
			value = StringUtil::ToString(mPosition); return true;
		}
		else if (name == token[TOKEN_KF_PITCH])
		{
			reCaculateAngle();
			value = StringUtil::ToString(mPitch); return true;
		}
		else if (name == token[TOKEN_KF_YAW])
		{
			reCaculateAngle();
			value = StringUtil::ToString(mYaw); return true;
		}
		else if (name == token[TOKEN_KF_ROLL])
		{
			reCaculateAngle();
			value = StringUtil::ToString(mRoll); return true;
		}
		else if (name == token[TOKEN_KF_SELFROTATE])
		{
			value = StringUtil::ToString(mSelfRotate); return true;
		}
		else if (name == token[TOKEN_KF_SCALE])
		{
			value = StringUtil::ToString(mScale); return true;
		}
		else if (name == token[TOKEN_KF_COLOR])
		{
			 value = StringUtil::ToString(mColor); return true;
		}
		/*else if (name == token[TOKEN_KF_BEGINTIME])
		{
		value = StringUtil::ToString(mBeginTime); return true;
		}*/// 该属性由编辑器KeyFrameUI控制
		else if (name == token[TOKEN_KF_AFFECTTIME])
		{
			value = StringUtil::ToString(mAffectTime); return true;
		}
		else if (name == token[TOKEN_KF_INTERPOLATION])
		{
			value = getInterpolationTypeString(); return true;
		}

		//音效部分
		else if (name == token[TOKEN_KF_AUDIO_ENABLE])
		{
			value = StringUtil::ToString(mEffectSound->enableSound); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND1])
		{
			value = mEffectSound->randomsound1; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND2])
		{
			value = mEffectSound->randomsound2; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_VOLUME])
		{
			value = StringUtil::ToString(mEffectSound->volume); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_PROBABILITY])
		{
			value = StringUtil::ToString(mEffectSound->probability); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_LOOP])
		{
			value = StringUtil::ToString(mEffectSound->loop); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_3DMODE])
		{
			value = StringUtil::ToString(mEffectSound->is3D); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MINDISTANCE])
		{
			value = StringUtil::ToString(mEffectSound->minDistance); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MAXDISTANCE])
		{
			value = StringUtil::ToString(mEffectSound->maxDistance); return true;
		}

		return false;
	}

	bool EffectKeyFrame::setPropertyValue(const String & name, const String & value)
	{
		if (name == token[TOKEN_KF_POSITION])
		{
			mPosition = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_KF_PITCH])
		{
			mPitch = StringUtil::ParseReal(value); 
			reCaculateOritation();
			return true;
		}
		else if (name == token[TOKEN_KF_YAW])
		{
			mYaw = StringUtil::ParseReal(value);
			reCaculateOritation();
			return true;
		}
		else if (name == token[TOKEN_KF_ROLL])
		{
			mRoll = StringUtil::ParseReal(value);
			reCaculateOritation();
			return true;
		}
		else if (name == token[TOKEN_KF_SELFROTATE])
		{
			mSelfRotate = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_KF_SCALE])
		{
			mScale = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_KF_COLOR])
		{
			mColor = StringUtil::ParseColor(value); return true;
		}
		/*else if (name == token[TOKEN_KF_BEGINTIME])
		{
		mBeginTime = Ogre::StringConverter::parseInt(value); return true;
		}*/
		else if (name == token[TOKEN_KF_AFFECTTIME])
		{
			mAffectTime = StringUtil::ParseInt(value); return true;
		}

		//音效部分
		else if (name == token[TOKEN_KF_AUDIO_ENABLE])
		{
			mEffectSound->enableSound = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND1])
		{
			mEffectSound->randomsound1 = value; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_RANDOMSOUND2])
		{
			mEffectSound->randomsound2 = value; return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_VOLUME])
		{
			mEffectSound->volume = StringUtil::ParseFloat(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_PROBABILITY])
		{
			mEffectSound->probability = StringUtil::ParseFloat(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_LOOP])
		{
			mEffectSound->loop = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_3DMODE])
		{
			mEffectSound->loop = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MINDISTANCE])
		{
			mEffectSound->minDistance = StringUtil::ParseFloat(value); return true;
		}
		else if (name == token[TOKEN_KF_AUDIO_MAXDISTANCE])
		{
			mEffectSound->maxDistance = StringUtil::ParseFloat(value); return true;
		}
		else if (name == token[TOKEN_KF_INTERPOLATION])
		{
			mInterpolationType = IT_CONTROLLER;
			if (value == "Spline")
				mInterpolationType = IT_SPLINE;
			else if (value == "Slerp")
				mInterpolationType = IT_SLERP;
			return true;
		}
		return false;
	}

	// 获取所有属性
	std::vector<std::pair<String, String> >	EffectKeyFrame::getAllProperties()
	{
		PropertyList list;
		std::vector<std::pair<String, String> > res;
		//关键帧属性
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
