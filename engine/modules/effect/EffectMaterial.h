#pragma once

#include "Engine/Core.h"
#include "engine/core/Util/PtrMonitor.h"
#include <engine/core/render/render/RenderState.h>
#include "EffectIElement.h"
#include "Engine/core/camera/Camera.h"

namespace Echo
{
	/**
	 * 特效材质
	 */
	class EffectMaterial : public PtrMonitor<EffectMaterial>, public IElement
	{
	public:
		String getBlendModeStr() const;
		void setBlendModeStr(const String& blendStr);

		String getDistortionTypeStr() const;
		void setDistortionTypeStr(const String &typeStr);

		EFFECT_BLEND_MODE getBlendMode() const;
		int getUV2_RGB_BlendMode(){ return mUV2_rgb_blendModel; }
		int getUV2_A_BlendMode(){ return mUV2_a_blendModel; }
	public:
		//interface
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name, const String& value);

	public:
		static const EFFECT_BLEND_MODE	DEFAULT_MT_BLEND_MODE;
		static const EFFECT_DISTORTION_TYPE DEFAULT_DS_TYPE;
		static const String				DEFAULT_MT_TEXTURENAME;
		static const String				DEFAULT_DS_TEXTURENAME;

	public:
		EffectMaterial();
		~EffectMaterial();

		void prepare_res();
		void prepare_io();
		void prepare();
		void unprepare();

		void importData(DataStream* pStream, int version);
		void exportData(DataStream* pStream, int version);

		void _notifyStart();
		void _update(i32 time);

		virtual void copyAttributesTo (EffectMaterial* material);

		inline EFFECT_BLEND_MODE getSceneBlend() const { return mBlendMode; }
		inline void setSceneBlend(EFFECT_BLEND_MODE mode) { mBlendMode = mode; }
		BlendState::BlendFactor getSrcBlendFactor() const;
		BlendState::BlendFactor getDestBlendFactor() const;
		inline const String& getTextureName() const { return mTextureName; }
		inline void setTextureName(const String& tn) { mTextureName = tn; }
		TextureRes* getTexturePtr() const { return mTexturePtr; }

		// 获取纹理采样
		const TextureSampler& getTextureSampler() { return m_textureSampler; }

		inline float getTexAddrLeft() const { return mTexAddrLeftRealtime; }
		inline float getTexAddrTop() const { return mTexAddrTopRealtime; }
		inline float getTexAddrRight() const { return mTexAddrRightRealtime; }
		inline float getTexAddrBottom() const { return mTexAddrBottomRealtime; }
		inline float getUSpeed() const { return mUSpped; }
		inline float getVSpeed() const { return mVSpeed; }
		inline bool getSwapUV() const { return mbSwapUV; }
		inline int getRenderPriority()const{ return mRenderPriority; }
		String getUVString() const;
		void setUVString(const String& str);
		String getDistortionUV()const;
		void setDistortionUV(const String& str);
		inline const String& getDistortionTextureName() const { return mNoiseTexName; }

		inline void setDistortionEnable( bool _val ) { mDistortionEnable = _val; }
		inline bool getDistortionEnable() const { return mDistortionEnable; }

		inline void setGlowEnable(bool _val) { mGlowEnable = _val; }
		inline bool getGlowEnable() const { return mGlowEnable; }

		void getDistortionUVScaleOffset( Vector4& outVec ) const
		{
			outVec.x = mDistortionTcScaleU;
			outVec.y = mDistortionTcScaleV;
			outVec.z = mDistortionTcOffsetU;
			outVec.w = mDistortionTcOffsetV;
		}

		float getDistortionAlphaThreshold() const
		{
			return mDistortionAlphaThreshold;
		}

		float getDistortionBlendWeight() const
		{
			return mDistortionBlendWeight;
		}

		Vector2 getCenterUV()
		{
			return Vector2((mTexAddrRight - mTexAddrLeft)/2 + mTexAddrLeft,(mTexAddrBottom - mTexAddrTop)/2 + mTexAddrTop);
		}

		EFFECT_DISTORTION_TYPE getDistortionType()
		{
			return mDistortionType;
		}

		TextureRes* getDistortionTexturePtr() const
		{
			return mNoiseTextPtr;
		}

		void setDistortionTexturePtr( TextureRes* pTex )
		{
			mNoiseTextPtr = pTex;
		}

		bool getOnlyDistortionLayer() const
		{
			return mDistortionOnly;
		}

