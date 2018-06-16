#include "RenderTargetManager.h"
#include "Material.h"
#include "Engine/core/main/Engine.h"
#include "render/ShaderProgramRes.h"
#include "TextureRes.h"
#include <algorithm>

namespace Echo
{
	// 构造函数
	RenderTargetManager::RenderTargetManager()
		: m_pScreenAlignedQuadDownsampleDepth(nullptr)
		, m_pMaterialDownsampleDepth(nullptr)
		, m_pMaterialBaseUpdate(nullptr)
		, m_pMaterialNightSight(nullptr)
		, m_inUsingRenderTarget( RTI_End )
		, m_bEnableMultiResolution( false )
		, m_bEnableFilter(false)
		, m_pMaterialFilterUpdate(NULL)
		, m_logicWidthScale( 1.0f )
		, m_logicHeightScale( 1.0f )
		, m_logicWidth(0)
		, m_logicHeight(0)
		, m_filterBlendmapTexture( NULL)
		, m_pScreenAlignedQuad(nullptr)
	{
		m_preDefinedTargetIDs.reserve(RTI_PostProcess + 1);
		m_preDefinedTargetIDs.push_back("RTI_DefaultBackBuffer");
		m_preDefinedTargetIDs.push_back("RTI_SceneColorMap");
		m_preDefinedTargetIDs.push_back("RTI_LDRSceneColorMap");
		m_preDefinedTargetIDs.push_back("RTI_LDRVRSceneColorMap");
		m_preDefinedTargetIDs.push_back("RTI_CoverageMap");
		m_preDefinedTargetIDs.push_back("RTI_ShadowMap");
		m_preDefinedTargetIDs.push_back("RTI_ShadowMapVBlur");
		m_preDefinedTargetIDs.push_back("RTI_WaterRefectMap");
		m_preDefinedTargetIDs.push_back("RTI_WaterRefractionMap");
		m_preDefinedTargetIDs.push_back("RTI_FinalLDRBuffer");
		m_preDefinedTargetIDs.push_back("RTI_HalfResDepth");
		m_preDefinedTargetIDs.push_back("RTI_Glow");
		m_preDefinedTargetIDs.push_back("RTI_PostProcess");
	}

	// 析构函数
	RenderTargetManager::~RenderTargetManager()
	{
		if (!m_pScreenAlignedQuad)
			return;

		for( RenderTargetMap::iterator bit = m_mapRenderTargets.begin(); bit != m_mapRenderTargets.end();  )
		{
			EchoSafeDelete( bit->second, RenderTarget);
			m_mapRenderTargets.erase(bit ++);
		}

		EchoSafeDelete(m_pMaterialBaseUpdate, ShaderProgramRes);
		EchoSafeDelete(m_pMaterialNightSight, ShaderProgramRes);
		EchoSafeDelete(m_pMaterialDownsampleDepth, ShaderProgramRes);

		Renderer::instance()->destroyRenderables( &m_pScreenAlignedQuad, 1);

		if (m_pScreenAlignedQuadDownsampleDepth)
		{
			Renderer::instance()->destroyRenderables(&m_pScreenAlignedQuadDownsampleDepth, 1);
		}
		Renderer::instance()->destroyRenderables( &m_pScreenAlignedQuadQuarterLB, 1);

		if(m_bEnableFilter)
		{
			EchoSafeDelete(m_pMaterialFilterUpdate, ShaderProgramRes);
			Renderer::instance()->releaseTexture(m_pFilterBlendmap.m_texture);
			m_pFilterBlendmap.m_texture = NULL;
			Renderer::instance()->destroyRenderables( &m_pScreenAlignedQuadFilter, 1);
		}
	}

	// 获取渲染实例
	RenderTargetManager* RenderTargetManager::instance()
	{
		static RenderTargetManager* inst = EchoNew(RenderTargetManager);
		return inst;
	}

