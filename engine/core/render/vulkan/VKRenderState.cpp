#include "GLES2RenderStd.h"
#include "GLES2RenderState.h"
#include "GLES2Mapping.h"
#include "GLES2Texture.h"
#include "Render/RenderThread.h"
#include "GLES2RenderStateTasks.h"
#include "Render/Renderer.h"
#include "Render/RenderTask.h"
#include "Foundation/Util/Exception.h"

namespace LORD
{
	/////////////////////////////////////////////////////////////////////////////////////////
	GLES2BlendState::GLES2BlendState()
	{
		create();
	}

	GLES2BlendState::GLES2BlendState(const BlendDesc& desc)
		: BlendState(desc)
	{
		create();
	}

	GLES2BlendState::~GLES2BlendState()
	{
	}

	void GLES2BlendState::active()
	{
		BlendStateParams blendParams;
		memset(&blendParams, 0, sizeof(blendParams));
		blendParams.isA2CEnable = m_desc.bA2CEnable;
		blendParams.isBlendEnable = m_desc.bBlendEnable;
		blendParams.blend_op = m_glBlendOP;
		blendParams.alpha_blend_op = m_glAlphaBlendOP;
		blendParams.src_blend = m_glSrcBlend;
		blendParams.dst_blend = m_glDstBlend;
		blendParams.src_alpha_blend = m_glSrcAlphaBlend;
		blendParams.dst_alpha_blend = m_glDstAlphaBlend;
		blendParams.red_mask = m_glRedMask;
		blendParams.green_mask = m_glGreenMask;
		blendParams.blue_mask = m_glBlueMask;
		blendParams.alpha_mask = m_glAlphaMask;
		blendParams.blendFactor = m_desc.blendFactor;

		BlendState* pCurState = Renderer::instance()->getBlendState();
		if(pCurState)
		{
			const BlendDesc& currDesc = pCurState->getDesc();

			if (m_desc.bA2CEnable != currDesc.bA2CEnable)
			{
				blendParams.isChangeA2C = true;
			}

			if (m_desc.bBlendEnable != currDesc.bBlendEnable)
			{
				blendParams.isChangeBlendEnable = true;
			}

			if (m_desc.blendOP != currDesc.blendOP)
			{
				blendParams.isChangeBlendOp = true;
			}

			if ((m_desc.srcBlend != currDesc.srcBlend) ||
				(m_desc.dstBlend != currDesc.dstBlend) ||
				(m_desc.srcAlphaBlend != currDesc.srcAlphaBlend) ||
				(m_desc.dstAlphaBlend != currDesc.dstAlphaBlend))
			{
				blendParams.isChangeBlendFunc = true;
			}

			if (m_desc.colorWriteMask != currDesc.colorWriteMask)
			{
				blendParams.isChangeColorWriteMask = true;
			}

			if (m_desc.blendFactor != currDesc.blendFactor)
			{
				blendParams.isChangeBlendFactor = true;
			}
		}
		else
		{
			blendParams.isChangeA2C = true;
			blendParams.isChangeBlendEnable = true;
			blendParams.isChangeBlendOp = true;
			blendParams.isChangeBlendFunc = true;
			blendParams.isChangeColorWriteMask = true;
			blendParams.isChangeBlendFactor = true;
		}

		TRenderTask<GLES2StateTaskActiveBlend>::CreateTask(blendParams);
	}

