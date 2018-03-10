#ifndef __ECHO_EFFECT_CONTROLLER_H__
#define __ECHO_EFFECT_CONTROLLER_H__

#include "Engine/Core.h"
#include "EffectIElement.h"
#include "engine/core/Memory/MemManager.h"

namespace Echo
{
	/**
	 * ÌØÐ§¿ØÖÆÆ÷
	 */
	class EffectController : public IElement
	{
	public:
		//interface
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const i32 DEFAULT_BEGINTIME;
		static const i32 DEFAULT_ENDTIME;

	public:
		EffectController();
		~EffectController();

		//stream
		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		// target
		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);
		virtual void _notifyStart(EffectParticle* particle);

		// type
		EFFECT_CONTROL_TYPE getControllerType() const { return mType; }

		virtual void copyAttributesTo(EffectController* controller);

		int getPriority() { return mPriority; }
		virtual bool isControllerMovement() = 0; //is controll pos an direction

		i32 getBeginTime() const { return mBeginTime; }

		std::vector<std::pair<String, String> >	getAllProperties();

	protected:
		i32 mBeginTime;
		i32 mEndTime;
		EFFECT_CONTROL_TYPE mType;

		// first fly away, then move , at last oritation
		int         mPriority;
		bool		mInController; //is in controller
	};

	// translate  Priority 1
	class EffectTranslation : public EffectController
	{
		friend class TranslationTranslator;
		friend class TranslationWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const Vector3	DEFAULT_DIRECTION;
		static const float	DEFAULT_SPEED;
		static const float	DEFAULT_ADDSPEED;

	public:
		EffectTranslation();
		~EffectTranslation();


		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void _notifyStart(EffectParticle* particle);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return true; }

	protected:
		Vector3 mDirection;
		float mSpeed;
		float mAddSpeed;
	};

	// Self rotation Priority 4
	class EffectRotationSelf : public EffectController
	{
		friend class RotationSelfTranslator;
		friend class RotationSelfWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const i32 DEFAULT_ROTATIONSPEED;
		static const i32 DEFAULT_ROTATIONADDSPEED;

	public:
		EffectRotationSelf();
		~EffectRotationSelf();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		i32 mSpeed;
		i32 mAddSpeed;
	};

	// Rotateion Axis Priority 2
	class EffectRotationAxis : public EffectController
	{
		friend class RotationAxisTranslator;
		friend class RotationAxisWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const Vector3 DEFAULT_AXIS;
		static const Vector3 DEFAULT_DIRECTION;
		static const i32 DEFAULT_ROTATIONSPEED;
		static const i32 DEFAULT_ROTATIONADDSPEED;

	public:
		EffectRotationAxis();

		~EffectRotationAxis();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return true; }

	protected:
		Vector3 mAxisPosition;
		Vector3 mAxisDirection;
		i32 mSpeed;
		i32 mAddSpeed;
	};

	// Priority 3
	class EffectRotationRevolution : public EffectController
	{
		friend class RotationRevolutionTranslator;
		friend class RotationRevolutionWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);
	public:
		static const Vector3 DEFAULT_AXIS;
		static const Vector3 DEFAULT_DIRECTION;
		static const i32 DEFAULT_ROTATIONSPEED;
		static const i32 DEFAULT_ROTATIONADDSPEED;

	public:
		EffectRotationRevolution();
		~EffectRotationRevolution();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return true; }

	protected:
		Vector3 mAxisPosition;
		Vector3 mAxisDirection;
		i32 mSpeed;
		i32 mAddSpeed;
	};

	// Priority 1
	class EffectOffcenter : public EffectController
	{
		friend class OffcenterTranslator;
		friend class OffcenterWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const Vector3	DEFAULT_CENTER;
		static const float		DEFAULT_OFFSPEED;
		static const float		DEFAULT_OFFADDSPEED;

	public:
		EffectOffcenter();
		~EffectOffcenter();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return true; }

	protected:
		Vector3 mOffCenter;
		float mSpeed;
		float mAddSpeed;
	};

	// Priority 6
	class EffectColorChange : public EffectController
	{
		friend class ColorChangeTranslator;
		friend class ColorChangeWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const i32 DEFAULT_COLORR;
		static const i32 DEFAULT_COLORG;
		static const i32 DEFAULT_COLORB;
		static const i32 DEFAULT_COLORA;

	public:
		EffectColorChange();
		~EffectColorChange();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		i32 mColorR;
		i32 mColorG;
		i32 mColorB;
		i32 mColorA;
	};

	// Priority 9
	class EffectScaleChange : public EffectController
	{
		friend class ScaleChangeTranslator;
		friend class ScaleChangeWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const float DEFAULT_SCALE;
		static const float DEFAULT_MAXSCALE;
		static const float DEFAULT_MINSCALE;

	public:
		EffectScaleChange();
		~EffectScaleChange();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		float mScale;
		float mMaxScale;
		float mMinScale;
	};

	// Priority 8
	class EffectScaleNoise : public EffectController
	{
		friend class ScaleNoiseTranslator;
		friend class ScaleNoiseWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const float DEFAULT_SWING;
		static const float DEFAULT_WAVELEN;

	public:
		EffectScaleNoise();
		~EffectScaleNoise();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		float mSwing;
		float mWaveLen;
	};

	// Priority 5
	class EffectColorNoise : public EffectController
	{
		friend class ColorNoiseTranslator;
		friend class ColorNoiseWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const float DEFAULT_SWING;
		static const float DEFAULT_WAVELEN;
		static const Color DEFAULT_BASECOLOR;

	public:
		EffectColorNoise();
		~EffectColorNoise();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		float mSwing;
		float mWaveLen;
		Color mBaseColor;
	};

	// Priority 7
	class EffectColorSet : public EffectController
	{
		friend class ColorSetTranslator;
		friend class ColorSetWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const Color	DEFAULT_BEGIN_COLOR;
		static const i32	DEFAULT_SET_NUM;
		static const Color	DEFAULT_TARGET_COLOR;
		static const i32	DEFAULT_SET_TIME;

	public:
		EffectColorSet();
		~EffectColorSet();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void _notifyStart(EffectParticle* particle);	
		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		Color	mBeginColor;
		i32		mSetNum;
		Color	mTargetColor1;
		i32		mSetTime1;
		Color	mTargetColor2;
		i32		mSetTime2;
		Color	mTargetColor3;
		i32		mSetTime3;
		Color	mTargetColor4;
		i32		mSetTime4;
		i32		mSetTime[5];
		Color	mLastColor;
	};

	// Priority 10
	class EffectScaleSet : public EffectController
	{
		friend class ScaleSetTranslator;
		friend class ScaleSetWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		static const float		DEFAULT_BEGIN_SCALE;
		static const i32		DEFAULT_SET_NUM;
		static const float		DEFAULT_TARGET_SCALE;
		static const i32		DEFAULT_SET_TIME;

	public:
		EffectScaleSet();
		~EffectScaleSet();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void _notifyStart(EffectParticle* particle);
		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		float	mBeginScale;
		i32		mSetNum;
		float	mTargetScale1;
		i32		mSetTime1;
		float	mTargetScale2;
		i32		mSetTime2;
		float	mTargetScale3;
		i32		mSetTime3;
		float	mTargetScale4;
		i32		mSetTime4;
		i32		mSetTime[5];
		float	mLastScale;
	};

	class EffectUVSpeedSet : public EffectController
	{
		friend class ScaleSetTranslator;
		friend class ScaleSetWriter;
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name, const String & value);

	public:
		static const Vector2	DEFAULT_BEGIN_UVSPEED;
		static const i32		DEFAULT_SET_NUM;
		static const Vector2	DEFAULT_TARGET_UVSPEED;
		static const i32		DEFAULT_SET_TIME;

	public:
		EffectUVSpeedSet();
		~EffectUVSpeedSet();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);

		virtual void _notifyStart(EffectParticle* particle);
		virtual void controll(i32 time, EffectParticle* particle, bool bParticleSet = false);

		virtual void copyAttributesTo(EffectController* controller);

		virtual bool isControllerMovement() { return false; }

	protected:
		i32     mEffectTexture;
		Vector2	mBeginUVSpeed;
		i32		mSetNum;
		Vector2	mTargetUVSpeed1;
		i32		mSetTime1;
		Vector2	mTargetUVSpeed2;
		i32		mSetTime2;
		Vector2	mTargetUVSpeed3;
		i32		mSetTime3;
		Vector2	mTargetUVSpeed4;
		i32		mSetTime4;
		i32		mSetTime[5];
		Vector2	mLastUVSpeed;
	};
}

#endif