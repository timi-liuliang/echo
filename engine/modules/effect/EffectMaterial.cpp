#include "EffectMaterial.h"
#include "engine/core/Util/Exception.h"
#include "Render/Material.h"
#include "Engine/core/Scene/Scene_Manager.h"
#include "engine/core/resource/DataStream.h"
#include "EffectSystemManager.h"
#include "render/RenderQueueGroup.h"

namespace Echo
{
	const EFFECT_BLEND_MODE	EffectMaterial::DEFAULT_MT_BLEND_MODE = AlphaLightMode;
	const EFFECT_DISTORTION_TYPE EffectMaterial::DEFAULT_DS_TYPE = DISTORTION_LINEAR;
	const String			EffectMaterial::DEFAULT_MT_TEXTURENAME = "blank.png";
	const String			EffectMaterial::DEFAULT_DS_TEXTURENAME = "noise.jpg";

	template<> HashNode<EffectMaterial>** PtrMonitor<EffectMaterial>::m_ptrs = NULL;
	template<> int PtrMonitor<EffectMaterial>::m_hashcount = 0;
	template<> unsigned int PtrMonitor<EffectMaterial>::m_maxhash = 10;

	EffectMaterial::EffectMaterial()
		: PtrMonitor()
		, mBlendMode(DEFAULT_MT_BLEND_MODE)
		, mTextureName(DEFAULT_MT_TEXTURENAME)
		, mTexturePtr(NULL)
		, mTexAddrLeft(0)
		, mTexAddrRight(1)
		, mTexAddrTop(0)
		, mTexAddrBottom(1)
		, mbFlipU(false)
		, mbFlipV(false)
		, mbSwapUV(false)
		, mUSpped(0)
		, mVSpeed(0)
		, mTimer(0)
		, mRenderPriority(3)
		, mDistortionEnable(false)
		, mNoiseTexName(DEFAULT_DS_TEXTURENAME)
		, mNoiseTextPtr(NULL)
		, mDistorTexAddrLeft(0)
		, mDistorTexAddrRight(1)
		, mDistorTexAddrTop(0)
		, mDistorTexAddrBottom(1)
		, mDistortionAlphaThreshold(0.0f)
		, mDistortionTcOffsetU(0.0f)
		, mDistortionTcOffsetV(0.0f)
		, mDistortionTcScaleU(0.0f)
		, mDistortionTcScaleV(0.0f)
		, mDistortionBlendWeight(0.0f)
		, mDistortionType(DEFAULT_DS_TYPE)
		, mDistortionOnly(false)
		, mGlowEnable(false)
		, mbUV2Enable(false)
		, mUV2_rgb_blendModel(0)
		, mUV2_a_blendModel(0)
		, mTextureName1(DEFAULT_MT_TEXTURENAME)
		, mTexturePtr1(NULL)
		, mTexAddrLeft1(0)
		, mTexAddrRight1(1)
		, mTexAddrTop1(0)
		, mTexAddrBottom1(1)
		, mbFlipU1(false)
		, mbFlipV1(false)
		, mbSwapUV1(false)
		, mUSpped1(0)
		, mVSpeed1(0)
		, mDissolutionParameter(Vector3::ZERO)
		, mDissolutionSpeed(0.f)
		, mIsUIEffect(false)
		, mIsTestDepth(true)
	{

	}

	EffectMaterial::~EffectMaterial()
	{
		unprepare();
	}

	void EffectMaterial::prepare_res()
	{
		if(mTexturePtr==NULL)
		{
			mTexturePtr = TextureResManager::instance()->createTexture(mTextureName);
		}

		if( mDistortionEnable && mNoiseTextPtr == NULL &&  mNoiseTexName != "")
		{
			mNoiseTextPtr = TextureResManager::instance()->createTexture(mNoiseTexName);
		}

		if (mbUV2Enable && mTexturePtr1 == NULL)
		{
			mTexturePtr1 = TextureResManager::instance()->createTexture(mTextureName1);
		}
	}

	void EffectMaterial::prepare_io()
	{
		if (mTexturePtr)
		{
			mTexturePtr->prepare();
		}
		else
		{
			EchoLogWarning("EffectMaterial::prepare_io failed, mTexturePtr is null");
		}

		if( mDistortionEnable && mNoiseTextPtr )
		{
			mNoiseTextPtr->prepare();
		}

		if (mbUV2Enable)
		{
			if (mTexturePtr1)
				mTexturePtr1->prepare();
			else
				EchoLogWarning("EffectMaterial::prepare_io failed, mTexturePtr1 is null");
		}
	}

