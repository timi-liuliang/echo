#include "ShadowMapRenderStage.h"
#include <engine/core/render/render/Material.h>
#include "Engine/core/Scene/NodeTree.h"
#include "Engine/core/Render/RenderTargetManager.h"
#include "Engine/core/Render/MaterialInst.h"
#include "Engine/core/main/EngineSettings.h"
#include "render/RenderQueueGroup.h"

namespace Echo
{
	static const i32 s_ShadowMapEdge = 2;

	ShadowMapRenderStage::ShadowMapRenderStage()
		: RenderStage(RSI_ShadowMap)
		, m_pScreenAlignedQuadIB(NULL)
		, m_pScreenAlignedQuadVB(NULL)
		, m_shadowMapSamplerState(NULL)
		, m_bNeedBlur(false)
		, m_shadowShade(0.0f)
		, m_init(false)
		, m_isShadowCameraEnable(false)
	{
		m_shadowMapSize = EngineSettingsMgr::instance()->getShadowMapSize();
	}

	ShadowMapRenderStage::~ShadowMapRenderStage()
	{
		destroy();
	}

	bool ShadowMapRenderStage::initialize()
	{
		// 阴影图渲染目标
		RenderTarget::Options option;
		option.depth = true;
		RenderTarget* target = RenderTargetManager::instance()->createRenderTarget(RTI_ShadowMap, m_shadowMapSize, m_shadowMapSize, PF_RGBA8_UNORM, option);
		if (target)
		{
			SamplerState::SamplerDesc desc;
			desc.addrUMode = SamplerState::AM_CLAMP;
			desc.addrVMode = SamplerState::AM_CLAMP;
			desc.addrWMode = SamplerState::AM_CLAMP;
			m_shadowMapSamplerState = Renderer::instance()->getSamplerState(desc);

			// 初始化清理
			RenderTargetManager::instance()->beginRenderTarget(RTI_ShadowMap, true, Color::WHITE);
			RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);
		}
		else
		{
			EchoLogError("RenderTargetManager::createRenderTarget( m_shadowMapSamplerState ) ... Failed");
			return false;
		}

		target = RenderTargetManager::instance()->createRenderTarget(RTI_ShadowMapVBlur, m_shadowMapSize, m_shadowMapSize, PF_RGBA8_UNORM, option);
		if (!target)
		{
			EchoLogError("RenderTargetManager::createRenderTarget( RTI_ShadowMapVBlur ) ... Failed");
			return false;
		}

		m_pMtlShadowMapHBlurPass = EchoNew(Material);
		EchoAssert(m_pMtlShadowMapHBlurPass);
		m_pMtlShadowMapHBlurPass->loadFromFile("pp_shadowMapHBlur.xml", "");

		m_pMtlShadowMapVBlurPass = EchoNew(Material);
		EchoAssert(m_pMtlShadowMapVBlurPass);
		m_pMtlShadowMapVBlurPass->loadFromFile("pp_shadowMapVBlur.xml", "");

