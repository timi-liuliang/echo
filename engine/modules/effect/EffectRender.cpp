#include "EffectRender.h"
#include "Render/Renderer.h"
#include "Render/GPUBuffer.h"
#include "EffectMaterial.h"
#include "EffectSystemManager.h"

namespace Echo
{
	template<> HashNode<EffectRenderable>** PtrMonitor<EffectRenderable>::m_ptrs = NULL;
	template<> int PtrMonitor<EffectRenderable>::m_hashcount = 0;
	template<> unsigned int PtrMonitor<EffectRenderable>::m_maxhash = 10;

	// 构造函数
	EffectRenderable::EffectRenderable()
		: m_Texture(NULL)
		, m_RenderGroup(ERG_GROUPUP)
		, m_BlendMode(AlphaLightMode)
		, m_distortion(false)
		, m_distortionAlphaThreshold(0.5f)
		, m_distortionTexPtr(NULL)
		, m_onlyDistortionLayer(false)
		, m_timeSinceCreate(0)
		, m_idleTime(0)
		, m_bUseUV2(false)
		, m_Texture1(NULL)
		, m_UV2rgbBlendModel(0)
		, m_UV2aBlendModel(0)
		, m_GlowEnable(false)
		, m_isUiEffect(false)
		, m_isTestDepth(false)
		, m_DissolutionParameter(NULL)
	{
	}

	// 析构函数
	EffectRenderable::~EffectRenderable()
	{
		TextureResManager::instance()->releaseResource(m_distortionTexPtr);
		TextureResManager::instance()->releaseResource(m_Texture);
		TextureResManager::instance()->releaseResource(m_Texture1);

		destroyVertexPool();
		destroyIndexPool();
	}

	// 创建顶点数据池
	void EffectRenderable::createVertexPool(size_t verticesNum)
	{
		m_vertexBufferData.reserve(verticesNum);
	}

	// 创建索引池
	void EffectRenderable::createIndexPool(size_t indicesNum)
	{
		m_indexBufferData.reserve(indicesNum);
	}

	// 更新
	void EffectRenderable::tick(ui32 elapsedTime)
	{
		m_idleTime += elapsedTime;
	}

	// 锁定
	bool EffectRenderable::getLockPtr(size_t vertexNum, size_t indexNum, ui16& vertexBegin, EffectVertexFmt*& vertexAddr, ui16*& indexAddr)
	{
		if (vertexNum == 0 || indexNum == 0)
			return false;

		m_idleTime = 0;

		size_t vSize = m_vertexBufferData.size();
		size_t iSize = m_indexBufferData.size();

		// 预判空间使用情况，使用自己的自增策略
		if (vSize + vertexNum > m_vertexBufferData.capacity())
		{
			size_t n = vertexNum / 1024;
			m_vertexBufferData.reserve(vSize + 1024 * (n + 1));
		}

		if (iSize + indexNum > m_indexBufferData.capacity())
		{
			size_t n = indexNum / 1024;
			m_indexBufferData.reserve(iSize + 2048 * (n + 1));
		}

		// 顶点数据修改
		vertexBegin = static_cast<ui16>(vSize);
		m_vertexBufferData.resize(vSize + vertexNum);
		vertexAddr = &m_vertexBufferData[vSize];

		m_indexBufferData.resize(iSize + indexNum);
		indexAddr = &m_indexBufferData[iSize];

		return true;
	}

	// 开始多边形
	void EffectRenderable::clearGeometryData()
	{
		m_vertexBufferData.clear();
		m_indexBufferData.clear();
	}

	// 回退
	void EffectRenderable::reversePtr(size_t vertexNum, size_t indexNum)
	{
		if (vertexNum > 0)
		{
			EchoAssert(m_vertexBufferData.size() >= vertexNum);
			m_vertexBufferData.resize(m_vertexBufferData.size() - vertexNum);
		}

		if (indexNum > 0)
		{
			EchoAssert(m_indexBufferData.size() >= indexNum);
			m_indexBufferData.resize(m_indexBufferData.size() - indexNum);
		}
	}

	void EffectRenderable::destroyVertexPool()
	{
		m_vertexBufferData.clear();
	}

	void EffectRenderable::destroyIndexPool()
	{
		m_indexBufferData.clear();
	}