		//  特效支持两张贴图的材质
		inline void setUV2Enable(bool _val) { mbUV2Enable = _val; }
		inline bool getUV2Enable() const { return mbUV2Enable; }
		inline const String& getTextureName1() const { return mTextureName1; }
		inline void setTextureName1(const String& tn) { mTextureName1 = tn; }
		TextureRes* getTexturePtr1() const { return mTexturePtr1; }
		const TextureSampler& getTextureSampler1() { return m_textureSampler1; }
		inline float getTexAddrLeft1() const { return mTexAddrLeftRealtime1; }
		inline float getTexAddrTop1() const { return mTexAddrTopRealtime1; }
		inline float getTexAddrRight1() const { return mTexAddrRightRealtime1; }
		inline float getTexAddrBottom1() const { return mTexAddrBottomRealtime1; }
		inline float getUSpeed1() const { return mUSpped1; }
		inline float getVSpeed1() const { return mVSpeed1; }
		inline bool getSwapUV1() const { return mbSwapUV1; }

		bool getIsUIEffect(){ return mIsUIEffect; }
		void setIsUIEffect(bool setting){ mIsUIEffect = setting; }

		Vector3* getDissolutionParameter(){ return &mDissolutionParameter; }

		bool isDissolution(){ return !!mDissolutionParameter.x; }

		bool isTestDepth(){ return mIsTestDepth; }
		void setTestDepth(bool setting){ mIsTestDepth = setting; }

		bool operator==(const EffectMaterial& anotherMaterial) const
		{
			if(!mbUV2Enable)
			{
				return mTextureName == anotherMaterial.mTextureName &&
					mBlendMode == anotherMaterial.mBlendMode &&
					mIsUIEffect == anotherMaterial.mIsUIEffect &&
					mIsTestDepth == anotherMaterial.mIsTestDepth;
			}
			else
			{
				return mTextureName == anotherMaterial.mTextureName &&
					mBlendMode == anotherMaterial.mBlendMode &&
					mTextureName1 == anotherMaterial.mTextureName1 && 
					mIsUIEffect == anotherMaterial.mIsUIEffect &&
					mIsTestDepth == anotherMaterial.mIsTestDepth;
			}
		}

	protected:
		EFFECT_BLEND_MODE	mBlendMode;			
		String				mTextureName;					
		TextureRes*		    mTexturePtr;
		TextureSampler		m_textureSampler;
		float				mTexAddrLeft;
		float				mTexAddrRight;
		float				mTexAddrTop;
		float				mTexAddrBottom;
		float				mTexAddrLeftRealtime;
		float				mTexAddrRightRealtime;
		float				mTexAddrTopRealtime;
		float				mTexAddrBottomRealtime;
		bool				mbFlipU;			
		bool				mbFlipV;
		bool				mbSwapUV;
		float				mUSpped;
		float				mVSpeed;
		int					mTimer;

		int					mRenderPriority; //渲染优先级

		bool                mDistortionEnable;
		String				mNoiseTexName;					
		TextureRes*		    mNoiseTextPtr;	
		float				mDistorTexAddrLeft;
		float				mDistorTexAddrRight;
		float				mDistorTexAddrTop;
		float				mDistorTexAddrBottom;

		float				mDistortionAlphaThreshold;
		float				mDistortionTcOffsetU;
		float				mDistortionTcOffsetV;
		float               mDistortionTcScaleU;
		float               mDistortionTcScaleV;
		float               mDistortionBlendWeight;
		EFFECT_DISTORTION_TYPE mDistortionType;
		bool				mDistortionOnly;

		bool				mGlowEnable;

		bool                mbUV2Enable;
		int					mUV2_rgb_blendModel;
		int					mUV2_a_blendModel;
		String				mTextureName1;
		TextureRes*		    mTexturePtr1;
		TextureSampler		m_textureSampler1;
		float				mTexAddrLeft1;
		float				mTexAddrRight1;
		float				mTexAddrTop1;
		float				mTexAddrBottom1;
		float				mTexAddrLeftRealtime1;
		float				mTexAddrRightRealtime1;
		float				mTexAddrTopRealtime1;
		float				mTexAddrBottomRealtime1;
		bool				mbFlipU1;
		bool				mbFlipV1;
		bool				mbSwapUV1;
		float				mUSpped1;
		float				mVSpeed1;
		Vector3				mDissolutionParameter;
		float				mDissolutionSpeed;

		bool				mIsUIEffect;
		bool				mIsTestDepth;
	};
}