	void GLES2BlendState::create()
	{
		m_glBlendOP = GLES2Mapping::MapBlendOperation(m_desc.blendOP);
		m_glAlphaBlendOP = GLES2Mapping::MapBlendOperation(m_desc.alphaBlendOP);
		m_glSrcBlend = GLES2Mapping::MapBlendFactor(m_desc.srcBlend);
		m_glDstBlend = GLES2Mapping::MapBlendFactor(m_desc.dstBlend);
		m_glSrcAlphaBlend = GLES2Mapping::MapBlendFactor(m_desc.srcAlphaBlend);
		m_glDstAlphaBlend = GLES2Mapping::MapBlendFactor(m_desc.dstAlphaBlend);
		m_glRedMask = (m_desc.colorWriteMask & CMASK_RED) != 0;
		m_glGreenMask = (m_desc.colorWriteMask & CMASK_GREEN) != 0;
		m_glBlueMask = (m_desc.colorWriteMask & CMASK_BLUE) != 0;
		m_glAlphaMask = (m_desc.colorWriteMask & CMASK_ALPHA) != 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	GLES2DepthStencilState::GLES2DepthStencilState()
	{
		create();
	}

	GLES2DepthStencilState::GLES2DepthStencilState(const DepthStencilDesc &desc)
		: DepthStencilState(desc)
	{
		create();
	}

	GLES2DepthStencilState::~GLES2DepthStencilState()
	{
	}

	void GLES2DepthStencilState::active()
	{
		DepthStencilStateParams depthStencilParams;
		memset(&depthStencilParams, 0, sizeof(depthStencilParams));
		depthStencilParams.isEnableDepthTest = m_desc.bDepthEnable;
		depthStencilParams.isEnableStencilTest = m_desc.bFrontStencilEnable || m_desc.bBackStencilEnable;
		depthStencilParams.isWriteDepth = m_desc.bWriteDepth;
		depthStencilParams.frontStencilRef = m_desc.frontStencilRef;
		depthStencilParams.frontStencilReadMask = m_desc.frontStencilReadMask;
		depthStencilParams.frontStencilWriteMask = m_desc.frontStencilWriteMask;
		depthStencilParams.backStencilRef = m_desc.backStencilRef;
		depthStencilParams.backStencilReadMask = m_desc.backStencilReadMask;
		depthStencilParams.backStencilWriteMask = m_desc.backStencilWriteMask;
		depthStencilParams.depthFunc = m_glDepthFunc;
		depthStencilParams.frontStencilFunc = m_glFrontStencilFunc;
		depthStencilParams.frontStencilFailOP = m_glFrontStencilFailOP;
		depthStencilParams.frontStencilDepthFailOP = m_glFrontStencilDepthFailOP;
		depthStencilParams.frontStencilPassOP = m_glFrontStencilPassOP;
		depthStencilParams.backStencilFunc = m_glBackStencilFunc;
		depthStencilParams.backStencilFailOP = m_glBackStencilFailOP;
		depthStencilParams.backStencilDepthFailOP = m_glBackStencilDepthFailOP;
		depthStencilParams.backStencilPassOP = m_glBackStencilPassOP;
		
		DepthStencilState* pCurState = Renderer::instance()->getDepthStencilState();
		if(pCurState)
		{
			const  DepthStencilDesc& currDesc = pCurState->getDesc();

			if (m_desc.bDepthEnable != currDesc.bDepthEnable)
			{
				depthStencilParams.isChangeDepthTest = true;
			}

			if (m_desc.bWriteDepth != currDesc.bWriteDepth)
			{
				depthStencilParams.isChangeDepthWrite = true;
			}

			if (m_desc.depthFunc != currDesc.depthFunc)
			{
				depthStencilParams.isSetDepthFunc = true;
			}

			if ((m_desc.frontStencilFunc != currDesc.frontStencilFunc) ||
				(m_desc.frontStencilRef != currDesc.frontStencilRef) ||
				(m_desc.frontStencilReadMask != currDesc.frontStencilReadMask))
			{
				depthStencilParams.isSetStencilFuncFront = true;
			}

			if ((m_desc.frontStencilFailOP != currDesc.frontStencilFailOP) ||
				(m_desc.frontStencilDepthFailOP != currDesc.frontStencilDepthFailOP) ||
				(m_desc.frontStencilPassOP != currDesc.frontStencilPassOP))
			{
				depthStencilParams.isSetStencilOpFront = true;
			}

			if (m_desc.frontStencilWriteMask != currDesc.frontStencilWriteMask)
			{
				depthStencilParams.isSetStencilMaskFront = true;
			}

			if ((m_desc.backStencilFunc != currDesc.backStencilFunc) ||
				(m_desc.backStencilRef != currDesc.backStencilRef) ||
				(m_desc.backStencilReadMask != currDesc.backStencilReadMask))
			{
				depthStencilParams.isSetStencilFuncBack = true;
			}

			if ((m_desc.backStencilFailOP != currDesc.backStencilFailOP) ||
				(m_desc.backStencilDepthFailOP != currDesc.backStencilDepthFailOP) ||
				(m_desc.backStencilPassOP != currDesc.backStencilPassOP))
			{
				depthStencilParams.isSetStencilOpBack = true;
			}

			if (m_desc.backStencilWriteMask != currDesc.backStencilWriteMask)
			{
				depthStencilParams.isSetStencilMaskBack = true;
			}

			if ((m_desc.bFrontStencilEnable != currDesc.bFrontStencilEnable) ||
				(m_desc.bBackStencilEnable != currDesc.bBackStencilEnable))
			{
				depthStencilParams.isChangeStencilTest = true;
			}
		}
		else
		{
			depthStencilParams.isChangeDepthTest = true;
			depthStencilParams.isChangeDepthWrite = true;
			depthStencilParams.isSetDepthFunc = true;
			depthStencilParams.isSetStencilFuncFront = true;
			depthStencilParams.isSetStencilOpFront = true;
			depthStencilParams.isSetStencilMaskFront = true;
			depthStencilParams.isSetStencilFuncBack = true;
			depthStencilParams.isSetStencilOpBack = true;
			depthStencilParams.isSetStencilMaskBack = true;
			depthStencilParams.isChangeStencilTest = true;
		}

		TRenderTask<GLES2StateTaskActiveDepthStencil>::CreateTask(depthStencilParams);
	}

	void GLES2DepthStencilState::create()
	{
		m_glDepthMask = m_desc.bWriteDepth? GL_TRUE : GL_FALSE;
		m_glDepthFunc = GLES2Mapping::MapComparisonFunc(m_desc.depthFunc);
		m_glFrontStencilFunc = GLES2Mapping::MapComparisonFunc(m_desc.frontStencilFunc);
		m_glFrontStencilFailOP = GLES2Mapping::MapStencilOperation(m_desc.frontStencilFailOP);
		m_glFrontStencilDepthFailOP = GLES2Mapping::MapStencilOperation(m_desc.frontStencilDepthFailOP);
		m_glFrontStencilPassOP = GLES2Mapping::MapStencilOperation(m_desc.frontStencilPassOP);
		m_glBackStencilFunc = GLES2Mapping::MapComparisonFunc(m_desc.backStencilFunc);
		m_glBackStencilFailOP = GLES2Mapping::MapStencilOperation(m_desc.backStencilFailOP);
		m_glBackStencilDepthFailOP = GLES2Mapping::MapStencilOperation(m_desc.backStencilDepthFailOP);
		m_glBackStencilPassOP = GLES2Mapping::MapStencilOperation(m_desc.backStencilPassOP);
	}

	// 构造函数
	GLES2RasterizerState::GLES2RasterizerState()
	{
		create();
	}

	// 构造函数
	GLES2RasterizerState::GLES2RasterizerState(const RasterizerDesc& desc)
		: RasterizerState(desc)
	{
		create();
	}

	// 析构函数
	GLES2RasterizerState::~GLES2RasterizerState()
	{
	}

	// 设置光栅化状态
	void GLES2RasterizerState::active()
	{
		RasterizerState* pCurState = Renderer::instance()->getRasterizerState();
		const RasterizerDesc* currDesc = nullptr;
		if (pCurState)
		{
			currDesc = &pCurState->getDesc();
		}

		TRenderTask<GLES2StateTaskActiveRasterizer>::CreateTask(m_desc, currDesc, m_glFrontFace);
	}

	// 创建光栅化状态
	void GLES2RasterizerState::create()
	{
		if(m_desc.polygonMode != PM_FILL)
			LordLogError("GLES2Renderer only support polygon fill mode [PM_FILL]. Check polygonMode property.");
		
		if(m_desc.shadeModel != SM_GOURAND)
			LordLogError("GLES2Renderer only support smooth shading [SM_SMOOTH]. Check shadeModel property.");
		
		m_glFrontFace = m_desc.bFrontFaceCCW ? GL_CCW : GL_CW;
	}

	// 构造函数
	GLES2SamplerState::GLES2SamplerState()
	{
		create();
	}

	// 构造函数
	GLES2SamplerState::GLES2SamplerState(const SamplerDesc& desc)
		: SamplerState(desc)
	{
		create();
	}

	// 析构函数
	GLES2SamplerState::~GLES2SamplerState()
	{
		// attention!
		// 没有放到渲染线程释放是因为Sampler对象只在Render对象destroy时释放。
		LordSafeDelete(m_sampler_proxy);
	}

	// 激活
	void GLES2SamplerState::active(const SamplerState* pre) const
	{
		const SamplerDesc* prev_desc = nullptr;
		const SamplerDesc* curr_desc = nullptr;
		if (pre)
		{
			prev_desc = &pre->getDesc();
			curr_desc = &m_desc;
		}

		TRenderTask<GLES2RenderStateTaskActiveSampler>::CreateTask(m_sampler_proxy, curr_desc, prev_desc);
	}

	// 新建
	void GLES2SamplerState::create()
	{
		m_sampler_proxy = LordNew(GLES2SamplerStateGPUProxy);

		if(m_desc.minFilter == FO_NONE)
		{
			m_sampler_proxy->m_glMinFilter = 0;
		}
		else if(m_desc.minFilter == FO_LINEAR || m_desc.minFilter == FO_ANISOTROPIC)
		{
			if(m_desc.mipFilter == FO_NONE)
				m_sampler_proxy->m_glMinFilter = GL_LINEAR;
			else if(m_desc.mipFilter == FO_LINEAR || m_desc.mipFilter == FO_ANISOTROPIC)
				m_sampler_proxy->m_glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			else
				m_sampler_proxy->m_glMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		}
		else
		{
			if(m_desc.mipFilter == FO_NONE)
				m_sampler_proxy->m_glMinFilter = GL_NEAREST;
			else if(m_desc.mipFilter == FO_LINEAR || m_desc.mipFilter == FO_ANISOTROPIC)
				m_sampler_proxy->m_glMinFilter = GL_NEAREST_MIPMAP_LINEAR;
			else
				m_sampler_proxy->m_glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
		}

		if(m_desc.magFilter == FO_NONE)
			m_sampler_proxy->m_glMagFilter = 0;
		else if(m_desc.magFilter == FO_LINEAR || m_desc.magFilter == FO_ANISOTROPIC)
			m_sampler_proxy->m_glMagFilter = GL_LINEAR;
		else
			m_sampler_proxy->m_glMagFilter = GL_NEAREST;

		m_sampler_proxy->m_glAddrModeU = GLES2Mapping::MapAddressMode(m_desc.addrUMode);
		m_sampler_proxy->m_glAddrModeV = GLES2Mapping::MapAddressMode(m_desc.addrVMode);
		m_sampler_proxy->m_glAddrModeW = GLES2Mapping::MapAddressMode(m_desc.addrWMode);
	}

	GLint GLES2SamplerState::getGLMinFilter() const
	{
		return m_sampler_proxy->m_glMinFilter;
	}

	GLint GLES2SamplerState::getGLMagFilter() const
	{
		return m_sampler_proxy->m_glMagFilter;
	}

	GLint GLES2SamplerState::getGLAddrModeU() const
	{
		return m_sampler_proxy->m_glAddrModeU;
	}

	GLint GLES2SamplerState::getGLAddrModeV() const
	{
		return m_sampler_proxy->m_glAddrModeV;
	}

	GLint GLES2SamplerState::getGLAddrModeW() const
	{
		return m_sampler_proxy->m_glAddrModeW;
	}
}