		m_verElementLists.clear();
		m_verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_POSITION, Echo::PF_RGB32_FLOAT));
		m_verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TEXCOORD0, Echo::PF_RG32_FLOAT));

		// 顶点数据
		float vertex_buffer[] = {
			-1.f, -1.f, 0.f, 0.f, 0.f,
			1.f, -1.f, 0.f, 1.f, 0.f,
			1.f, 1.f, 0.f, 1.f, 1.f,
			-1.f, 1.f, 0.f, 0.f, 1.f
		};

		ui16 index_buffer[] = { 0, 3, 2, 0, 2, 1 };

		Buffer bufferVertices(sizeof(vertex_buffer), vertex_buffer);
		Buffer bufferIndices(sizeof(index_buffer), index_buffer);
		m_pScreenAlignedQuadVB = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
		m_pScreenAlignedQuadIB = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, bufferIndices);

		// shader
		RenderInput*	 pRenderInput = 0;
		ShaderProgram*	 pShaderProgram = 0;

		pShaderProgram = m_pMtlShadowMapVBlurPass->getShaderProgram();
		m_pRaShadowMapVBlurPass = Renderer::instance()->createRenderable(NULL, m_pMtlShadowMapVBlurPass);

		pRenderInput = Renderer::instance()->createRenderInput(m_pMtlShadowMapVBlurPass->getShaderProgram());
		pRenderInput->bindVertexStream(m_verElementLists, m_pScreenAlignedQuadVB);
		pRenderInput->bindIndexStream(m_pScreenAlignedQuadIB, sizeof(ui16));
		pRenderInput->setTopologyType(RenderInput::TT_TRIANGLELIST);

		static float texelSize[2] = { 1.f / m_shadowMapSize, 1.f / m_shadowMapSize };
		//m_pRaShadowMapVBlurPass->setRenderInput(pRenderInput);
		m_pRaShadowMapVBlurPass->beginShaderParams(2);
		m_pRaShadowMapVBlurPass->setShaderParam(pShaderProgram->getParamPhysicsIndex("texSampler"), SPT_TEXTURE, (void*)&SHADER_TEXTURE_SLOT0);
		m_pRaShadowMapVBlurPass->setShaderParam(pShaderProgram->getParamPhysicsIndex("texelSize"), SPT_VEC2, (void*)texelSize);
		m_pRaShadowMapVBlurPass->endShaderParams();

		pShaderProgram = m_pMtlShadowMapHBlurPass->getShaderProgram();
		m_pRaShadowMapHBlurPass = Renderer::instance()->createRenderable(NULL, m_pMtlShadowMapHBlurPass);

		pRenderInput = Renderer::instance()->createRenderInput(m_pMtlShadowMapHBlurPass->getShaderProgram());
		pRenderInput->bindVertexStream(m_verElementLists, m_pScreenAlignedQuadVB);
		pRenderInput->bindIndexStream(m_pScreenAlignedQuadIB, sizeof(ui16));
		pRenderInput->setTopologyType(RenderInput::TT_TRIANGLELIST);

		//m_pRaShadowMapHBlurPass->setRenderInput(pRenderInput);
		m_pRaShadowMapHBlurPass->beginShaderParams(2);
		m_pRaShadowMapHBlurPass->setShaderParam(pShaderProgram->getParamPhysicsIndex("texSampler"), SPT_TEXTURE, (void*)&SHADER_TEXTURE_SLOT0);
		m_pRaShadowMapHBlurPass->setShaderParam(pShaderProgram->getParamPhysicsIndex("texelSize"), SPT_VEC2, (void*)texelSize);
		m_pRaShadowMapHBlurPass->endShaderParams();

		m_init = true;

		return true;
	}

	// 销毁
	void ShadowMapRenderStage::destroy()
	{
		RenderTargetManager::instance()->destroyRenderTargetByID(RTI_ShadowMap);
		RenderTargetManager::instance()->destroyRenderTargetByID(RTI_ShadowMapVBlur);

		EchoSafeDelete(m_pMtlShadowMapVBlurPass, Material);
		Renderer::instance()->destroyRenderables(&m_pRaShadowMapVBlurPass, 1);

		EchoSafeDelete(m_pMtlShadowMapHBlurPass, Material);
		Renderer::instance()->destroyRenderables(&m_pRaShadowMapHBlurPass, 1);

		EchoSafeDelete(m_pScreenAlignedQuadVB, GPUBuffer);
		EchoSafeDelete(m_pScreenAlignedQuadIB, GPUBuffer);

		m_init = false;
	}

	void ShadowMapRenderStage::setShadowMapSize(ui32 size)
	{
		// 防止误传过小的值，导致错误
		if (size < 64 || m_shadowMapSize == size)
			return;

		destroy();
		m_shadowMapSize = size;
		initialize();
	}

	void ShadowMapRenderStage::enableBlurShadow(bool isEnable)
	{
		m_bNeedBlur = isEnable;
	}

	bool ShadowMapRenderStage::isEnableBlurShadow() const
	{
		return m_bNeedBlur;
	}

	// 执行
	void ShadowMapRenderStage::render()
	{
		bool isShadowCameraEnable = NodeTree::instance()->getShadowCamera().isEnable();
		if (m_bEnable && m_init && isShadowCameraEnable)
		{
			m_isShadowCameraEnable = isShadowCameraEnable;
			// 设置渲染目标
			RenderTargetManager::instance()->beginRenderTarget(RTI_ShadowMap, true, Color::WHITE);
			m_renderViewPort.resize(s_ShadowMapEdge, s_ShadowMapEdge, m_shadowMapSize - s_ShadowMapEdge * 2, m_shadowMapSize - s_ShadowMapEdge * 2);
			Renderer::instance()->setViewport(&m_renderViewPort);

			int beginIdx = RenderQueueGroup::instance()->getRenderQueueIndex("ShadowMap") + 1;
			int endIdx = RenderQueueGroup::instance()->getRenderQueueIndex("/ShadowMap");
			// 判断是否有投射阴影的物体
			if (beginIdx == endIdx)
			{
				m_renderViewPort.resize(0, 0, m_shadowMapSize, m_shadowMapSize);
				Renderer::instance()->setViewport(&m_renderViewPort);
				RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);
				return;
			}
			bool needRet = false;
			for (int i = beginIdx; i < endIdx; i++)
			{
				RenderQueue* renderQueue = RenderQueueGroup::instance()->getRenderQueueByIndex(i);
				if (!renderQueue->getRenderableCount())
				{
					needRet = true;
				}
				else
				{
					needRet = false;
					break;
				}
			}
			if (needRet)
			{
				m_renderViewPort.resize(0, 0, m_shadowMapSize, m_shadowMapSize);
				Renderer::instance()->setViewport(&m_renderViewPort);
				RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);
				return;
			}

			// 执行渲染
			// 在RenderQueueDefine.xml中配置好了
			// 			StringArray qName;
			// 			qName.push_back("StaticMeshForActorShodowMap");
			// 			qName.push_back("SkinMeshForActorShodowMap");
			for (int i = beginIdx; i < endIdx; i++)
			{
				RenderQueue* renderQueue = RenderQueueGroup::instance()->getRenderQueueByIndex(i);
				if (renderQueue)
				{
					renderQueue->render();
					renderQueue->clear();
				}
			}

			m_renderViewPort.resize(0, 0, m_shadowMapSize, m_shadowMapSize);
			Renderer::instance()->setViewport(&m_renderViewPort);
			RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);

			if (m_bNeedBlur)
			{
				// blur(Vertical)
				RenderTargetManager::instance()->beginRenderTarget(RTI_ShadowMapVBlur, true, Renderer::BGCOLOR, false);

				RenderTarget* pSrcRt = RenderTargetManager::instance()->getRenderTargetByID(RTI_ShadowMap);
				Texture*      pSrcTex = pSrcRt->getBindTexture();

				const SamplerState* samplerState = m_pMtlShadowMapVBlurPass->getSamplerState(0);
				m_pRaShadowMapVBlurPass->setTexture(0, pSrcTex, samplerState);

				m_pRaShadowMapVBlurPass->render();

				RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMapVBlur);

				// H
				RenderTargetManager::instance()->beginRenderTarget(RTI_ShadowMap, true, Renderer::BGCOLOR, false);

				pSrcRt = RenderTargetManager::instance()->getRenderTargetByID(RTI_ShadowMapVBlur);

				pSrcTex = pSrcRt->getBindTexture();
				m_pRaShadowMapHBlurPass->setTexture(0, pSrcTex, m_pMtlShadowMapHBlurPass->getSamplerState(0));

				m_pRaShadowMapHBlurPass->render();

				RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);
				RenderTargetManager::instance()->invalidateFrameBuffer(RTI_ShadowMapVBlur, false, true, true);
				RenderTargetManager::instance()->invalidateFrameBuffer(RTI_ShadowMap, false, true, true);
			}
		}
		else
		{
			// 第一次失效，清空原图
			if (!isShadowCameraEnable && m_isShadowCameraEnable)
			{
				m_isShadowCameraEnable = isShadowCameraEnable;
				RenderTargetManager::instance()->beginRenderTarget(RTI_ShadowMap, true, Color::WHITE);
				RenderTargetManager::instance()->endRenderTarget(RTI_ShadowMap);
			}
		}
	}
}