	// 初始化
	bool RenderTargetManager::initialize()
	{
		m_bEnableMultiResolution = true;
		ui32 screenWidth = Renderer::instance()->getScreenWidth();
		ui32 screenHeight = Renderer::instance()->getScreenHeight();

		if( m_bEnableMultiResolution )
		{
			AdaptSize( screenWidth, screenHeight, m_logicWidth, m_logicHeight);
		
			m_logicWidthScale = static_cast<float>(m_logicWidth) / static_cast<float>( screenWidth);
			m_logicHeightScale = static_cast<float>(m_logicHeight) / static_cast<float>(screenHeight);
		}

		if( storeDefaultRenderTarget() )
		{
			EchoLogInfo( "RenderTargetManager::storeDefaultRenderTarget() ... succeeded" );
		}
		else
		{
			EchoLogError( "storeDefaultRenderTarget() ... Failed ! " );
			return false;
		}

		{
			RenderTarget::Options option;
			option.depth = true;
			option.multiResolution = m_bEnableMultiResolution;
			// 创建正常屏幕尺寸的LDRMap
			if (NULL == createRenderTarget(RTI_LDRSceneColorMap, screenWidth, screenHeight, PF_RGBA16_FLOAT, option))
			{
				EchoLogError("RenderTargetManager::createRenderTarget( RTI_LDRSceneColor ) ... Failed");
				return false;
			}
		}


		if( restoreDefaultRenderTarget() )
		{
			EchoLogInfo( "RenderTargetManager::restoreDefaultRenderTarget() succeeded" );
		}
		else
		{
			EchoLogInfo( "RenderTargetManager::restoreDefaultRenderTarget() Failed !" );
			return false;
		}

		if( !createScreenAlignedQuad_ext() )
		{
			EchoLogInfo("createScreenAlignedQuad_ext() Failed !");
			return false;
		}
		
		if( !createScreenAlignedQuadQuarterLB_ext() )
		{
			EchoLogInfo("createScreenAlignedQuadQuarterLB_ext() Failed !");
			return false;
		}

		return true;
	}

	void RenderTargetManager::destroyRenderTargetByID(ui32 _id)
	{
		RenderTargetMap::iterator fit = m_mapRenderTargets.find( _id );

		if( fit == m_mapRenderTargets.end() )
		{
			EchoLogError( "Could not found RenderTarget[%d]", _id );
			return;
		}

		if (Renderer::instance()->isEnableFrameProfile())
		{
			ui32 size = (fit->second)->getMemorySize();
			Renderer::instance()->getFrameState().decrRendertargetSize(size);
		}

		EchoSafeDelete(fit->second, RenderTarget);
		m_mapRenderTargets.erase(fit);

		m_allocatedRenderTargetIDs.erase(_id);
	}

	bool RenderTargetManager::updateRenderTarget( ui32 _srcID, ui32 _dstID, bool _clear_color /* =false */, bool _clear_depth /* = false */, bool _clear_stencil /* =false */ )
	{
		EchoAssert( _srcID != RTI_End && _dstID != RTI_End && m_pMaterialBaseUpdate );

		if( _srcID == RTI_DefaultBackBuffer )
		{
			EchoLogError( "Could not support copy from default back buffer!" );
			return false;
		}

		if( !checkRenderTargetIDValid(_srcID) || !checkRenderTargetIDValid(_dstID) )
		{
			return false;
		}

		beginRenderTarget(_dstID, _clear_color, Renderer::BGCOLOR, _clear_depth, 1.0f, _clear_stencil, 0);
		m_pScreenAlignedQuad->setTexture(0, getRenderTargetByID(_srcID)->getBindTexture(), m_pMaterialBaseUpdate->getSamplerState(0));

		m_pScreenAlignedQuad->render();

		endRenderTarget(_dstID);
		return true;
	}