	void EffectMaterial::prepare()
	{
		mTexturePtr->load();

		if( mDistortionEnable )
		{
			mNoiseTextPtr->prepareLoad();
		}

		Material* pMaterial = RenderQueueGroup::instance()->getRenderQueue("Effect")->getMaterial();

		m_textureSampler.m_texture = mTexturePtr->getTexture();
		m_textureSampler.m_samplerState = pMaterial->getSamplerState(0);

		mTexAddrLeftRealtime = mTexAddrLeft;
		mTexAddrRightRealtime = mTexAddrRight;
		mTexAddrTopRealtime = mTexAddrTop;
		mTexAddrBottomRealtime = mTexAddrBottom;

		if(mbFlipU)
		{
			Math::Swap(mTexAddrLeftRealtime, mTexAddrRightRealtime);
		}
		if(mbFlipV)
		{
			Math::Swap(mTexAddrTopRealtime, mTexAddrBottomRealtime);
		}

		if (mbUV2Enable)
		{
			mTexturePtr1->load();


			Material* pMaterial = EffectSystemManager::instance()->getMtlUV2();

			m_textureSampler1.m_texture = mTexturePtr1->getTexture();
			m_textureSampler1.m_samplerState = pMaterial->getSamplerState(0);

			mTexAddrLeftRealtime1 = mTexAddrLeft1;
			mTexAddrRightRealtime1 = mTexAddrRight1;
			mTexAddrTopRealtime1 = mTexAddrTop1;
			mTexAddrBottomRealtime1 = mTexAddrBottom1;

			if (mbFlipU1)
			{
				Math::Swap(mTexAddrLeftRealtime1, mTexAddrRightRealtime1);
			}
			if (mbFlipV1)
			{
				Math::Swap(mTexAddrTopRealtime1, mTexAddrBottomRealtime1);
			}

			mDissolutionParameter.z = 0.f;
		}

		_update(0);
	}

	void EffectMaterial::unprepare()
	{
		if(mTexturePtr)
		{
			TextureResManager::instance()->releaseResource(mTexturePtr);
			mTexturePtr = NULL;
		}

		if( mNoiseTextPtr )
		{
			TextureResManager::instance()->releaseResource(mNoiseTextPtr);
			mNoiseTextPtr = NULL;
		}
		
		if (mTexturePtr1)
		{
			TextureResManager::instance()->releaseResource(mTexturePtr1);
			mTexturePtr1 = NULL;
		}
	}

	/*bool EffectMaterial::operator ==(const EffectMaterial& anotherMaterial) const
	{
		if( !mDistortionEnable )
		{
			return mTextureName == anotherMaterial.mTextureName &&
			mBlendMode == anotherMaterial.mBlendMode;
		}
		else
		{
			return mTextureName == anotherMaterial.mTextureName &&
				mBlendMode == anotherMaterial.mBlendMode && 
				mNoiseTexName == anotherMaterial.mNoiseTexName &&
				mDistortionOnly == mDistortionOnly;
		}
	}*/

	void EffectMaterial::_notifyStart()
	{
		mTexAddrLeftRealtime = mTexAddrLeft;
		mTexAddrRightRealtime = mTexAddrRight;
		mTexAddrTopRealtime = mTexAddrTop;
		mTexAddrBottomRealtime = mTexAddrBottom;
		if (mbFlipU)
		{
			Math::Swap(mTexAddrLeftRealtime, mTexAddrRightRealtime);
		}
		if (mbFlipV)
		{
			Math::Swap(mTexAddrTopRealtime, mTexAddrBottomRealtime);
		}

		if (mbUV2Enable)
		{
			mTexAddrLeftRealtime1 = mTexAddrLeft1;
			mTexAddrRightRealtime1 = mTexAddrRight1;
			mTexAddrTopRealtime1 = mTexAddrTop1;
			mTexAddrBottomRealtime1 = mTexAddrBottom1;

			if (mbFlipU1)
			{
				Math::Swap(mTexAddrLeftRealtime1, mTexAddrRightRealtime1);
			}
			if (mbFlipV1)
			{
				Math::Swap(mTexAddrTopRealtime1, mTexAddrBottomRealtime1);
			}

			mDissolutionParameter.z = 0.f;
		}
	}

