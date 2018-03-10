#include "EffectController.h"
#include "engine/core/Util/StringUtil.h"
#include "EffectParticle.h"
#include "EffectLayer.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{

	const i32 EffectController::DEFAULT_BEGINTIME = 0;
	const i32 EffectController::DEFAULT_ENDTIME = -1;

	EffectController::EffectController()
		:mBeginTime(DEFAULT_BEGINTIME)
		,mEndTime(DEFAULT_ENDTIME)
		,mInController(false)
	{

	}

	EffectController::~EffectController()
	{

	}

	void EffectController::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		if(bParticleSet)
		{
			if(particle->mTime > mBeginTime && particle->mInController == false)
			{
				particle->forwardParticleState2();
				particle->mInController = true;
				//mInController = true;
			}
		}
		else
		{
			particle->mTime = time;
			if(particle->mTime > mBeginTime && mInController == false)
			{
				//particle->forwardParticleState2();
				mInController = true;
			}
		}
	}

	void EffectController::_notifyStart(EffectParticle* particle)
	{
		if(particle)
		{
			particle->mTime = 0;
			particle->forwardParticleState();
			mInController = false;
			particle->mInController = false;
		}
	}

	void EffectController::copyAttributesTo(EffectController* controller)
	{
		controller->mBeginTime = mBeginTime;
		controller->mEndTime = mEndTime;
	}

	void EffectController::exportData(DataStream* pStream, int version)
	{
		pStream->write(&mBeginTime, sizeof(i32));
		pStream->write(&mEndTime, sizeof(i32));
	}

	void EffectController::importData(DataStream* pStream, int version)
	{
		pStream->read(&mBeginTime, sizeof(i32));
		pStream->read(&mEndTime, sizeof(i32));
	}

	void EffectController::getPropertyList(PropertyList & list)
	{
		list.push_back(token[TOKEN_CL_BEGINTIME]);
		list.push_back(token[TOKEN_CL_ENDTIME]);
	}

	bool EffectController::getPropertyType(const String & name, PropertyType &type)
	{
		if (name == token[TOKEN_CL_BEGINTIME])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_CL_ENDTIME])
		{
			type = IElement::PT_INT; return true;
		}
		return false;
	}

	bool EffectController::getPropertyValue(const String & name, String &value)
	{
		if (name == token[TOKEN_CL_BEGINTIME])
		{
			value = StringUtil::ToString(mBeginTime); return true;
		}
		else if (name == token[TOKEN_CL_ENDTIME])
		{
			value = StringUtil::ToString(mEndTime); return true;
		}
		return false;
	}

	bool EffectController::setPropertyValue(const String & name,const String & value)
	{
		if (name == token[TOKEN_CL_BEGINTIME])
		{
			mBeginTime = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_ENDTIME])
		{
			mEndTime = StringUtil::ParseInt(value); return true;
		}
		return false;
	}


	/************************************************************************/
	/* EffectTranslation                                                 */
	/************************************************************************/
	const Vector3 EffectTranslation::DEFAULT_DIRECTION(0, 0, 0);
	const float EffectTranslation::DEFAULT_SPEED = 0.0f;
	const float EffectTranslation::DEFAULT_ADDSPEED = 0.0f;

	EffectTranslation::EffectTranslation()
		: mDirection(DEFAULT_DIRECTION)
		, mSpeed(DEFAULT_SPEED)
		, mAddSpeed(DEFAULT_ADDSPEED)
	{
		mType = ECT_Translation;
		mPriority = 11;
	}

	EffectTranslation::~EffectTranslation()
	{

	}

	void EffectTranslation::_notifyStart(EffectParticle* particle)
	{
		EffectController::_notifyStart(particle);
		mDirection.normalize();
	}

	void EffectTranslation::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float realTime = (float)(affectTime-mBeginTime)/1000.0f;

				float ds = mSpeed*realTime + 0.5f*mAddSpeed*realTime*realTime;
				particle->position += mDirection * ds;
			}
		}

		return;
	}

	void EffectTranslation::copyAttributesTo(EffectController *controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectTranslation* pTranslation = static_cast<EffectTranslation* >(controller);
		pTranslation->mDirection = mDirection;
		pTranslation->mSpeed = mSpeed;
		pTranslation->mAddSpeed = mAddSpeed;
	}

	void EffectTranslation::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mDirection, sizeof(Vector3));
		pStream->write(&mSpeed, sizeof(float));
		pStream->write(&mAddSpeed, sizeof(float));
	}

	void EffectTranslation::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mDirection, sizeof(Vector3));
		pStream->read(&mSpeed, sizeof(float));
		pStream->read(&mAddSpeed, sizeof(float));
	}

	void EffectTranslation::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_TS_DIRECTION]);
		list.push_back(token[TOKEN_CL_TS_SPEED]);
		list.push_back(token[TOKEN_CL_TS_ADDSPEED]);
	}

	bool EffectTranslation::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_TS_DIRECTION])
		{
			type = IElement::PT_DIRECTION; return true;
		}
		else if(name == token[TOKEN_CL_TS_SPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_TS_ADDSPEED])
		{
			type = IElement::PT_REAL; return true;
		}

		return false;
	}

	bool EffectTranslation::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_TS_DIRECTION])
		{
			value = StringUtil::ToString(mDirection); return true;
		}
		else if (name == token[TOKEN_CL_TS_SPEED])
		{
			value = StringUtil::ToString(mSpeed); return true;
		}
		else if (name == token[TOKEN_CL_TS_ADDSPEED])
		{
			value = StringUtil::ToString(mAddSpeed); return true;
		}
		return false;
	}

	bool EffectTranslation::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_TS_DIRECTION])
		{
			mDirection = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_TS_SPEED])
		{
			mSpeed = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_TS_ADDSPEED])
		{
			mAddSpeed = StringUtil::ParseReal(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectRotationSelf                                                */
	/************************************************************************/
	const i32 EffectRotationSelf::DEFAULT_ROTATIONSPEED = 0;
	const i32 EffectRotationSelf::DEFAULT_ROTATIONADDSPEED = 0;

	EffectRotationSelf::EffectRotationSelf()
		:mSpeed(DEFAULT_ROTATIONSPEED)
		,mAddSpeed(DEFAULT_ROTATIONADDSPEED)
	{
		mType = ECT_RotateSelf;
		mPriority = 4;
	}

	EffectRotationSelf::~EffectRotationSelf()
	{

	}

	void EffectRotationSelf::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float realTime = (float)(affectTime-mBeginTime)/1000.0f;

				float dtangle = mSpeed*realTime + 0.5f*mAddSpeed*realTime*realTime;

				particle->selfRotation = particle->originalSelfRotation + dtangle;
			}
		}

		return;
	}

	void EffectRotationSelf::copyAttributesTo(EffectController *controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectRotationSelf* pRotationSelf = static_cast<EffectRotationSelf* >(controller);
		pRotationSelf->mSpeed = mSpeed;
		pRotationSelf->mAddSpeed = mAddSpeed;
	}

	void EffectRotationSelf::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mSpeed, sizeof(float));
		pStream->write(&mAddSpeed, sizeof(float));
	}

	void EffectRotationSelf::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mSpeed, sizeof(float));
		pStream->read(&mAddSpeed, sizeof(float));
	}

	void EffectRotationSelf::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_RS_SPEED]);
		list.push_back(token[TOKEN_CL_RS_ADDSPEED]);
	}

	bool EffectRotationSelf::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_RS_SPEED])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_RS_ADDSPEED])
		{
			type = IElement::PT_INT; return true;
		}

		return false;
	}

	bool EffectRotationSelf::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RS_SPEED])
		{
			value = StringUtil::ToString(mSpeed); return true;
		}
		else if (name == token[TOKEN_CL_RS_ADDSPEED])
		{
			value = StringUtil::ToString(mAddSpeed); return true;
		}

		return false;
	}

	bool EffectRotationSelf::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RS_SPEED])
		{
			mSpeed = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_RS_ADDSPEED])
		{
			mAddSpeed = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectRotationAxis                                                */
	/************************************************************************/
	const Vector3 EffectRotationAxis::DEFAULT_AXIS(0, 0, 0);
	const Vector3 EffectRotationAxis::DEFAULT_DIRECTION(0, 1, 0);
	const i32	EffectRotationAxis::DEFAULT_ROTATIONSPEED = 0;
	const i32	EffectRotationAxis::DEFAULT_ROTATIONADDSPEED = 0;

	EffectRotationAxis::EffectRotationAxis()
		:mAxisPosition(DEFAULT_AXIS)
		,mAxisDirection(DEFAULT_DIRECTION)
		,mSpeed(DEFAULT_ROTATIONSPEED)
		,mAddSpeed(DEFAULT_ROTATIONADDSPEED)
	{
		mType = ECT_RotateAxis;
		mPriority = 2;
	}

	EffectRotationAxis::~EffectRotationAxis()
	{

	}

	void EffectRotationAxis::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float realTime = (float)(affectTime-mBeginTime)/1000.0f;

				float deltaAngle = mSpeed*realTime+0.5f*mAddSpeed*realTime*realTime;

				// pos and ori
				Vector3 transVec = particle->position - mAxisPosition;
				Quaternion quaternion;
				quaternion.fromAxisAngle(mAxisDirection, deltaAngle*Math::DEG2RAD);
				Vector3 rotateVec = quaternion*transVec;
				particle->position = rotateVec + mAxisPosition; 

				particle->oritation = quaternion * particle->originalOritation;
				// cannot change the particle's direction.
				//particle->direction = particle->oritation * particle->originalDirection;
			}
		}

		return;
	}

	void EffectRotationAxis::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectRotationAxis* pRotationAxis = static_cast<EffectRotationAxis*>(controller);
		pRotationAxis->mAxisPosition = mAxisPosition;
		pRotationAxis->mAxisDirection = mAxisDirection;
		pRotationAxis->mSpeed = mSpeed;
		pRotationAxis->mAddSpeed = mAddSpeed;
	}

	void EffectRotationAxis::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mAxisPosition, sizeof(Vector3));
		pStream->write(&mAxisDirection, sizeof(Vector3));
		pStream->write(&mSpeed, sizeof(float));
		pStream->write(&mAddSpeed, sizeof(float));
	}

	void EffectRotationAxis::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mAxisPosition, sizeof(Vector3));
		pStream->read(&mAxisDirection, sizeof(Vector3));
		pStream->read(&mSpeed, sizeof(float));
		pStream->read(&mAddSpeed, sizeof(float));
	}

	void EffectRotationAxis::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_RA_AXISPOSITION]);
		list.push_back(token[TOKEN_CL_RA_AXISDIRECTION]);
		list.push_back(token[TOKEN_CL_RA_SPEED]);
		list.push_back(token[TOKEN_CL_RA_ADDSPEED]);
	}

	bool EffectRotationAxis::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_RA_AXISPOSITION])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_CL_RA_AXISDIRECTION])
		{
			type = IElement::PT_DIRECTION; return true;
		}
		else if(name == token[TOKEN_CL_RA_SPEED])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_RA_ADDSPEED])
		{
			type = IElement::PT_INT; return true;
		}

		return false;
	}

	bool EffectRotationAxis::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RA_AXISPOSITION])
		{
			value = StringUtil::ToString(mAxisPosition); return true;
		}
		else if (name == token[TOKEN_CL_RA_AXISDIRECTION])
		{
			value = StringUtil::ToString(mAxisDirection); return true;
		}
		else if (name == token[TOKEN_CL_RA_SPEED])
		{
			value = StringUtil::ToString(mSpeed); return true;
		}
		else if (name == token[TOKEN_CL_RA_ADDSPEED])
		{
			value = StringUtil::ToString(mAddSpeed); return true;
		}

		return false;
	}

	bool EffectRotationAxis::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RA_AXISPOSITION])
		{
			mAxisPosition = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_RA_AXISDIRECTION])
		{
			mAxisDirection = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_RA_SPEED])
		{
			mSpeed = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_RA_ADDSPEED])
		{
			mAddSpeed = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/*EffectRotationRevolution                                           */
	/************************************************************************/
	const Vector3 EffectRotationRevolution::DEFAULT_AXIS(0, 0, 0);
	const Vector3 EffectRotationRevolution::DEFAULT_DIRECTION(0, 1, 0);
	const i32	EffectRotationRevolution::DEFAULT_ROTATIONSPEED = 0;
	const i32	EffectRotationRevolution::DEFAULT_ROTATIONADDSPEED = 0;


	EffectRotationRevolution::EffectRotationRevolution()
		:mAxisPosition(DEFAULT_AXIS)
		,mAxisDirection(DEFAULT_DIRECTION)
		,mSpeed(DEFAULT_ROTATIONSPEED)
		,mAddSpeed(DEFAULT_ROTATIONADDSPEED)
	{
		mType = ECT_RotateRevolution;
		mPriority = 3;
	}

	EffectRotationRevolution::~EffectRotationRevolution()
	{

	}

	void EffectRotationRevolution::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float realTime = (float)(affectTime-mBeginTime)/1000.0f;

				float deltaAngle = mSpeed*realTime + 0.5f*mAddSpeed*realTime*realTime;

				// position.
				Vector3 transVec = particle->position - mAxisPosition;
				Quaternion quaternion;
				quaternion.fromAxisAngle(mAxisDirection, deltaAngle*Math::DEG2RAD);
				Vector3 rotateVec = quaternion*transVec;
				particle->position = rotateVec + mAxisPosition;
			}
		}

		return;
	}

	void EffectRotationRevolution::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectRotationRevolution* pRotationRevolution = static_cast<EffectRotationRevolution*>(controller);
		pRotationRevolution->mAxisPosition = mAxisPosition;
		pRotationRevolution->mAxisDirection = mAxisDirection;
		pRotationRevolution->mSpeed = mSpeed;
		pRotationRevolution->mAddSpeed = mAddSpeed;
	}

	void EffectRotationRevolution::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mAxisPosition, sizeof(Vector3));
		pStream->write(&mAxisDirection, sizeof(Vector3));
		pStream->write(&mSpeed, sizeof(float));
		pStream->write(&mAddSpeed, sizeof(float));
	}

	void EffectRotationRevolution::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mAxisPosition, sizeof(Vector3));
		pStream->read(&mAxisDirection, sizeof(Vector3));
		pStream->read(&mSpeed, sizeof(float));
		pStream->read(&mAddSpeed, sizeof(float));
	}

	void EffectRotationRevolution::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_RR_AXISPOSITION]);
		list.push_back(token[TOKEN_CL_RR_AXISDIRECTION]);
		list.push_back(token[TOKEN_CL_RR_SPEED]);
		list.push_back(token[TOKEN_CL_RR_ADDSPEED]);
	}

	bool EffectRotationRevolution::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_RR_AXISPOSITION])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_CL_RR_AXISDIRECTION])
		{
			type = IElement::PT_DIRECTION; return true;
		}
		else if(name == token[TOKEN_CL_RR_SPEED])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_RR_ADDSPEED])
		{
			type = IElement::PT_INT; return true;
		}

		return false;
	}

	bool EffectRotationRevolution::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RR_AXISPOSITION])
		{
			value = StringUtil::ToString(mAxisPosition); return true;
		}
		else if (name == token[TOKEN_CL_RR_AXISDIRECTION])
		{
			value = StringUtil::ToString(mAxisDirection); return true;
		}
		else if (name == token[TOKEN_CL_RR_SPEED])
		{
			value = StringUtil::ToString(mSpeed); return true;
		}
		else if (name == token[TOKEN_CL_RR_ADDSPEED])
		{
			value = StringUtil::ToString(mAddSpeed); return true;
		}

		return false;
	}

	bool EffectRotationRevolution::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_RR_AXISPOSITION])
		{
			mAxisPosition = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_RR_AXISDIRECTION])
		{
			mAxisDirection = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_RR_SPEED])
		{
			mSpeed = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_RR_ADDSPEED])
		{
			mAddSpeed = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/*EffectOffcenter                                                    */
	/************************************************************************/
	const Vector3 EffectOffcenter::DEFAULT_CENTER(0, 0, 0);
	const float	EffectOffcenter::DEFAULT_OFFSPEED = 0.f;
	const float	EffectOffcenter::DEFAULT_OFFADDSPEED = 0.f;

	EffectOffcenter::EffectOffcenter()
		:mOffCenter(DEFAULT_CENTER)
		,mSpeed(DEFAULT_OFFSPEED)
		,mAddSpeed(DEFAULT_OFFADDSPEED)
	{
		mType = ECT_Offcenter;
		mPriority = 1;
	}

	EffectOffcenter::~EffectOffcenter()
	{

	}


	// ??¨C?¡®??? ¡Ò? ??¡°? ¡°?¡±¡Ì?€??????????
	void EffectOffcenter::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float realTime = (float)(affectTime-mBeginTime)/1000.0f;

				float deltaDistance = mSpeed*realTime + 0.5f*mAddSpeed*realTime*realTime;

				Vector3 offDirection = particle->originalPosition - mOffCenter;
				offDirection.normalize();
				// ¡±¡Þ???¡ëposition.
				particle->position += offDirection * deltaDistance;
			}
		}

		return;
	}

	void EffectOffcenter::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mOffCenter, sizeof(Vector3));
		pStream->write(&mSpeed, sizeof(float));
		pStream->write(&mAddSpeed, sizeof(float));
	}

	void EffectOffcenter::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mOffCenter, sizeof(Vector3));
		pStream->read(&mSpeed, sizeof(float));
		pStream->read(&mAddSpeed, sizeof(float));
	}

	void EffectOffcenter::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectOffcenter* pOffcenter = static_cast<EffectOffcenter*>(controller);
		pOffcenter->mOffCenter = mOffCenter;
		pOffcenter->mSpeed = mSpeed;
		pOffcenter->mAddSpeed = mAddSpeed;
	}

	void EffectOffcenter::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_OC_OFFCENTER]);
		list.push_back(token[TOKEN_CL_OC_SPEED]);
		list.push_back(token[TOKEN_CL_OC_ADDSPEED]);
	}

	bool EffectOffcenter::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_OC_OFFCENTER])
		{
			type = IElement::PT_VECTOR3; return true;
		}
		else if(name == token[TOKEN_CL_OC_SPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_OC_ADDSPEED])
		{
			type = IElement::PT_REAL; return true;
		}

		return false;
	}

	bool EffectOffcenter::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_OC_OFFCENTER])
		{
			value = StringUtil::ToString(mOffCenter); return true;
		}
		else if (name == token[TOKEN_CL_OC_SPEED])
		{
			value = StringUtil::ToString(mSpeed); return true;
		}
		else if (name == token[TOKEN_CL_OC_ADDSPEED])
		{
			value = StringUtil::ToString(mAddSpeed); return true;
		}

		return false;
	}

	bool EffectOffcenter::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_OC_OFFCENTER])
		{
			mOffCenter = StringUtil::ParseVec3(value); return true;
		}
		else if (name == token[TOKEN_CL_OC_SPEED])
		{
			mSpeed = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_OC_ADDSPEED])
		{
			mAddSpeed = StringUtil::ParseReal(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectColorChange                                                 */
	/************************************************************************/
	const i32	EffectColorChange::DEFAULT_COLORR = 0;
	const i32	EffectColorChange::DEFAULT_COLORG = 0;
	const i32	EffectColorChange::DEFAULT_COLORB = 0;
	const i32	EffectColorChange::DEFAULT_COLORA = 0;

	EffectColorChange::EffectColorChange()
		:mColorR(DEFAULT_COLORR)
		,mColorG(DEFAULT_COLORG)
		,mColorB(DEFAULT_COLORB)
		,mColorA(DEFAULT_COLORA)
	{
		mType = ECT_ColorChange;
		mPriority = 6;
	}

	EffectColorChange::~EffectColorChange()
	{

	}

	void EffectColorChange::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float controllTime = float(affectTime-mBeginTime)/1000.0f;

				float deltaR = controllTime * mColorR / 256.0f;
				float deltaG = controllTime * mColorG / 256.0f;
				float deltaB = controllTime * mColorB / 256.0f;
				float deltaA = controllTime * mColorA / 256.0f;

				Color color(deltaR, deltaG, deltaB, deltaA);

				particle->color = particle->originalColor + color;
				particle->color.saturate();
			}
		}

		return;
	}

	void EffectColorChange::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectColorChange* pColorChange = static_cast<EffectColorChange*>(controller);
		pColorChange->mColorR = mColorR;
		pColorChange->mColorG = mColorG;
		pColorChange->mColorB = mColorB;
		pColorChange->mColorA = mColorA;
	}

	void EffectColorChange::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mColorR, sizeof(i32));
		pStream->write(&mColorG, sizeof(i32));
		pStream->write(&mColorB, sizeof(i32));
		pStream->write(&mColorA, sizeof(i32));
	}

	void EffectColorChange::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mColorR, sizeof(i32));
		pStream->read(&mColorG, sizeof(i32));
		pStream->read(&mColorB, sizeof(i32));
		pStream->read(&mColorA, sizeof(i32));
	}

	void EffectColorChange::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_CC_COLORR]);
		list.push_back(token[TOKEN_CL_CC_COLORG]);
		list.push_back(token[TOKEN_CL_CC_COLORB]);
		list.push_back(token[TOKEN_CL_CC_COLORA]);
	}

	bool EffectColorChange::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_CC_COLORR])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CC_COLORG])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CC_COLORB])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CC_COLORA])
		{
			type = IElement::PT_INT; return true;
		}

		return false;
	}

	bool EffectColorChange::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CC_COLORR])
		{
			value = StringUtil::ToString(mColorR); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORG])
		{
			value = StringUtil::ToString(mColorG); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORB])
		{
			value = StringUtil::ToString(mColorB); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORA])
		{
			value = StringUtil::ToString(mColorA); return true;
		}
		return false;
	}

	bool EffectColorChange::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CC_COLORR])
		{
			mColorR = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORG])
		{
			mColorG = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORB])
		{
			mColorB = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CC_COLORA])
		{
			mColorA = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectScaleChange                                                 */
	/************************************************************************/
	const float	EffectScaleChange::DEFAULT_SCALE = 0.0f;
	const float	EffectScaleChange::DEFAULT_MINSCALE = 0.01f;
	const float	EffectScaleChange::DEFAULT_MAXSCALE = 9999.0f;

	EffectScaleChange::EffectScaleChange()
		:mScale(DEFAULT_SCALE)
		,mMaxScale(DEFAULT_MAXSCALE)
		,mMinScale(DEFAULT_MINSCALE)
	{
		mType = ECT_ScaleChange;
		mPriority= 9;
	}

	EffectScaleChange::~EffectScaleChange()
	{

	}

	void EffectScaleChange::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float controllTime = float(affectTime-mBeginTime)/1000.0f;

				float currentScale = mScale*controllTime;

				particle->scale = particle->originalScale + (currentScale);
				if(particle->scale > mMaxScale) particle->scale = mMaxScale;
				if(particle->scale < mMinScale) particle->scale = mMinScale;
			}
		}

		return;
	}

	void EffectScaleChange::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectScaleChange* pScalechange = static_cast<EffectScaleChange*>(controller);
		pScalechange->mScale = mScale;
		pScalechange->mMinScale = mMinScale;
		pScalechange->mMaxScale = mMaxScale;
	}

	void EffectScaleChange::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mScale, sizeof(float));
		pStream->write(&mMinScale, sizeof(float));
		pStream->write(&mMaxScale, sizeof(float));
	}

	void EffectScaleChange::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mScale, sizeof(float));
		pStream->read(&mMinScale, sizeof(float));
		pStream->read(&mMaxScale, sizeof(float));
	}

	void EffectScaleChange::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_SC_SCALE]);
		list.push_back(token[TOKEN_CL_SC_MINSCALE]);
		list.push_back(token[TOKEN_CL_SC_MAXSCALE]);
	}

	bool EffectScaleChange::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_SC_SCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SC_MINSCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SC_MAXSCALE])
		{
			type = IElement::PT_REAL; return true;
		}

		return false;
	}

	bool EffectScaleChange::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SC_SCALE])
		{
			value = StringUtil::ToString(mScale); return true;
		}
		else if (name == token[TOKEN_CL_SC_MINSCALE])
		{
			value = StringUtil::ToString(mMinScale); return true;
		}
		else if (name == token[TOKEN_CL_SC_MAXSCALE])
		{
			value = StringUtil::ToString(mMaxScale); return true;
		}

		return false;
	}

	bool EffectScaleChange::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SC_SCALE])
		{
			mScale = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SC_MINSCALE])
		{
			mMinScale = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SC_MAXSCALE])
		{
			mMaxScale = StringUtil::ParseReal(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectScaleNoise                                                 */
	/************************************************************************/
	const float	EffectScaleNoise::DEFAULT_SWING = 1.0f;
	const float	EffectScaleNoise::DEFAULT_WAVELEN = 1.0f;

	EffectScaleNoise::EffectScaleNoise()
		:mSwing(DEFAULT_SWING)
		,mWaveLen(DEFAULT_WAVELEN)
	{
		mType = ECT_ScaleNoise;
		mPriority = 8;
	}

	EffectScaleNoise::~EffectScaleNoise()
	{

	}

	void EffectScaleNoise::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float noise = (Math::Sin((float(affectTime-mBeginTime/1000.0f))/mWaveLen) + 1.0f) * mSwing;
				particle->scale = particle->originalScale + noise;
			}
		}

		return;
	}

	void EffectScaleNoise::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectScaleNoise* pScalenoise = static_cast<EffectScaleNoise*>(controller);
		pScalenoise->mSwing = mSwing;
		pScalenoise->mWaveLen = mWaveLen;
	}

	void EffectScaleNoise::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mSwing, sizeof(float));
		pStream->write(&mWaveLen, sizeof(float));
	}

	void EffectScaleNoise::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mSwing, sizeof(float));
		pStream->read(&mWaveLen, sizeof(float));
	}

	void EffectScaleNoise::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_SN_SWING]);
		list.push_back(token[TOKEN_CL_SN_WAVELEN]);
	}

	bool EffectScaleNoise::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_SN_SWING])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SN_WAVELEN])
		{
			type = IElement::PT_REAL; return true;
		}

		return false;
	}

	bool EffectScaleNoise::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SN_SWING])
		{
			value = StringUtil::ToString(mSwing); return true;
		}
		else if (name == token[TOKEN_CL_SN_WAVELEN])
		{
			value = StringUtil::ToString(mWaveLen); return true;
		}

		return false;
	}

	bool EffectScaleNoise::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SN_SWING])
		{
			mSwing = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SN_WAVELEN])
		{
			mWaveLen = StringUtil::ParseReal(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectColorNoise                                                */
	/************************************************************************/
	const float		EffectColorNoise::DEFAULT_SWING = 2.0f;
	const float		EffectColorNoise::DEFAULT_WAVELEN = 0.1f;
	const Color		EffectColorNoise::DEFAULT_BASECOLOR(1, 1, 1, 1);

	EffectColorNoise::EffectColorNoise()
		:mSwing(DEFAULT_SWING)
		,mWaveLen(DEFAULT_WAVELEN)
		,mBaseColor(DEFAULT_BASECOLOR)
	{
		mType = ECT_ColorNoise;
		mPriority = 5;
	}

	EffectColorNoise::~EffectColorNoise()
	{

	}

	void EffectColorNoise::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				float noise = (Math::Sin((float(affectTime-mBeginTime)/1000.0f)/mWaveLen) + 1.0f) / mSwing;
				particle->color = mBaseColor*noise;
				particle->color.saturate();
			}
		}

		return;
	}

	void EffectColorNoise::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectColorNoise* pColorNoise = static_cast<EffectColorNoise*>(controller);
		pColorNoise->mSwing = mSwing;
		pColorNoise->mWaveLen = mWaveLen;
		pColorNoise->mBaseColor = mBaseColor;
	}

	void EffectColorNoise::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mSwing, sizeof(float));
		pStream->write(&mWaveLen, sizeof(float));
		pStream->write(&mBaseColor, sizeof(Color));
	}

	void EffectColorNoise::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mSwing, sizeof(float));
		pStream->read(&mWaveLen, sizeof(float));
		pStream->read(&mBaseColor, sizeof(Color));
	}

	void EffectColorNoise::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);
		list.push_back(token[TOKEN_CL_CN_SWING]);
		list.push_back(token[TOKEN_CL_CN_WAVELEN]);
		list.push_back(token[TOKEN_CL_CN_BASECOLOR]);
	}

	bool EffectColorNoise::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_CN_SWING])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_CN_WAVELEN])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_CN_BASECOLOR])
		{
			type = IElement::PT_COLOR; return true;
		}

		return false;
	}

	bool EffectColorNoise::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CN_SWING])
		{
			value = StringUtil::ToString(mSwing); return true;
		}
		else if (name == token[TOKEN_CL_CN_WAVELEN])
		{
			value = StringUtil::ToString(mWaveLen); return true;
		}
		else if (name == token[TOKEN_CL_CN_BASECOLOR])
		{
			value = StringUtil::ToString(mBaseColor); return true;
		}

		return false;
	}

	bool EffectColorNoise::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CN_SWING])
		{
			mSwing = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_CN_WAVELEN])
		{
			mWaveLen = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_CN_BASECOLOR])
		{
			mBaseColor = StringUtil::ParseColor(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectColorSet                                                    */
	/************************************************************************/
	const Color	EffectColorSet::DEFAULT_BEGIN_COLOR(1, 1, 1, 1);
	const i32	EffectColorSet::DEFAULT_SET_NUM = 1;
	const Color	EffectColorSet::DEFAULT_TARGET_COLOR(1, 1, 1, 1);
	const i32	EffectColorSet::DEFAULT_SET_TIME = 100;


	EffectColorSet::EffectColorSet()
		:mBeginColor(DEFAULT_BEGIN_COLOR)
		,mSetNum(DEFAULT_SET_NUM)
		,mTargetColor1(DEFAULT_TARGET_COLOR)
		,mSetTime1(DEFAULT_SET_TIME)
		,mTargetColor2(DEFAULT_TARGET_COLOR)
		,mSetTime2(DEFAULT_SET_TIME)
		,mTargetColor3(DEFAULT_TARGET_COLOR)
		,mSetTime3(DEFAULT_SET_TIME)
		,mTargetColor4(DEFAULT_TARGET_COLOR)
		,mSetTime4(DEFAULT_SET_TIME)
	{
		mType = ECT_ColorSet;
		mPriority = 7;
	}

	EffectColorSet::~EffectColorSet()
	{

	}

	void EffectColorSet::_notifyStart(EffectParticle* particle)
	{
		EffectController::_notifyStart(particle);
		mSetTime[0] = mBeginTime;
		mSetTime[1] = mSetTime[0] + mSetTime1;
		mSetTime[2] = mSetTime[1] + mSetTime2;
		mSetTime[3] = mSetTime[2] + mSetTime3;
		mSetTime[4] = mSetTime[3] + mSetTime4;
		switch(mSetNum)
		{
		case 0: mLastColor = mBeginColor; break;
		case 1: mLastColor = mTargetColor1; break;
		case 2: mLastColor = mTargetColor2; break;
		case 3: mLastColor = mTargetColor3; break;
		case 4: mLastColor = mTargetColor4; break;
		}
	}

	void EffectColorSet::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				//???¡Ì¦Ð??¡­??¡°?¡Ò??¡À?¡ë.
				int currentIndex = -1;
				Color src,dest;

				for(int i=0; i<=mSetNum; ++i)
				{
					if(affectTime < mSetTime[i])
					{
						currentIndex = i; break;
					}
				}

				assert(affectTime != 0);

				if(-1 == currentIndex)
				{
					particle->color = mLastColor;
					return;
				}

				switch(currentIndex)
				{
				case 1: src = mBeginColor;   dest = mTargetColor1;  break;
				case 2: src = mTargetColor1; dest = mTargetColor2; break;
				case 3: src = mTargetColor2; dest = mTargetColor3; break;
				case 4: src = mTargetColor3; dest = mTargetColor4; break;
				default:  break;
				}

				int dt = affectTime - mSetTime[currentIndex-1];
				float slerp = float(dt)/float(mSetTime[currentIndex] - mSetTime[currentIndex-1]);

				particle->color = src + (dest-src) * slerp; 
			}
		}

		return;
	}

	void EffectColorSet::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectColorSet* pColorSet = static_cast<EffectColorSet*>(controller);
		pColorSet->mBeginColor = mBeginColor;
		pColorSet->mSetNum = mSetNum;
		pColorSet->mTargetColor1 = mTargetColor1;
		pColorSet->mSetTime1 = mSetTime1;
		pColorSet->mTargetColor2 = mTargetColor2;
		pColorSet->mSetTime2 = mSetTime2;
		pColorSet->mTargetColor3 = mTargetColor3;
		pColorSet->mSetTime3 = mSetTime3;
		pColorSet->mTargetColor4 = mTargetColor4;
		pColorSet->mSetTime4 = mSetTime4;
	}

	void EffectColorSet::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mBeginColor, sizeof(Color));
		pStream->write(&mSetNum, sizeof(i32));
		pStream->write(&mTargetColor1, sizeof(Color));
		pStream->write(&mSetTime1, sizeof(i32));
		pStream->write(&mTargetColor2, sizeof(Color));
		pStream->write(&mSetTime2, sizeof(i32));
		pStream->write(&mTargetColor3, sizeof(Color));
		pStream->write(&mSetTime3, sizeof(i32));
		pStream->write(&mTargetColor4, sizeof(Color));
		pStream->write(&mSetTime4, sizeof(i32));
	}

	void EffectColorSet::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mBeginColor, sizeof(Color));
		pStream->read(&mSetNum, sizeof(i32));
		pStream->read(&mTargetColor1, sizeof(Color));
		pStream->read(&mSetTime1, sizeof(i32));
		pStream->read(&mTargetColor2, sizeof(Color));
		pStream->read(&mSetTime2, sizeof(i32));
		pStream->read(&mTargetColor3, sizeof(Color));
		pStream->read(&mSetTime3, sizeof(i32));
		pStream->read(&mTargetColor4, sizeof(Color));
		pStream->read(&mSetTime4, sizeof(i32));
	}

	void EffectColorSet::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);

		list.push_back(token[TOKEN_CL_CS_BEGINCOLOR]);
		list.push_back(token[TOKEN_CL_CS_SETNUM]);
		list.push_back(token[TOKEN_CL_CS_TARGET1]);
		list.push_back(token[TOKEN_CL_CS_TIME1]);
		list.push_back(token[TOKEN_CL_CS_TARGET2]);
		list.push_back(token[TOKEN_CL_CS_TIME2]);
		list.push_back(token[TOKEN_CL_CS_TARGET3]);
		list.push_back(token[TOKEN_CL_CS_TIME3]);
		list.push_back(token[TOKEN_CL_CS_TARGET4]);
		list.push_back(token[TOKEN_CL_CS_TIME4]);

	}

	bool EffectColorSet::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_CS_BEGINCOLOR])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_CL_CS_SETNUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CS_TARGET1])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_CL_CS_TIME1])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CS_TARGET2])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_CL_CS_TIME2])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CS_TARGET3])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_CL_CS_TIME3])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_CS_TARGET4])
		{
			type = IElement::PT_COLOR; return true;
		}
		else if(name == token[TOKEN_CL_CS_TIME4])
		{
			type = IElement::PT_INT; return true;
		}
		return false;
	}

	bool EffectColorSet::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CS_BEGINCOLOR])
		{
			value = StringUtil::ToString(mBeginColor); return true;
		}
		else if (name == token[TOKEN_CL_CS_SETNUM])
		{
			value = StringUtil::ToString(mSetNum); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET1])
		{
			value = StringUtil::ToString(mTargetColor1); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME1])
		{
			value = StringUtil::ToString(mSetTime1); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET2])
		{
			value = StringUtil::ToString(mTargetColor2); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME2])
		{
			value = StringUtil::ToString(mSetTime2); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET3])
		{
			value = StringUtil::ToString(mTargetColor3); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME3])
		{
			value = StringUtil::ToString(mSetTime3); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET4])
		{
			value = StringUtil::ToString(mTargetColor4); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME4])
		{
			value = StringUtil::ToString(mSetTime4); return true;
		}
		return false;
	}

	bool EffectColorSet::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_CS_BEGINCOLOR])
		{
			mBeginColor = StringUtil::ParseColor(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_SETNUM])
		{
			mSetNum = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET1])
		{
			mTargetColor1 = StringUtil::ParseColor(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME1])
		{
			mSetTime1 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET2])
		{
			mTargetColor2 = StringUtil::ParseColor(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME2])
		{
			mSetTime2 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET3])
		{
			mTargetColor3 = StringUtil::ParseColor(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME3])
		{
			mSetTime3 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TARGET4])
		{
			mTargetColor4 = StringUtil::ParseColor(value); return true;
		}
		else if (name == token[TOKEN_CL_CS_TIME4])
		{
			mSetTime4 = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	/************************************************************************/
	/* EffectScaleSet                                                    */
	/************************************************************************/
	const float		EffectScaleSet::DEFAULT_BEGIN_SCALE = 1.0f;
	const i32		EffectScaleSet::DEFAULT_SET_NUM = 1;
	const float		EffectScaleSet::DEFAULT_TARGET_SCALE = 1.0f;
	const i32		EffectScaleSet::DEFAULT_SET_TIME = 100;


	EffectScaleSet::EffectScaleSet()
		:mBeginScale(DEFAULT_BEGIN_SCALE)
		,mSetNum(DEFAULT_SET_NUM)
		,mTargetScale1(DEFAULT_TARGET_SCALE)
		,mSetTime1(DEFAULT_SET_TIME)
		,mTargetScale2(DEFAULT_TARGET_SCALE)
		,mSetTime2(DEFAULT_SET_TIME)
		,mTargetScale3(DEFAULT_TARGET_SCALE)
		,mSetTime3(DEFAULT_SET_TIME)
		,mTargetScale4(DEFAULT_TARGET_SCALE)
		,mSetTime4(DEFAULT_SET_TIME)
	{
		mType = ECT_ScaleSet;
		mPriority = 10;
	}

	EffectScaleSet::~EffectScaleSet()
	{

	}

	void EffectScaleSet::_notifyStart(EffectParticle* particle)
	{
		EffectController::_notifyStart(particle);
		mSetTime[0] = mBeginTime;
		mSetTime[1] = mSetTime[0] + mSetTime1;
		mSetTime[2] = mSetTime[1] + mSetTime2;
		mSetTime[3] = mSetTime[2] + mSetTime3;
		mSetTime[4] = mSetTime[3] + mSetTime4;
		switch(mSetNum)
		{
		case 0: mLastScale = mBeginScale; break;
		case 1: mLastScale = mTargetScale1; break;
		case 2: mLastScale = mTargetScale2; break;
		case 3: mLastScale = mTargetScale3; break;
		case 4: mLastScale = mTargetScale4; break;
		}
	}

	void EffectScaleSet::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if( (!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if(particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				int currentIndex = -1;
				float src = 0.0f;
				float dest = 0.0f;

				for(int i=0; i<=mSetNum; ++i)
				{
					if(affectTime < mSetTime[i])
					{
						currentIndex = i; break;
					}
				}

				assert(affectTime != 0);

				if(-1 == currentIndex)
				{
					particle->scale = mLastScale;
					return;
				}

				switch(currentIndex)
				{
				case 1: src = mBeginScale;   dest = mTargetScale1; break;
				case 2: src = mTargetScale1; dest = mTargetScale2; break;
				case 3: src = mTargetScale2; dest = mTargetScale3; break;
				case 4: src = mTargetScale3; dest = mTargetScale4; break;
				default: break;
				}

				int dt = affectTime - mSetTime[currentIndex-1];
				float slerp = float(dt)/float(mSetTime[currentIndex] - mSetTime[currentIndex-1]);

				particle->scale = particle->originalScale * (src + (dest-src) * slerp); 
			}
		}

		return;
	}

	void EffectScaleSet::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectScaleSet* pScaleSet = static_cast<EffectScaleSet*>(controller);
		pScaleSet->mBeginScale = mBeginScale;
		pScaleSet->mSetNum = mSetNum;
		pScaleSet->mTargetScale1 = mTargetScale1;
		pScaleSet->mSetTime1 = mSetTime1;
		pScaleSet->mTargetScale2 = mTargetScale2;
		pScaleSet->mSetTime2 = mSetTime2;
		pScaleSet->mTargetScale3 = mTargetScale3;
		pScaleSet->mSetTime3 = mSetTime3;
		pScaleSet->mTargetScale4 = mTargetScale4;
		pScaleSet->mSetTime4 = mSetTime4;
	}

	void EffectScaleSet::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mBeginScale, sizeof(float));
		pStream->write(&mSetNum, sizeof(i32));
		pStream->write(&mTargetScale1, sizeof(float));
		pStream->write(&mSetTime1, sizeof(i32));
		pStream->write(&mTargetScale2, sizeof(float));
		pStream->write(&mSetTime2, sizeof(i32));
		pStream->write(&mTargetScale3, sizeof(float));
		pStream->write(&mSetTime3, sizeof(i32));
		pStream->write(&mTargetScale4, sizeof(float));
		pStream->write(&mSetTime4, sizeof(i32));
	}

	void EffectScaleSet::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mBeginScale, sizeof(float));
		pStream->read(&mSetNum, sizeof(i32));
		pStream->read(&mTargetScale1, sizeof(float));
		pStream->read(&mSetTime1, sizeof(i32));
		pStream->read(&mTargetScale2, sizeof(float));
		pStream->read(&mSetTime2, sizeof(i32));
		pStream->read(&mTargetScale3, sizeof(float));
		pStream->read(&mSetTime3, sizeof(i32));
		pStream->read(&mTargetScale4, sizeof(float));
		pStream->read(&mSetTime4, sizeof(i32));
	}

	void EffectScaleSet::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);

		list.push_back(token[TOKEN_CL_SS_BEGINSCALE]);
		list.push_back(token[TOKEN_CL_SS_SETNUM]);
		list.push_back(token[TOKEN_CL_SS_TARGET1]);
		list.push_back(token[TOKEN_CL_SS_TIME1]);
		list.push_back(token[TOKEN_CL_SS_TARGET2]);
		list.push_back(token[TOKEN_CL_SS_TIME2]);
		list.push_back(token[TOKEN_CL_SS_TARGET3]);
		list.push_back(token[TOKEN_CL_SS_TIME3]);
		list.push_back(token[TOKEN_CL_SS_TARGET4]);
		list.push_back(token[TOKEN_CL_SS_TIME4]);

	}

	bool EffectScaleSet::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectController::getPropertyType(name, type))
			return true;

		if(name == token[TOKEN_CL_SS_BEGINSCALE])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SS_SETNUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_SS_TARGET1])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SS_TIME1])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_SS_TARGET2])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SS_TIME2])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_SS_TARGET3])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SS_TIME3])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_SS_TARGET4])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_CL_SS_TIME4])
		{
			type = IElement::PT_INT; return true;
		}
		return false;
	}

	bool EffectScaleSet::getPropertyValue(const String & name, String &value)
	{
		if(EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SS_BEGINSCALE])
		{
			value = StringUtil::ToString(mBeginScale); return true;
		}
		else if (name == token[TOKEN_CL_SS_SETNUM])
		{
			value = StringUtil::ToString(mSetNum); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET1])
		{
			value = StringUtil::ToString(mTargetScale1); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME1])
		{
			value = StringUtil::ToString(mSetTime1); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET2])
		{
			value = StringUtil::ToString(mTargetScale2); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME2])
		{
			value = StringUtil::ToString(mSetTime2); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET3])
		{
			value = StringUtil::ToString(mTargetScale3); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME3])
		{
			value = StringUtil::ToString(mSetTime3); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET4])
		{
			value = StringUtil::ToString(mTargetScale4); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME4])
		{
			value = StringUtil::ToString(mSetTime4); return true;
		}
		return false;
	}

	bool EffectScaleSet::setPropertyValue(const String & name,const String & value)
	{
		if(EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_SS_BEGINSCALE])
		{
			mBeginScale = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_SETNUM])
		{
			mSetNum = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET1])
		{
			mTargetScale1 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME1])
		{
			mSetTime1 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET2])
		{
			mTargetScale2 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME2])
		{
			mSetTime2 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET3])
		{
			mTargetScale3 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME3])
		{
			mSetTime3 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TARGET4])
		{
			mTargetScale4 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_CL_SS_TIME4])
		{
			mSetTime4 = StringUtil::ParseInt(value); return true;
		}

		return false;
	}


	/************************************************************************/
	/* EffectUVSpeed                                                        */
	/************************************************************************/
	const Vector2	EffectUVSpeedSet::DEFAULT_BEGIN_UVSPEED = Vector2::ZERO;
	const i32		EffectUVSpeedSet::DEFAULT_SET_NUM = 1;
	const Vector2	EffectUVSpeedSet::DEFAULT_TARGET_UVSPEED = Vector2::ZERO;
	const i32		EffectUVSpeedSet::DEFAULT_SET_TIME = 100;


	EffectUVSpeedSet::EffectUVSpeedSet()
		: mEffectTexture(1)
		, mBeginUVSpeed(DEFAULT_BEGIN_UVSPEED)
		, mSetNum(DEFAULT_SET_NUM)
		, mTargetUVSpeed1(DEFAULT_TARGET_UVSPEED)
		, mSetTime1(DEFAULT_SET_TIME)
		, mTargetUVSpeed2(DEFAULT_TARGET_UVSPEED)
		, mSetTime2(DEFAULT_SET_TIME)
		, mTargetUVSpeed3(DEFAULT_TARGET_UVSPEED)
		, mSetTime3(DEFAULT_SET_TIME)
		, mTargetUVSpeed4(DEFAULT_TARGET_UVSPEED)
		, mSetTime4(DEFAULT_SET_TIME)
	{
		mType = ECT_UVSpeedSet;
		mPriority = 1;
	}

	EffectUVSpeedSet::~EffectUVSpeedSet()
	{

	}

	void EffectUVSpeedSet::_notifyStart(EffectParticle* particle)
	{
		EffectController::_notifyStart(particle);
		mSetTime[0] = mBeginTime;
		mSetTime[1] = mSetTime[0] + mSetTime1;
		mSetTime[2] = mSetTime[1] + mSetTime2;
		mSetTime[3] = mSetTime[2] + mSetTime3;
		mSetTime[4] = mSetTime[3] + mSetTime4;
		switch (mSetNum)
		{
		case 0: mLastUVSpeed = mBeginUVSpeed; break;
		case 1: mLastUVSpeed = mTargetUVSpeed1; break;
		case 2: mLastUVSpeed = mTargetUVSpeed2; break;
		case 3: mLastUVSpeed = mTargetUVSpeed3; break;
		case 4: mLastUVSpeed = mTargetUVSpeed4; break;
		}
	}

	void EffectUVSpeedSet::controll(i32 time, EffectParticle* particle, bool bParticleSet)
	{
		EffectController::controll(time, particle, bParticleSet);
		if ((!bParticleSet && mInController)
			|| (bParticleSet && particle->mInController))
		{
			if (bParticleSet && particle->mTime <= mBeginTime)
			{
				return;
			}

			i32 affectTime = particle->mTime;
			if (particle->mTime > mEndTime && mEndTime != -1)
				affectTime = mEndTime;
			{
				int currentIndex = -1;
				Vector2 src = Vector2::ZERO;
				Vector2 dest = Vector2::ZERO;

				for (int i = 0; i <= mSetNum; ++i)
				{
					if (affectTime < mSetTime[i])
					{
						currentIndex = i; break;
					}
				}

				assert(affectTime != 0);

				EffectLayer* _layer = dynamic_cast<EffectLayer*>(particle);
				EffectMaterial* _mt = _layer->getMaterial();

				if (-1 == currentIndex)
				{
					if (mEffectTexture == 1)
					{
						_mt->setPropertyValue("e_material_uspeed", StringUtil::ToString(mLastUVSpeed.x));
						_mt->setPropertyValue("e_material_vspeed", StringUtil::ToString(mLastUVSpeed.y));
					}
					else
					{
						_mt->setPropertyValue("e_material_uspeed1", StringUtil::ToString(mLastUVSpeed.x));
						_mt->setPropertyValue("e_material_vspeed1", StringUtil::ToString(mLastUVSpeed.y));
					}
					
					return;
				}

				switch (currentIndex)
				{
				case 1: src = mBeginUVSpeed;   dest = mTargetUVSpeed1; break;
				case 2: src = mTargetUVSpeed1; dest = mTargetUVSpeed2; break;
				case 3: src = mTargetUVSpeed2; dest = mTargetUVSpeed3; break;
				case 4: src = mTargetUVSpeed3; dest = mTargetUVSpeed4; break;
				default: break;
				}

				int dt = affectTime - mSetTime[currentIndex - 1];
				float slerp = float(dt) / float(mSetTime[currentIndex] - mSetTime[currentIndex - 1]);

				Vector2 _currentUVSpeed = (src + (dest - src) * slerp);

				if (mEffectTexture == 1)
				{
					_mt->setPropertyValue("e_material_uspeed", StringUtil::ToString(_currentUVSpeed.x));
					_mt->setPropertyValue("e_material_vspeed", StringUtil::ToString(_currentUVSpeed.y));
				}
				else
				{
					_mt->setPropertyValue("e_material_uspeed1", StringUtil::ToString(_currentUVSpeed.x));
					_mt->setPropertyValue("e_material_vspeed1", StringUtil::ToString(_currentUVSpeed.y));
				}
			}
		}

		return;
	}

	void EffectUVSpeedSet::copyAttributesTo(EffectController* controller)
	{
		EffectController::copyAttributesTo(controller);

		EffectUVSpeedSet* pUVSpeedSet = static_cast<EffectUVSpeedSet*>(controller);
		pUVSpeedSet->mEffectTexture = mEffectTexture;
		pUVSpeedSet->mBeginUVSpeed = mBeginUVSpeed;
		pUVSpeedSet->mSetNum = mSetNum;
		pUVSpeedSet->mTargetUVSpeed1 = mTargetUVSpeed1;
		pUVSpeedSet->mSetTime1 = mSetTime1;
		pUVSpeedSet->mTargetUVSpeed2 = mTargetUVSpeed2;
		pUVSpeedSet->mSetTime2 = mSetTime2;
		pUVSpeedSet->mTargetUVSpeed3 = mTargetUVSpeed3;
		pUVSpeedSet->mSetTime3 = mSetTime3;
		pUVSpeedSet->mTargetUVSpeed4 = mTargetUVSpeed4;
		pUVSpeedSet->mSetTime4 = mSetTime4;
	}

	void EffectUVSpeedSet::exportData(DataStream* pStream, int version)
	{
		EffectController::exportData(pStream, version);
		pStream->write(&mEffectTexture, sizeof(i32));
		pStream->write(&mBeginUVSpeed, sizeof(Vector2));
		pStream->write(&mSetNum, sizeof(i32));
		pStream->write(&mTargetUVSpeed1, sizeof(Vector2));
		pStream->write(&mSetTime1, sizeof(i32));
		pStream->write(&mTargetUVSpeed2, sizeof(Vector2));
		pStream->write(&mSetTime2, sizeof(i32));
		pStream->write(&mTargetUVSpeed3, sizeof(Vector2));
		pStream->write(&mSetTime3, sizeof(i32));
		pStream->write(&mTargetUVSpeed4, sizeof(Vector2));
		pStream->write(&mSetTime4, sizeof(i32));
	}

	void EffectUVSpeedSet::importData(DataStream* pStream, int version)
	{
		EffectController::importData(pStream, version);
		pStream->read(&mEffectTexture, sizeof(i32));
		pStream->read(&mBeginUVSpeed, sizeof(Vector2));
		pStream->read(&mSetNum, sizeof(i32));
		pStream->read(&mTargetUVSpeed1, sizeof(Vector2));
		pStream->read(&mSetTime1, sizeof(i32));
		pStream->read(&mTargetUVSpeed2, sizeof(Vector2));
		pStream->read(&mSetTime2, sizeof(i32));
		pStream->read(&mTargetUVSpeed3, sizeof(Vector2));
		pStream->read(&mSetTime3, sizeof(i32));
		pStream->read(&mTargetUVSpeed4, sizeof(Vector2));
		pStream->read(&mSetTime4, sizeof(i32));
	}

	void EffectUVSpeedSet::getPropertyList(PropertyList & list)
	{
		EffectController::getPropertyList(list);

		list.push_back(token[TOKEN_CL_UVSS_EFFECTTEXTURE]);
		list.push_back(token[TOKEN_CL_UVSS_BEGINSCALE]);
		list.push_back(token[TOKEN_CL_UVSS_SETNUM]);
		list.push_back(token[TOKEN_CL_UVSS_TARGET1]);
		list.push_back(token[TOKEN_CL_UVSS_TIME1]);
		list.push_back(token[TOKEN_CL_UVSS_TARGET2]);
		list.push_back(token[TOKEN_CL_UVSS_TIME2]);
		list.push_back(token[TOKEN_CL_UVSS_TARGET3]);
		list.push_back(token[TOKEN_CL_UVSS_TIME3]);
		list.push_back(token[TOKEN_CL_UVSS_TARGET4]);
		list.push_back(token[TOKEN_CL_UVSS_TIME4]);

	}

	bool EffectUVSpeedSet::getPropertyType(const String & name, PropertyType &type)
	{
		if (EffectController::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_CL_UVSS_EFFECTTEXTURE])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_CL_UVSS_BEGINSCALE])
		{
			type = IElement::PT_VECTOR2; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_SETNUM])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET1])
		{
			type = IElement::PT_VECTOR2; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME1])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET2])
		{
			type = IElement::PT_VECTOR2; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME2])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET3])
		{
			type = IElement::PT_VECTOR2; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME3])
		{
			type = IElement::PT_INT; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET4])
		{
			type = IElement::PT_VECTOR2; return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME4])
		{
			type = IElement::PT_INT; return true;
		}
		return false;
	}

	bool EffectUVSpeedSet::getPropertyValue(const String & name, String &value)
	{
		if (EffectController::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_UVSS_EFFECTTEXTURE])
		{
			value = StringUtil::ToString(mEffectTexture); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_BEGINSCALE])
		{
			value = StringUtil::ToString(mBeginUVSpeed); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_SETNUM])
		{
			value = StringUtil::ToString(mSetNum); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET1])
		{
			value = StringUtil::ToString(mTargetUVSpeed1); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME1])
		{
			value = StringUtil::ToString(mSetTime1); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET2])
		{
			value = StringUtil::ToString(mTargetUVSpeed2); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME2])
		{
			value = StringUtil::ToString(mSetTime2); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET3])
		{
			value = StringUtil::ToString(mTargetUVSpeed3); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME3])
		{
			value = StringUtil::ToString(mSetTime3); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET4])
		{
			value = StringUtil::ToString(mTargetUVSpeed4); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME4])
		{
			value = StringUtil::ToString(mSetTime4); return true;
		}
		return false;
	}

	bool EffectUVSpeedSet::setPropertyValue(const String & name, const String & value)
	{
		if (EffectController::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_CL_UVSS_EFFECTTEXTURE])
		{
			mEffectTexture = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_BEGINSCALE])
		{
			mBeginUVSpeed = StringUtil::ParseVec2(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_SETNUM])
		{
			mSetNum = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET1])
		{
			mTargetUVSpeed1 = StringUtil::ParseVec2(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME1])
		{
			mSetTime1 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET2])
		{
			mTargetUVSpeed2 = StringUtil::ParseVec2(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME2])
		{
			mSetTime2 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET3])
		{
			mTargetUVSpeed3 = StringUtil::ParseVec2(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME3])
		{
			mSetTime3 = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TARGET4])
		{
			mTargetUVSpeed4 = StringUtil::ParseVec2(value); return true;
		}
		else if (name == token[TOKEN_CL_UVSS_TIME4])
		{
			mSetTime4 = StringUtil::ParseInt(value); return true;
		}

		return false;
	}

	std::vector<std::pair<String, String> >	EffectController::getAllProperties()
	{
		PropertyList list;
		std::vector<std::pair<String, String> > res;
		//¿ØÖÆÆ÷ÊôÐÔ
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