	bool RenderTargetManager::updateRenderTargetWithFilter( ui32 _srcID, ui32 _dstID, bool _clear_color /* =false */, bool _clear_depth /* = false */, bool _clear_stencil /* =false */ )
	{
		EchoAssert( _srcID != RTI_End && _dstID != RTI_End && m_pMaterialBaseUpdate );
		
		if( _srcID == RTI_DefaultBackBuffer )
		{
			EchoLogError( "Could not support copy from default back buffer!" );
			return false;
		}

		if( !checkRenderTargetIDValid(_srcID) || !checkRenderTargetIDValid(_dstID) )
		{
			return false;
		}

		beginRenderTarget(_dstID, _clear_color, Renderer::BGCOLOR, _clear_depth, 1.0f, _clear_stencil, 0);
		m_pScreenAlignedQuadFilter->setTexture(0, getRenderTargetByID(_srcID)->getBindTexture(), m_pMaterialBaseUpdate->getSamplerState(0));
		m_pScreenAlignedQuadFilter->setTexture(1, m_pFilterBlendmap.m_texture, m_pMaterialBaseUpdate->getSamplerState(0));
		m_pScreenAlignedQuadFilter->render();

		endRenderTarget(_dstID);
		return true;
	}

	bool RenderTargetManager::downsampleDepthTarget(ui32 srcID, ui32 dstID)
	{
		EchoAssert( srcID != RTI_End && dstID != RTI_End && m_pMaterialBaseUpdate );

		if( srcID == RTI_DefaultBackBuffer )
		{
			EchoLogError( "Could not support copy from default back buffer!" );
			return false;
		}

		if( !checkRenderTargetIDValid(srcID) || !checkRenderTargetIDValid(dstID) || !m_pScreenAlignedQuadDownsampleDepth)
		{
			return false;
		}

		beginRenderTarget(dstID, true, Renderer::BGCOLOR, true, 0.0f, true, 0);
		m_pScreenAlignedQuadDownsampleDepth->setTexture(0, getRenderTargetByID(srcID)->getDepthTexture(), m_pMaterialBaseUpdate->getSamplerState(0));
		m_pScreenAlignedQuadDownsampleDepth->render();
		endRenderTarget(dstID);
		return true;
	}

	bool RenderTargetManager::createScreenAlignedQuad_ext()
	{
		return true;

		RenderInput::VertexElementList verElementLists;
		verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_POSITION, Echo::PF_RGB32_FLOAT));
		verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TEXCOORD0, Echo::PF_RG32_FLOAT));

		float vertex_buffer[] = {-1.f, -1.f, 0.0f, 0.f,  0.f,
			1.f, -1.f, 0.0f, 1.f,  0.f,
			1.f,  1.f, 0.0f, 1.f,  1.f,
			-1.f,  1.f,0.0f, 0.f,  1.f};

		ui16 index_buffer[] = {0, 3, 2, 0, 2, 1};

		Buffer bufferVertices( sizeof(vertex_buffer), vertex_buffer );
		Buffer bufferIndices( sizeof(index_buffer), index_buffer );
		GPUBuffer* vertexGPUBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
		GPUBuffer* indexGPUBuffer = Renderer::instance()->createIndexBuffer( GPUBuffer::GBU_GPU_READ, bufferIndices );

		ShaderProgram* pShaderProgram = m_pMaterialBaseUpdate->getShaderProgram();
		m_pScreenAlignedQuad = Renderer::instance()->createRenderable(NULL, m_pMaterialBaseUpdate);

		pShaderProgram->getParamPhysicsIndex("texSampler");

		RenderInput* pRenderInput = Renderer::instance()->createRenderInput( m_pMaterialBaseUpdate->getShaderProgram());
		pRenderInput->bindVertexStream(verElementLists, vertexGPUBuffer);
		pRenderInput->bindIndexStream(indexGPUBuffer, sizeof(ui16));
		pRenderInput->setTopologyType(RenderInput::TT_TRIANGLELIST);

		//m_pScreenAlignedQuad->setRenderInput(pRenderInput);