	// 特效材质更新
	void EffectMaterial::_update(i32 time)
	{
		if(mUSpped != 0)
		{
			float transu = mUSpped * (float)(time) * 0.001f;
			mTexAddrLeftRealtime += transu;
			mTexAddrRightRealtime += transu;

			float dx = Math::Abs( mTexAddrRight - mTexAddrLeft);
			if (mTexAddrLeftRealtime > (mTexAddrLeft + dx))
			{
				mTexAddrLeftRealtime -= dx;
				mTexAddrRightRealtime -= dx;
			}
				

			if (mTexAddrLeftRealtime < (mTexAddrLeft - dx))
			{
				mTexAddrLeftRealtime += dx;
				mTexAddrRightRealtime += dx;
			}
				
		}
		if(mVSpeed != 0)
		{
			float transv = mVSpeed * (float)(time) * 0.001f;
			mTexAddrTopRealtime += transv;
			mTexAddrBottomRealtime += transv;

			float dy = Math::Abs(mTexAddrTop - mTexAddrBottom);
			if (mTexAddrTopRealtime > (mTexAddrTop + dy))
			{
				mTexAddrTopRealtime -= dy;
				mTexAddrBottomRealtime -= dy;
			}
				

			if (mTexAddrTopRealtime < (mTexAddrTop - dy))
			{
				mTexAddrTopRealtime += dy;
				mTexAddrBottomRealtime += dy;
			}
		}

		if (mbUV2Enable)
		{
			if (mUSpped1 != 0)
			{
				float transu = mUSpped1 * (float)(time)* 0.001f;
				mTexAddrLeftRealtime1 += transu;
				mTexAddrRightRealtime1 += transu;

				float dx = Math::Abs(mTexAddrRight1 - mTexAddrLeft1);
				if (mTexAddrLeftRealtime1 > (mTexAddrLeft1 + dx))
				{
					mTexAddrLeftRealtime1 -= dx;
					mTexAddrRightRealtime1 -= dx;
				}


				if (mTexAddrLeftRealtime1 < (mTexAddrLeft1 - dx))
				{
					mTexAddrLeftRealtime1 += dx;
					mTexAddrRightRealtime1 += dx;
				}

			}
			if (mVSpeed1 != 0)
			{
				float transv = mVSpeed1 * (float)(time)* 0.001f;
				mTexAddrTopRealtime1 += transv;
				mTexAddrBottomRealtime1 += transv;

				float dy = Math::Abs(mTexAddrTop1 - mTexAddrBottom1);
				if (mTexAddrTopRealtime1 > (mTexAddrTop1 + dy))
				{
					mTexAddrTopRealtime1 -= dy;
					mTexAddrBottomRealtime1 -= dy;
				}


				if (mTexAddrTopRealtime1 < (mTexAddrTop1 - dy))
				{
					mTexAddrTopRealtime1 += dy;
					mTexAddrBottomRealtime1 += dy;
				}
			}

			if (mDissolutionSpeed != 0.0f)
			{
				float transu = mDissolutionSpeed * (float)(time)* 0.001f;
				mDissolutionParameter.z += transu;
			}
		}
	}

	BlendState::BlendFactor EffectMaterial::getSrcBlendFactor()const
	{
		switch(mBlendMode)
		{
		case AlphaLightMode:
			return BlendState::BF_SRC_ALPHA;
		case HighLightMode:
			return BlendState::BF_SRC_ALPHA;
		}
		return BlendState::BF_SRC_ALPHA;
	}

	BlendState::BlendFactor EffectMaterial::getDestBlendFactor()const
	{
		switch(mBlendMode)
		{
		case AlphaLightMode:
			return BlendState::BF_INV_SRC_ALPHA;
		case HighLightMode:
			return BlendState::BF_ONE;
		}
		return BlendState::BF_DST_ALPHA;
	}

	String EffectMaterial::getUVString() const
	{
		String ret = StringUtil::ToString(Vector4(mTexAddrLeft, mTexAddrTop, mTexAddrRight, mTexAddrBottom));
		return ret;
	}

	void EffectMaterial::setUVString(const String& str)
	{
		Vector4 vec = StringUtil::ParseVec4(str);
		mTexAddrLeft = vec.x;
		mTexAddrTop = vec.y;
		mTexAddrRight = vec.z;
		mTexAddrBottom = vec.w;
	}

	String EffectMaterial::getDistortionUV() const
	{
		String ret = StringUtil::ToString(Vector4(mDistorTexAddrLeft, mDistorTexAddrTop, mDistorTexAddrRight, mDistorTexAddrBottom));
		return ret;
	}

	void EffectMaterial::setDistortionUV(const String& str)
	{
		Vector4 vec = StringUtil::ParseVec4(str);
		mDistorTexAddrLeft = vec.x;
		mDistorTexAddrTop = vec.y;
		mDistorTexAddrRight = vec.z;
		mDistorTexAddrBottom = vec.w;
	}