	void EffectRenderable::setMaterialParam(TextureRes* texture, EFFECT_RENDER_GROUP group, EFFECT_BLEND_MODE blendmode, EffectMaterial* pEm)
	{
		m_Texture = texture;
		m_Texture->addRefrenceCount();
		m_RenderGroup = group;
		m_BlendMode = blendmode;

		m_GlowEnable = pEm->getGlowEnable();

		m_texSampler = pEm->getTextureSampler();
		m_distortion = pEm->getDistortionEnable();
		pEm->getDistortionUVScaleOffset(m_distortionUVScaleOffset);
		m_distortionAlphaThreshold = pEm->getDistortionAlphaThreshold();
		m_onlyDistortionLayer = pEm->getOnlyDistortionLayer();
		EchoAssert(!m_distortionTexPtr);
		m_distortionTexPtr = pEm->getDistortionTexturePtr();
		if (m_distortionTexPtr)
		{
			m_distortionTexPtr->addRefrenceCount();
			m_distortionBlendWeight = pEm->getDistortionBlendWeight();
		}

		m_centerUV = pEm->getCenterUV();
		m_distortionType = pEm->getDistortionType();
		m_bUseUV2 = pEm->getUV2Enable();
		m_UV2rgbBlendModel = pEm->getUV2_RGB_BlendMode();
		m_UV2aBlendModel = pEm->getUV2_A_BlendMode();
		m_DissolutionParameter = pEm->getDissolutionParameter();
		m_Texture1 = pEm->getTexturePtr1();
		if (m_Texture1)
		{
			m_Texture1->addRefrenceCount();
		}
		m_texSampler1 = pEm->getTextureSampler1();

		m_isUiEffect = pEm->getIsUIEffect();
		m_isTestDepth = pEm->isTestDepth();
	}

	void EffectRenderable::refreshMaterialParam(EffectMaterial* pEm)
	{
		m_DissolutionParameter = pEm->getDissolutionParameter();
	}

	bool EffectRenderable::isDistortion() const
	{
		return m_distortion;
	}

	const Vector4& EffectRenderable::getDistortionUVScaleOffset() const
	{
		return m_distortionUVScaleOffset;
	}

	const float& EffectRenderable::getDistortionAlphaThreshold() const
	{
		return m_distortionAlphaThreshold;
	}

	bool EffectRenderable::getOnyDistortionLayer() const
	{
		return m_onlyDistortionLayer;
	}

	TextureRes* EffectRenderable::getDistortionTexturePtr() const
	{
		return m_distortionTexPtr;
	}

	const float EffectRenderable::getDistortionBlendWeight() const
	{
		return m_distortionBlendWeight;
	}

	const Vector2 EffectRenderable::getCenterUV() const
	{
		return m_centerUV;
	}

	const EFFECT_DISTORTION_TYPE EffectRenderable::getDistortionType() const
	{
		return m_distortionType;
	}

	Vector3* EffectRenderable::getUV2BlendMode()
	{
		static Vector3 blendMode[2];
		//相乘, 相加, 图1, 图2, 2倍相加
		switch (m_UV2rgbBlendModel)
		{
		case 0:
			blendMode[0] = Vector3(1.f, 0.f, 0.f);
			break;
		case 1:
			blendMode[0] = Vector3(0.f, 1.f, 1.f);
			break;
		case 2:
			blendMode[0] = Vector3(0.f, 1.f, 0.f);
			break;
		case 3:
			blendMode[0] = Vector3(0.f, 0.f, 1.f);
			break;
		case 4:
			blendMode[0] = Vector3(0.f, 1.f, 2.f);
			break;
		default:
			blendMode[0] = Vector3(1.f, 0.f, 0.f);
			break;
		}

		switch (m_UV2aBlendModel)
		{
		case 0:
			blendMode[1] = Vector3(1.f, 0.f, 0.f);
			break;
		case 1:
			blendMode[1] = Vector3(0.f, 1.f, 1.f);
			break;
		case 2:
			blendMode[1] = Vector3(0.f, 1.f, 0.f);
			break;
		case 3:
			blendMode[1] = Vector3(0.f, 0.f, 1.f);
			break;
		default:
			blendMode[1] = Vector3(1.f, 0.f, 0.f);
			break;
		}

		return blendMode;
	}
}