/*
		getRenderTargetByID(RTI_SceneColorMap)->getBindTexture()->setSamplerState(m_pMaterialBaseUpdate->getSamplerStateByTexStage(0));
		m_pScreenAlignedQuad->setTexture( 0, getRenderTargetByID(RTI_SceneColorMap)->m_bindTexture );*/

		m_pScreenAlignedQuad->beginShaderParams(1);
		m_pScreenAlignedQuad->setShaderParam(pShaderProgram->getParamPhysicsIndex("texSampler"), SPT_TEXTURE, (void*)&SHADER_TEXTURE_SLOT0);
		m_pScreenAlignedQuad->endShaderParams();

		if (m_pMaterialDownsampleDepth)
		{
			m_pScreenAlignedQuadDownsampleDepth = Renderer::instance()->createRenderable(NULL, m_pMaterialDownsampleDepth);
			pRenderInput = Renderer::instance()->createRenderInput( m_pMaterialDownsampleDepth->getShaderProgram());
			pRenderInput->bindVertexStream(verElementLists, vertexGPUBuffer);
			pRenderInput->bindIndexStream(indexGPUBuffer, sizeof(ui16));
			pRenderInput->setTopologyType(RenderInput::TT_TRIANGLELIST);
			//m_pScreenAlignedQuadDownsampleDepth->setRenderInput(pRenderInput);
			m_pScreenAlignedQuadDownsampleDepth->beginShaderParams(1);
			m_pScreenAlignedQuadDownsampleDepth->setShaderParam(m_pMaterialDownsampleDepth->getShaderProgram()->getParamPhysicsIndex("texSampler"), SPT_TEXTURE, (void*)&SHADER_TEXTURE_SLOT0);
			m_pScreenAlignedQuadDownsampleDepth->endShaderParams();
		}


		if(m_bEnableFilter)
		{
			GPUBuffer* vertexGPUBuffer_filter = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
			GPUBuffer* indexGPUBuffer_filter = Renderer::instance()->createIndexBuffer( GPUBuffer::GBU_GPU_READ, bufferIndices );

			ShaderProgram* pShaderProgram_filter = m_pMaterialFilterUpdate->getShaderProgram();
			pShaderProgram_filter->getParamPhysicsIndex("sceneColorSampler");
			pShaderProgram_filter->getParamPhysicsIndex("FilterAdditionalSampler");

			RenderInput* pRenderInput_filter = Renderer::instance()->createRenderInput( m_pMaterialFilterUpdate->getShaderProgram());
			pRenderInput->bindVertexStream( verElementLists, vertexGPUBuffer_filter);
			pRenderInput->bindIndexStream( indexGPUBuffer_filter, sizeof(ui16));
			pRenderInput_filter->setTopologyType(RenderInput::TT_TRIANGLELIST);

			m_pScreenAlignedQuadFilter = Renderer::instance()->createRenderable(NULL, m_pMaterialFilterUpdate);
			//m_pScreenAlignedQuadFilter->setRenderInput(pRenderInput_filter);

			m_FilterTex[0] = 0;
			m_FilterTex[1] = 1;
			m_pScreenAlignedQuadFilter->beginShaderParams(2);
			m_pScreenAlignedQuadFilter->setShaderParam(pShaderProgram_filter->getParamPhysicsIndex("sceneColorSampler"), SPT_TEXTURE, &m_FilterTex[0]);
			m_pScreenAlignedQuadFilter->setShaderParam(pShaderProgram_filter->getParamPhysicsIndex("FilterAdditionalSampler"), SPT_TEXTURE, &m_FilterTex[1]);
			m_pScreenAlignedQuadFilter->endShaderParams();
		}

		return true;

	}

	void RenderTargetManager::changeFilterBlendmapName(const String& texName)
	{
		if(!m_bEnableFilter)
			return;

		EchoAssert(m_pMaterialFilterUpdate);
		
		if(texName != m_pFilterBlendmapName)
		{
			m_pFilterBlendmapName = texName;

			if(m_pFilterBlendmap.m_texture)
			{
				Renderer::instance()->releaseTexture( m_pFilterBlendmap.m_texture);
				m_pFilterBlendmap.m_texture = NULL;
			}

			m_filterBlendmapTexture = TextureRes::createTexture(m_pFilterBlendmapName);
			m_filterBlendmapTexture->prepareLoad();
			m_pFilterBlendmap.m_texture = m_filterBlendmapTexture->getTexture();

			m_pFilterBlendmap.m_samplerState = m_pMaterialFilterUpdate->getSamplerState(1);
		}
	}

	// 开始渲染目标
	bool RenderTargetManager::beginRenderTarget( ui32 _id,bool _clearColor, const Color& _backgroundColor, bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue, ui32 rbo)
	{
		EchoAssert( _id != RTI_End );

		RenderTarget* pRT = getRenderTargetByID( _id );

		RenderTarget* pUsingRT = 0;

		if(m_inUsingRenderTarget != RTI_End)
		{
			pUsingRT = getRenderTargetByID( m_inUsingRenderTarget );
		}

		EchoAssert( pRT );

		if( m_inUsingRenderTarget != _id )
		{
			pRT->m_bFrameBufferChange = true;
		}
		else
		{
			pRT->m_bFrameBufferChange = false;
		}

		if( m_inUsingRenderTarget == RTI_End )
		{
			pRT->m_bViewportChange = true;
		}
		else if( pUsingRT && ( pUsingRT->width() != pRT->width() || pUsingRT->height() != pRT->height() ) )
		{
			pRT->m_bViewportChange = true;
		}
		else
		{
			pRT->m_bViewportChange = false;
		}

		m_inUsingRenderTarget = _id;

		// 切换RenderTarget，dirty 状态机纹理槽信息
		Renderer::instance()->dirtyTexSlot();

		return doBeginRenderTarget( _id, _clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, _clearStencil );
	}

	bool RenderTargetManager::createScreenAlignedQuadQuarterLB_ext()
	{
		return true;

		RenderInput::VertexElementList verElementLists;
		verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_POSITION, Echo::PF_RGB32_FLOAT));
		verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TEXCOORD0, Echo::PF_RG32_FLOAT));

		float vertex_buffer[] = 
		{-1.f, -1.f, 0.0f, 0.f,  0.f,
		/*1.f*/0.0f, -1.f, 0.0f, 1.f,  0.f,
		/*1.f*/0.0f, /*1.f*/0.0f, 0.0f, 1.f,  1.f,
		-1.f,  /*1.f*/0.0f, 0.0f, 0.f,  1.f};


		ui16 index_buffer[] = {0, 3, 2, 0, 2, 1};


		Buffer bufferVertices( sizeof(vertex_buffer), vertex_buffer );
		Buffer bufferIndices( sizeof(index_buffer), index_buffer );
		GPUBuffer* vertexGPUBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
		GPUBuffer* indexGPUBuffer = Renderer::instance()->createIndexBuffer( GPUBuffer::GBU_GPU_READ, bufferIndices );

		ShaderProgram* pShaderProgram = m_pMaterialBaseUpdate->getShaderProgram();

		RenderInput* pRenderInput = Renderer::instance()->createRenderInput( m_pMaterialBaseUpdate->getShaderProgram());
		pRenderInput->bindVertexStream( verElementLists, vertexGPUBuffer);
		pRenderInput->bindIndexStream(indexGPUBuffer, sizeof(ui16));
		pRenderInput->setTopologyType(RenderInput::TT_TRIANGLELIST);

		m_pScreenAlignedQuadQuarterLB = Renderer::instance()->createRenderable(NULL, m_pMaterialBaseUpdate);
		//m_pScreenAlignedQuadQuarterLB->setRenderInput(pRenderInput);
		m_pScreenAlignedQuadQuarterLB->beginShaderParams(1);
		m_pScreenAlignedQuadQuarterLB->setShaderParam(pShaderProgram->getParamPhysicsIndex("texSampler"), SPT_TEXTURE, (void*)&SHADER_TEXTURE_SLOT0);
		m_pScreenAlignedQuadQuarterLB->endShaderParams();

		return true;
	}

	void RenderTargetManager::renderScreenAlignedQuad()
	{
		m_pScreenAlignedQuad->render();
	}

	void RenderTargetManager::renderScreenAlignedQuadQuarterLB()
	{
		m_pScreenAlignedQuadQuarterLB->render();
	}

	// 当屏幕大小改变时调用 
	void RenderTargetManager::onScreensizeChanged( ui32 _width, ui32 _height )
	{
		if (m_bEnableMultiResolution)
		{
			AdaptSize(_width, _height, m_logicWidth, m_logicHeight);

			m_logicWidthScale = static_cast<float>(m_logicWidth) / static_cast<float>(_width);
			m_logicHeightScale = static_cast<float>(m_logicHeight) / static_cast<float>(_height);
		}

		// 遍历所有(阴影图除外)
		for (RenderTargetMap::iterator bit = m_mapRenderTargets.begin(); bit != m_mapRenderTargets.end(); ++bit)
		{
			RenderTarget* pRenderTarget = bit->second;
			if (pRenderTarget)
			{
				if (Renderer::instance()->isEnableFrameProfile())
				{
					ui32 size = pRenderTarget->getMemorySize();
					Renderer::instance()->getFrameState().decrRendertargetSize(size);
				}
				switch (pRenderTarget->id())
				{
				case RTI_DefaultBackBuffer: pRenderTarget->onResize(_width, _height); break;
				case RTI_LDRSceneColorMap:
					pRenderTarget->onResize(ui32(_width * m_logicWidthScale), ui32(_height * m_logicHeightScale));
					break;
				default:
					if (pRenderTarget->resolutionRelative())
					{
						float scale = pRenderTarget->scaleFactor();
						pRenderTarget->onResize(static_cast<ui32>(_width * m_logicWidthScale * scale), static_cast<ui32>(_height * m_logicHeightScale * scale));
					}
					break;
				}
				if (Renderer::instance()->isEnableFrameProfile())
				{
					ui32 size = pRenderTarget->getMemorySize();
					Renderer::instance()->getFrameState().incrRendertargetSize(size);
				}
			}
		}
	}

	ui32 RenderTargetManager::allocateRenderTargetID(const String& name)
	{
		ui32 begin = RTI_POSTIMAGEEFFECT_BEGIN;
		ui32 end = RTI_POSTIMAGEEFFECT_END;
		EchoAssertX(m_allocatedRenderTargetIDs.size() < end - begin, "Named RenderTarget out of range !!!");
		for (size_t i = begin; i < end; ++i)
		{
			if (m_allocatedRenderTargetIDs.find(i) == m_allocatedRenderTargetIDs.end())
			{
				m_allocatedRenderTargetIDs.insert(std::make_pair(i, name));
				return i;
			}
		}
		EchoAssertX(false, "reach unexpected code path.");
		return RTI_End;
	}

	ui32 RenderTargetManager::getRenderTargetID(const String& name) const
	{
		
		for (const auto& pair : m_allocatedRenderTargetIDs)
		{
			if (pair.second == name)
			{
				return pair.first;
			}
		}
		for (auto it = m_preDefinedTargetIDs.begin(); it != m_preDefinedTargetIDs.end(); ++it)
		{
			if (*it == name)
			{
				return it - m_preDefinedTargetIDs.begin();
			}
		}
		return RTI_End;
	}

	// 根据名称获取渲染目标
	RenderTarget* RenderTargetManager::getRenderTarget(const String& name)
	{
		return getRenderTargetByID(getRenderTargetID(name));
	}

	// 调整大小
	void RenderTargetManager::AdaptSize(ui32 nWidth, ui32 nHeight, ui32& outWidth, ui32& outHeight)
	{
		outWidth = static_cast<ui32>(nWidth);
		outHeight = static_cast<ui32>(nHeight);
	}

	// 创建渲染目标
	RenderTarget* RenderTargetManager::doCreateRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, RenderTarget::Options option)
	{
		RenderTargetMap::iterator iter = m_mapRenderTargets.find(_id);
		if (iter != m_mapRenderTargets.end())
		{
			EchoLogError("Rendertarget [%d] is already created.", _id);
			return iter->second;
		}

		if (_pixelFormat == PF_RGBA16_FLOAT && !Renderer::instance()->getDeviceFeatures().supportHFColorBf1())
		{
			_pixelFormat = PF_RGBA8_UNORM;
			EchoLogInfo("Device could not support PF_RGBA16_FLOAT(GL_EXT_color_buffer_half_float) use PF_RGBA8_UNORM!(Rendertarget[%d])",_id);
		}

		RenderTarget* pNewRenderTarget = Renderer::instance()->createRenderTarget(_id, _width, _height, _pixelFormat, option);

		if (!pNewRenderTarget)
		{
			EchoLogError("Allocate RenderTarget Failed !");
			return NULL;
		}

		if (option.depthTarget)
		{
			pNewRenderTarget->reusageDepthTarget(option.depthTarget);
		}

		if (!pNewRenderTarget->create())
		{
			EchoLogError("RenderTarget::create Failed !");
			EchoSafeDelete(pNewRenderTarget, RenderTarget);
			return NULL;
		}

		m_mapRenderTargets.insert(RenderTargetMap::value_type(_id, pNewRenderTarget));

		if (Renderer::instance()->isEnableFrameProfile())
		{
			ui32 size = pNewRenderTarget->getMemorySize();
			Renderer::instance()->getFrameState().incrRendertargetSize(size);
		}

		return pNewRenderTarget;
	}

	bool RenderTargetManager::doStoreDefaultRenderTarget()
	{
		RenderTarget::Options option;
		option.depth = true;
		RenderTarget* pDefaultRT = Renderer::instance()->createRenderTarget( RTI_DefaultBackBuffer, Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight(), Renderer::instance()->getBackBufferPixelFormat(), option);
		if (!pDefaultRT)
		{
			EchoLogError("EchoNew( RTI_DefaultBackBuffer ) Failed !");
			return false;
		}

		pDefaultRT->doStoreDefaultRenderTarget();
		m_mapRenderTargets.insert(RenderTargetMap::value_type(RTI_DefaultBackBuffer, pDefaultRT));

		return true;
	}

	bool RenderTargetManager::doRestoreDefaultRenderTarget()
	{
		RenderTarget* pDefaultRT = getRenderTargetByID(RTI_DefaultBackBuffer);
		if (!pDefaultRT)
		{
			EchoLogError("Could not found RenderTarget[RTI_DefaultBackBuffer]");
			return false;
		}

		pDefaultRT->doRestoreDefaultRenderTarget();

		return true;
	}

	bool RenderTargetManager::doBeginRenderTarget(ui32 _id, bool _clearColor, const Color& _backgroundColor, bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue)
	{
		return getRenderTargetByID(_id)->beginRender(_clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, stencilValue);
	}

	bool RenderTargetManager::doEndRenderTarget(ui32 _id)
	{
		//		EchoAssert( m_inUsingRenderTarget = RTI_End );

		return getRenderTargetByID(_id)->endRender();
	}
	
	bool RenderTargetManager::doInvalidateFrameBuffer(ui32 _id, bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
	{
		//		EchoAssert( m_inUsingRenderTarget = RTI_End );

		//return getRenderTargetByID(_id)->invalidateFrameBuffer(invalidateColor, invalidateDepth, invalidateStencil);
		return true;
	}
}