	void EffectMaterial::copyAttributesTo(EffectMaterial* material)
	{
		material->mBlendMode = mBlendMode;
		material->mTextureName = mTextureName;
		material->mTexAddrLeft = mTexAddrLeft;
		material->mTexAddrRight = mTexAddrRight;
		material->mTexAddrTop = mTexAddrTop;
		material->mTexAddrBottom = mTexAddrBottom;
		material->mbFlipU = mbFlipU;
		material->mbFlipV = mbFlipV;
		material->mbSwapUV = mbSwapUV;
		material->mUSpped = mUSpped;
		material->mVSpeed = mVSpeed;
		material->mRenderPriority = mRenderPriority;
		material->mDistortionEnable = mDistortionEnable;
		if (mDistortionEnable)
		{
			material->mNoiseTexName = mNoiseTexName;
			material->mDistorTexAddrLeft = mDistorTexAddrLeft;
			material->mDistorTexAddrRight = mDistorTexAddrRight;
			material->mDistorTexAddrTop = mDistorTexAddrTop;
			material->mDistorTexAddrBottom = mDistorTexAddrBottom;
			material->mDistortionAlphaThreshold = mDistortionAlphaThreshold;
			material->mDistortionTcOffsetU = mDistortionTcOffsetU;
			material->mDistortionTcOffsetV = mDistortionTcOffsetV;
			material->mDistortionTcScaleU = mDistortionTcScaleU;
			material->mDistortionTcScaleV = mDistortionTcScaleV;
			material->mDistortionBlendWeight = mDistortionBlendWeight;
			material->mDistortionType = mDistortionType;
			material->mDistortionOnly = mDistortionOnly;
		}
		material->mGlowEnable = mGlowEnable;
		material->mbUV2Enable = mbUV2Enable;
		if (mbUV2Enable)
		{
			material->mTextureName1 = mTextureName1;
			material->mUV2_rgb_blendModel = mUV2_rgb_blendModel;
			material->mUV2_a_blendModel = mUV2_a_blendModel;
			material->mTexAddrLeft1 = mTexAddrLeft1;
			material->mTexAddrRight1 = mTexAddrRight1;
			material->mTexAddrTop1 = mTexAddrTop1;
			material->mTexAddrBottom1 = mTexAddrBottom1;
			material->mbFlipU1 = mbFlipU1;
			material->mbFlipV1 = mbFlipV1;
			material->mbSwapUV1 = mbSwapUV1;
			material->mUSpped1 = mUSpped1;
			material->mVSpeed1 = mVSpeed1;
			material->mDissolutionParameter = mDissolutionParameter;
			material->mDissolutionSpeed = mDissolutionSpeed;
		}

		material->mIsUIEffect = mIsUIEffect;
		material->mIsTestDepth = mIsTestDepth;
	}

	String EffectMaterial::getDistortionTypeStr() const
	{
		String result;
		switch (mDistortionType)
		{
		case DISTORTION_LINEAR:
			result = "0"; break;
		case DISTORTION_CENTER:
			result = "1"; break;
		}
		return result;
	}

	void EffectMaterial::setDistortionTypeStr(const String &typeStr)
	{
		EFFECT_DISTORTION_TYPE type = DISTORTION_LINEAR;
		if (typeStr == "中心模式" || typeStr == "1")
			type = DISTORTION_CENTER;

		mDistortionType = type;
	}


	String EffectMaterial::getBlendModeStr() const
	{
		String result;
		switch (mBlendMode)
		{
		case AlphaLightMode:
			result = "alpha_lightting"; break;
		case HighLightMode:
			result = "high_lightting"; break;
		}
		return result;
	}

	void EffectMaterial::setBlendModeStr(const String &blendStr)
	{
		EFFECT_BLEND_MODE blendMode = AlphaLightMode;
		if (blendStr == "high_lightting")
			blendMode = HighLightMode;

		mBlendMode = blendMode;
	}

	EFFECT_BLEND_MODE EffectMaterial::getBlendMode() const
	{
		return mBlendMode;
	}

	void EffectMaterial::exportData(DataStream* pStream, int version)
	{
		pStream->write(&mBlendMode, sizeof(EFFECT_BLEND_MODE));
		ui32 strlength = mTextureName.length();
		pStream->write(&strlength, sizeof(ui32));
		pStream->write(mTextureName.c_str(), strlength);
		pStream->write(&mTexAddrLeft, sizeof(float));
		pStream->write(&mTexAddrTop, sizeof(float));
		pStream->write(&mTexAddrRight, sizeof(float));
		pStream->write(&mTexAddrBottom, sizeof(float));
		pStream->write(&mbFlipU, sizeof(bool));
		pStream->write(&mbFlipV, sizeof(bool));
		if (version > 0x00010000)
			pStream->write(&mbSwapUV, sizeof(bool));
		pStream->write(&mUSpped, sizeof(float));
		pStream->write(&mVSpeed, sizeof(float));
		if (version > 0x00010005)
		{
			pStream->write(&mRenderPriority, sizeof(i32));
		}

		pStream->write(&mDistortionEnable, sizeof(bool));
		if (mDistortionEnable)
		{
			ui32 strlength = mNoiseTexName.length();
			pStream->write(&strlength, sizeof(ui32));
			pStream->write(mNoiseTexName.c_str(), strlength);
			pStream->write(&mDistorTexAddrLeft, sizeof(float));
			pStream->write(&mDistorTexAddrTop, sizeof(float));
			pStream->write(&mDistorTexAddrRight, sizeof(float));
			pStream->write(&mDistorTexAddrBottom, sizeof(float));
			pStream->write(&mDistortionAlphaThreshold, sizeof(float));
			pStream->write(&mDistortionTcOffsetU, sizeof(float));
			pStream->write(&mDistortionTcOffsetV, sizeof(float));
			pStream->write(&mDistortionTcScaleU, sizeof(float));
			pStream->write(&mDistortionTcScaleV, sizeof(float));
			pStream->write(&mDistortionBlendWeight, sizeof(float));
			if (version > 0x00010004)
			{
				i32 type = StringUtil::ParseI32(getDistortionTypeStr());
				pStream->write(&type, sizeof(i32));
			}
			pStream->write(&mDistortionOnly, sizeof(bool));
		}

		pStream->write(&mGlowEnable,sizeof(bool));

		pStream->write(&mbUV2Enable, sizeof(bool));
		if (mbUV2Enable)
		{
			strlength = mTextureName1.length();
			pStream->write(&strlength, sizeof(ui32));
			pStream->write(mTextureName1.c_str(), strlength);
			pStream->write(&mUV2_rgb_blendModel, sizeof(int));
			pStream->write(&mUV2_a_blendModel, sizeof(int));
			pStream->write(&mTexAddrLeft1, sizeof(float));
			pStream->write(&mTexAddrTop1, sizeof(float));
			pStream->write(&mTexAddrRight1, sizeof(float));
			pStream->write(&mTexAddrBottom1, sizeof(float));
			pStream->write(&mbFlipU1, sizeof(bool));
			pStream->write(&mbFlipV1, sizeof(bool));
			pStream->write(&mbSwapUV1, sizeof(bool));
			pStream->write(&mUSpped1, sizeof(float));
			pStream->write(&mVSpeed1, sizeof(float));
			pStream->write(&mDissolutionParameter.x, sizeof(float));
			pStream->write(&mDissolutionParameter.y, sizeof(float));
			pStream->write(&mDissolutionSpeed, sizeof(float));
		}
		pStream->write(&mIsTestDepth, sizeof(bool));
	}

	void EffectMaterial::importData(DataStream* pStream, int version)
	{
		pStream->read(&mBlendMode, sizeof(EFFECT_BLEND_MODE));
		ui32 strLength;
		pStream->read(&strLength, sizeof(ui32));
		char str[128];
		pStream->read(str, strLength);
		str[strLength] = 0;
		mTextureName = str;
		pStream->read(&mTexAddrLeft, sizeof(float));
		pStream->read(&mTexAddrTop, sizeof(float));
		pStream->read(&mTexAddrRight, sizeof(float));
		pStream->read(&mTexAddrBottom, sizeof(float));
		pStream->read(&mbFlipU, sizeof(bool));
		pStream->read(&mbFlipV, sizeof(bool));
		if (version > 0x00010000)
			pStream->read(&mbSwapUV, sizeof(bool));
		pStream->read(&mUSpped, sizeof(float));
		pStream->read(&mVSpeed, sizeof(float));
		if (version > 0x00010005)
		{
			pStream->read(&mRenderPriority, sizeof(int));
		}

		if (version > 0x00010001)
		{
			pStream->read(&mDistortionEnable, sizeof(bool));
			if (mDistortionEnable)
			{
				ui32 strLength;
				pStream->read(&strLength, sizeof(ui32));
				char str[128];
				pStream->read(str, strLength);
				str[strLength] = 0;
				mNoiseTexName = str;
				pStream->read(&mDistorTexAddrLeft, sizeof(float));
				pStream->read(&mDistorTexAddrTop, sizeof(float));
				pStream->read(&mDistorTexAddrRight, sizeof(float));
				pStream->read(&mDistorTexAddrBottom, sizeof(float));
				pStream->read(&mDistortionAlphaThreshold, sizeof(float));
				pStream->read(&mDistortionTcOffsetU, sizeof(float));
				pStream->read(&mDistortionTcOffsetV, sizeof(float));
				pStream->read(&mDistortionTcScaleU, sizeof(float));
				pStream->read(&mDistortionTcScaleV, sizeof(float));
				pStream->read(&mDistortionBlendWeight, sizeof(float));
				if (version > 0x00010004)
				{
					i32 distortionType = (i32)DISTORTION_LINEAR;
					pStream->read(&distortionType, sizeof(i32));
					setDistortionTypeStr(StringUtil::ToString(distortionType));
				}
				pStream->read(&mDistortionOnly, sizeof(bool));
			}
		}

		if (version > 0x00010010)
		{
			pStream->read(&mGlowEnable, sizeof(bool));
		}

		if (version > 0x00010008)
		{
			pStream->read(&mbUV2Enable, sizeof(bool));
			if (mbUV2Enable)
			{
				strLength;
				pStream->read(&strLength, sizeof(ui32));
				char str[128];
				pStream->read(str, strLength);
				str[strLength] = 0;
				mTextureName1 = str;
				pStream->read(&mUV2_rgb_blendModel, sizeof(int));
				pStream->read(&mUV2_a_blendModel, sizeof(int));
				pStream->read(&mTexAddrLeft1, sizeof(float));
				pStream->read(&mTexAddrTop1, sizeof(float));
				pStream->read(&mTexAddrRight1, sizeof(float));
				pStream->read(&mTexAddrBottom1, sizeof(float));
				pStream->read(&mbFlipU1, sizeof(bool));
				pStream->read(&mbFlipV1, sizeof(bool));
				pStream->read(&mbSwapUV1, sizeof(bool));
				pStream->read(&mUSpped1, sizeof(float));
				pStream->read(&mVSpeed1, sizeof(float));

				if (version > 0x00010016)
				{
					pStream->read(&mDissolutionParameter.x, sizeof(float));
					pStream->read(&mDissolutionParameter.y, sizeof(float));
					pStream->read(&mDissolutionSpeed, sizeof(float));
				}
			}
		}

		if (version > 0x00010013)
		{
			pStream->read(&mIsTestDepth, sizeof(bool));
		}
	}

	void EffectMaterial::getPropertyList(PropertyList& list)
	{
		list.push_back(token[TOKEN_MT_BLENDMODE]);
		list.push_back(token[TOKEN_MT_DEPTHMODE]);
		list.push_back(token[TOKEN_MT_TEXTURE]);
		list.push_back(token[TOKEN_MT_UVVALUE]);
		list.push_back(token[TOKEN_MT_FLIPU]);
		list.push_back(token[TOKEN_MT_FLIPV]);
		list.push_back(token[TOKEN_MT_SWAPUV]);
		list.push_back(token[TOKEN_MT_TASPEEDU]);
		list.push_back(token[TOKEN_MT_TASPEEDV]);
		list.push_back(token[TOKEN_MT_RENDERPRIORITY]);
		list.push_back(token[TOKEN_DS_ENABEL]);
		//list.push_back(token[TOKEN_DS_TEXTURE]);
		//list.push_back(token[TOKEN_DS_UVVALUE]);
		//list.push_back(token[TOKEN_DS_ALPHA_THRESHOLD]);
		//list.push_back(token[TOKEN_DS_TC_OFFSETU]);
		//list.push_back(token[TOKEN_DS_TC_OFFSETV]);
		//list.push_back(token[TOKEN_DS_TC_SCALEU]);
		//list.push_back(token[TOKEN_DS_TC_SCALEV]);
		list.push_back(token[TOKEN_DS_BLEND_WEIGHT]);
		list.push_back(token[TOKEN_DS_TYPE]);
		//list.push_back(token[TOKEN_DS_ONLY]);
		list.push_back(token[TOKEN_GL_ENABLE]);
		list.push_back(token[TOKEN_MT_UV2_ENABLE]);
		list.push_back(token[TOKEN_MT_UV2_RGB_BLENDMODE]);
		list.push_back(token[TOKEN_MT_UV2_A_BLENDMODE]);
		list.push_back(token[TOKEN_MT_TEXTURE1]);
		list.push_back(token[TOKEN_MT_UVVALUE1]);
		list.push_back(token[TOKEN_MT_FLIPU1]);
		list.push_back(token[TOKEN_MT_FLIPV1]);
		list.push_back(token[TOKEN_MT_SWAPUV1]);
		list.push_back(token[TOKEN_MT_TASPEEDU1]);
		list.push_back(token[TOKEN_MT_TASPEEDV1]);
		list.push_back(token[TOKEN_MT_DSL_ENABLE]);
		list.push_back(token[TOKEN_MT_DSL_THRESHOLD]);
		list.push_back(token[TOKEN_MT_DSL_SPEED]);
		
	}

	bool EffectMaterial::getPropertyType(const String& name, PropertyType& type)
	{
		if (name == token[TOKEN_MT_BLENDMODE])
		{
			type = IElement::PT_MATERIAL_BLEND_MODE; return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE])
		{
			type = IElement::PT_MATERIAL_TEXTURE; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE])
		{
			type = IElement::PT_VECTOR4; return true;
		}
		else if (name == token[TOKEN_MT_FLIPU])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_FLIPV])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_MT_RENDERPRIORITY])
		{
			type = IElement::PT_RENDER_PRIORITY; return true;
		}
		else if (name == token[TOKEN_DS_ENABEL])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_DS_TEXTURE])
		{
			type = IElement::PT_MATERIAL_TEXTURE; return true;
		}
		else if (name == token[TOKEN_DS_UVVALUE])
		{
			type = IElement::PT_VECTOR4; return true;
		}
		else if (name == token[TOKEN_DS_ALPHA_THRESHOLD])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETU])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETV])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEU])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEV])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEU])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_BLEND_WEIGHT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_DS_TYPE])
		{
			type = IElement::PT_DISTORTION_TYPE; return true;
		}
		else if (name == token[TOKEN_DS_ONLY])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_GL_ENABLE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_UV2_ENABLE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_UV2_RGB_BLENDMODE])
		{
			type = IElement::PT_UV2_BLENDMODE; return true;
		}
		else if (name == token[TOKEN_MT_UV2_A_BLENDMODE])
		{
			type = IElement::PT_UV2_BLENDMODE; return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE1])
		{
			type = IElement::PT_MATERIAL_TEXTURE; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE1])
		{
			type = IElement::PT_VECTOR4; return true;
		}
		else if (name == token[TOKEN_MT_FLIPU1])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_FLIPV1])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV1])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU1])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV1])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_MT_DEPTHMODE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_DSL_ENABLE])
		{
			type = IElement::PT_BOOL; return true;
		}
		else if (name == token[TOKEN_MT_DSL_THRESHOLD])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_MT_DSL_SPEED])
		{
			type = IElement::PT_REAL; return true;
		}
		return false;
	}

	bool EffectMaterial::getPropertyValue(const String& name, String& value)
	{
		if (name == token[TOKEN_MT_BLENDMODE])
		{
			value = getBlendModeStr(); return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE])
		{
			value = mTextureName; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE])
		{
			value = getUVString(); return true;
		}
		else if (name == token[TOKEN_MT_FLIPU])
		{
			value = StringUtil::ToString(mbFlipU); return true;
		}
		else if (name == token[TOKEN_MT_FLIPV])
		{
			value = StringUtil::ToString(mbFlipV); return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV])
		{
			value = StringUtil::ToString(mbSwapUV); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU])
		{
			value = StringUtil::ToString(mUSpped); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV])
		{
			value = StringUtil::ToString(mVSpeed); return true;
		}
		else if (name == token[TOKEN_MT_RENDERPRIORITY])
		{
			value = StringUtil::ToString(mRenderPriority); return true;
		}
		else if (name == token[TOKEN_DS_ENABEL])
		{
			value = StringUtil::ToString(mDistortionEnable); return true;
		}
		else if (name == token[TOKEN_DS_TEXTURE])
		{
			value = mNoiseTexName; return true;
		}
		else if (name == token[TOKEN_DS_UVVALUE])
		{
			value = getDistortionUV(); return true;
		}
		else if (name == token[TOKEN_DS_ALPHA_THRESHOLD])
		{
			value = StringUtil::ToString(mDistortionAlphaThreshold); return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETU])
		{
			value = StringUtil::ToString(mDistortionTcOffsetU); return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETV])
		{
			value = StringUtil::ToString(mDistortionTcOffsetV); return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEU])
		{
			value = StringUtil::ToString(mDistortionTcScaleU); return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEV])
		{
			value = StringUtil::ToString(mDistortionTcScaleV); return true;
		}
		else if (name == token[TOKEN_DS_BLEND_WEIGHT])
		{
			value = StringUtil::ToString(mDistortionBlendWeight); return true;
		}
		else if (name == token[TOKEN_DS_TYPE])
		{
			value = getDistortionTypeStr(); return true;
		}
		else if (name == token[TOKEN_DS_ONLY])
		{
			value = StringUtil::ToString(mDistortionOnly); return true;
		}
		else if (name == token[TOKEN_GL_ENABLE])
		{
			value = StringUtil::ToString(mGlowEnable); return true;
		}
		else if (name == token[TOKEN_MT_UV2_ENABLE])
		{
			value = StringUtil::ToString(mbUV2Enable); return true;
		}
		else if (name == token[TOKEN_MT_UV2_RGB_BLENDMODE])
		{
			value = StringUtil::ToString(mUV2_rgb_blendModel); return true;
		}
		else if (name == token[TOKEN_MT_UV2_A_BLENDMODE])
		{
			value = StringUtil::ToString(mUV2_a_blendModel); return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE1])
		{
			value = mTextureName1; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE1])
		{
			value = StringUtil::ToString(Vector4(mTexAddrLeft1, mTexAddrTop1, mTexAddrRight1, mTexAddrBottom1));
			return true;
		}
		else if (name == token[TOKEN_MT_FLIPU1])
		{
			value = StringUtil::ToString(mbFlipU1); return true;
		}
		else if (name == token[TOKEN_MT_FLIPV1])
		{
			value = StringUtil::ToString(mbFlipV1); return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV1])
		{
			value = StringUtil::ToString(mbSwapUV1); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU1])
		{
			value = StringUtil::ToString(mUSpped1); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV1])
		{
			value = StringUtil::ToString(mVSpeed1); return true;
		}
		else if (name == token[TOKEN_MT_DEPTHMODE])
		{
			value = StringUtil::ToString(mIsTestDepth); return true;
		}
		else if (name == token[TOKEN_MT_DSL_ENABLE])
		{
			value = StringUtil::ToString(static_cast<bool>(mDissolutionParameter.x)); return true;
		}
		else if (name == token[TOKEN_MT_DSL_THRESHOLD])
		{
			value = StringUtil::ToString(mDissolutionParameter.y); return true;
		}
		else if (name == token[TOKEN_MT_DSL_SPEED])
		{
			value = StringUtil::ToString(mDissolutionSpeed); return true;
		}

		return false;
	}

	bool EffectMaterial::setPropertyValue(const String& name, const String& value)
	{
		if (name == token[TOKEN_MT_BLENDMODE])
		{
			setBlendModeStr(value); return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE])
		{
			mTextureName = value; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE])
		{
			setUVString(value); return true;
		}
		else if (name == token[TOKEN_MT_FLIPU])
		{
			mbFlipU = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_FLIPV])
		{
			mbFlipV = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV])
		{
			mbSwapUV = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU])
		{
			mUSpped = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV])
		{
			mVSpeed = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_MT_RENDERPRIORITY])
		{
			mRenderPriority = StringUtil::ParseInt(value); return true;
		}
		else if (name == token[TOKEN_DS_ENABEL])
		{
			mDistortionEnable = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_DS_TEXTURE])
		{
			mNoiseTexName = value; return true;
		}
		else if (name == token[TOKEN_DS_UVVALUE])
		{
			setDistortionUV(value); return true;
		}
		else if (name == token[TOKEN_DS_ALPHA_THRESHOLD])
		{
			mDistortionAlphaThreshold = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETU])
		{
			mDistortionTcOffsetU = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_TC_OFFSETV])
		{
			mDistortionTcOffsetV = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEU])
		{
			mDistortionTcScaleU = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_TC_SCALEV])
		{
			mDistortionTcScaleV = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_BLEND_WEIGHT])
		{
			mDistortionBlendWeight = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_DS_TYPE])
		{
			setDistortionTypeStr(value); return true;
		}
		else if (name == token[TOKEN_DS_ONLY])
		{
			mDistortionOnly = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_GL_ENABLE])
		{
			mGlowEnable = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_UV2_ENABLE])
		{
			mbUV2Enable = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_UV2_RGB_BLENDMODE])
		{
			mUV2_rgb_blendModel = StringUtil::ParseI32(value); return true;
		}
		else if (name == token[TOKEN_MT_UV2_A_BLENDMODE])
		{
			mUV2_a_blendModel = StringUtil::ParseI32(value); return true;
		}
		else if (name == token[TOKEN_MT_TEXTURE1])
		{
			mTextureName1 = value; return true;
		}
		else if (name == token[TOKEN_MT_UVVALUE1])
		{
			Vector4 vec = StringUtil::ParseVec4(value);
			mTexAddrLeft1 = vec.x;
			mTexAddrTop1 = vec.y;
			mTexAddrRight1 = vec.z;
			mTexAddrBottom1 = vec.w;
			return true;
		}
		else if (name == token[TOKEN_MT_FLIPU1])
		{
			mbFlipU1 = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_FLIPV1])
		{
			mbFlipV1 = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_SWAPUV1])
		{
			mbSwapUV1 = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDU1])
		{
			mUSpped1 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_MT_TASPEEDV1])
		{
			mVSpeed1 = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_MT_DEPTHMODE])
		{
			mIsTestDepth = StringUtil::ParseBool(value); return true;
		}
		else if (name == token[TOKEN_MT_DSL_ENABLE])
		{
			mDissolutionParameter.x = StringUtil::ParseBool(value) == true ? 1.0f:0.f; 
			return true;
		}
		else if (name == token[TOKEN_MT_DSL_THRESHOLD])
		{
			mDissolutionParameter.y = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_MT_DSL_SPEED])
		{
			mDissolutionSpeed = StringUtil::ParseReal(value); return true;
		}

		return false;
	}
}