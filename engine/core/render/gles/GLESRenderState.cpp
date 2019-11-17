#include "GLESRenderBase.h"
#include "GLESRenderState.h"
#include "GLESMapping.h"
#include "GLESTexture2D.h"
#include "interface/Renderer.h"
#include "engine/core/util/Exception.h"

namespace Echo
{
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

		if (!Renderer::instance()->getDeviceFeatures().supportHFColorBf())
		{
			blendParams.alpha_mask = true;
		}

		if (blendParams.isChangeA2C)
		{
			if (blendParams.isA2CEnable)
			{
				OGLESDebug(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			}
			else
			{
				OGLESDebug(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			}
		}

		if (blendParams.isChangeBlendEnable)
		{
			if (blendParams.isBlendEnable)
			{
				OGLESDebug(glEnable(GL_BLEND));
			}
			else
			{
				OGLESDebug(glDisable(GL_BLEND));
			}
		}

		if (blendParams.isChangeBlendOp)
		{
			OGLESDebug(glBlendEquationSeparate(blendParams.blend_op, blendParams.alpha_blend_op));
		}

		if (blendParams.isChangeBlendFunc)
		{
			OGLESDebug(glBlendFuncSeparate(blendParams.src_blend, blendParams.dst_blend, blendParams.src_alpha_blend, blendParams.dst_alpha_blend));
		}

		if (blendParams.isChangeColorWriteMask)
		{
			OGLESDebug(glColorMask(blendParams.red_mask, blendParams.green_mask, blendParams.blue_mask, blendParams.alpha_mask));
		}

		if (blendParams.isChangeBlendFactor)
		{
			OGLESDebug(glBlendColor((GLclampf)blendParams.blendFactor.r, (GLclampf)blendParams.blendFactor.g, (GLclampf)blendParams.blendFactor.b, (GLclampf)blendParams.blendFactor.a));
		}
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

		if (depthStencilParams.isChangeDepthTest)
		{
			if (depthStencilParams.isEnableDepthTest)
			{
				OGLESDebug(glEnable(GL_DEPTH_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_DEPTH_TEST));
			}
		}

		if (depthStencilParams.isChangeDepthWrite)
		{
			OGLESDebug(glDepthMask(depthStencilParams.isWriteDepth));
		}

		if (depthStencilParams.isSetDepthFunc)
		{
			OGLESDebug(glDepthFunc(depthStencilParams.depthFunc));
		}

		if (depthStencilParams.isSetStencilFuncFront)
		{
			OGLESDebug(glStencilFuncSeparate(GL_FRONT, depthStencilParams.frontStencilFunc, depthStencilParams.frontStencilRef, depthStencilParams.frontStencilReadMask));
		}

		if (depthStencilParams.isSetStencilOpFront)
		{
			OGLESDebug(glStencilOpSeparate(GL_FRONT, depthStencilParams.frontStencilFailOP, depthStencilParams.frontStencilDepthFailOP, depthStencilParams.frontStencilPassOP));
		}

		if (depthStencilParams.isSetStencilMaskFront)
		{
			OGLESDebug(glStencilMaskSeparate(GL_FRONT, depthStencilParams.frontStencilWriteMask));
		}

		if (depthStencilParams.isSetStencilFuncBack)
		{
			OGLESDebug(glStencilFuncSeparate(GL_BACK, depthStencilParams.backStencilFunc, depthStencilParams.backStencilRef, depthStencilParams.backStencilReadMask));
		}

		if (depthStencilParams.isSetStencilOpBack)
		{
			OGLESDebug(glStencilOpSeparate(GL_BACK, depthStencilParams.backStencilFailOP, depthStencilParams.backStencilDepthFailOP, depthStencilParams.backStencilPassOP));
		}

		if (depthStencilParams.isSetStencilMaskBack)
		{
			OGLESDebug(glStencilMaskSeparate(GL_BACK, depthStencilParams.backStencilWriteMask));
		}

		if (depthStencilParams.isChangeStencilTest)
		{
			if (depthStencilParams.isEnableStencilTest)
			{
				OGLESDebug(glEnable(GL_STENCIL_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_STENCIL_TEST));
			}
		}
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

	GLES2RasterizerState::GLES2RasterizerState(const RasterizerDesc& desc)
		: RasterizerState(desc)
	{
		create();
	}

	GLES2RasterizerState::~GLES2RasterizerState()
	{
	}

	void GLES2RasterizerState::active()
	{
		RasterizerState* pCurState = Renderer::instance()->getRasterizerState();
		const RasterizerDesc* currDesc = nullptr;
		if (pCurState)
		{
			currDesc = &pCurState->getDesc();
		}

		if (currDesc)
		{
			if (m_desc.cullMode != currDesc->cullMode)
			{
				switch (m_desc.cullMode)
				{
					case RasterizerState::CULL_NONE:
					{
						OGLESDebug(glDisable(GL_CULL_FACE));
					} break;
					case RasterizerState::CULL_FRONT:
					{
						OGLESDebug(glEnable(GL_CULL_FACE));
						OGLESDebug(glCullFace(GL_FRONT));
					} break;
					case RasterizerState::CULL_BACK:
					{
						OGLESDebug(glEnable(GL_CULL_FACE));
						OGLESDebug(glCullFace(GL_BACK));
					} break;
				}
			}

			if (m_desc.bFrontFaceCCW != currDesc->bFrontFaceCCW)
			{
				OGLESDebug(glFrontFace(m_glFrontFace));
			}


			if ((m_desc.depthBiasFactor != currDesc->depthBiasFactor) ||
				(m_desc.depthBias != currDesc->depthBias))
			{
				if (m_desc.depthBias != 0.0f || m_desc.depthBiasFactor != 0.0f)
				{
					OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
				}
				else
				{
					OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
				}

				OGLESDebug(glPolygonOffset(m_desc.depthBiasFactor, m_desc.depthBias));
			}

			if (m_desc.bScissor != currDesc->bScissor)
			{
				if (m_desc.bScissor)
				{
					OGLESDebug(glEnable(GL_SCISSOR_TEST));
				}
				else
				{
					OGLESDebug(glDisable(GL_SCISSOR_TEST));
				}
			}
		}
		else
		{
			// set cull mode
			switch (m_desc.cullMode)
			{
				case RasterizerState::CULL_NONE:
				{
					OGLESDebug(glDisable(GL_CULL_FACE));
				} break;
				case RasterizerState::CULL_FRONT:
				{
					OGLESDebug(glEnable(GL_CULL_FACE));
					OGLESDebug(glCullFace(GL_FRONT));
				} break;
				case RasterizerState::CULL_BACK:
				{
					OGLESDebug(glEnable(GL_CULL_FACE));
					OGLESDebug(glCullFace(GL_BACK));
				} break;
			}

			// set front face
			OGLESDebug(glFrontFace(m_glFrontFace));

			// Bias is in {0, 16}, scale the unit addition appropriately
			if (m_desc.depthBias != 0.0f && m_desc.depthBiasFactor != 0.0f)
			{
				OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
			}
			else
			{
				OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
			}

			OGLESDebug(glPolygonOffset(m_desc.depthBiasFactor, m_desc.depthBias));

			// depth clip
			/*
			if (myDesc.bDepthClip)
			{
			glDisable(GL_DEPTH_CLAMP);
			}
			else
			{
			glEnable(GL_DEPTH_CLAMP);
			}
			*/

			// scissor test
			if (m_desc.bScissor)
			{
				OGLESDebug(glEnable(GL_SCISSOR_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_SCISSOR_TEST));
			}
		}
	}

	void GLES2RasterizerState::create()
	{
		if(m_desc.polygonMode != PM_FILL)
			EchoLogError("GLES2Renderer only support polygon fill mode [PM_FILL]. Check polygonMode property.");
		
		if(m_desc.shadeModel != SM_GOURAND)
			EchoLogError("GLES2Renderer only support smooth shading [SM_SMOOTH]. Check shadeModel property.");
		
		m_glFrontFace = m_desc.bFrontFaceCCW ? GL_CCW : GL_CW;
	}

	GLES2SamplerState::GLES2SamplerState(const SamplerDesc& desc)
		: SamplerState(desc)
	{
		create();
	}

	GLES2SamplerState::~GLES2SamplerState()
	{
	}

	void GLES2SamplerState::active(const SamplerState* pre) const
	{
		const SamplerDesc* prev_desc = nullptr;
		const SamplerDesc* curr_desc = nullptr;
		if (pre)
		{
			prev_desc = &pre->getDesc();
			curr_desc = &m_desc;
		}

		if (prev_desc && curr_desc)
		{
			// only 2D texture type.
			if (prev_desc->minFilter != curr_desc->minFilter || prev_desc->mipFilter != curr_desc->mipFilter)
			{
				OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_glMinFilter));
			}

			if (prev_desc->magFilter != curr_desc->magFilter || prev_desc->mipFilter != curr_desc->mipFilter)
			{
				OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_glMagFilter));
			}

			if (prev_desc->addrUMode != curr_desc->addrUMode)
			{
				OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_glAddrModeU));
			}

			if (prev_desc->addrVMode != curr_desc->addrVMode)
			{
				OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_glAddrModeV));
			}
		}
		else
		{
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_glMinFilter));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_glMagFilter));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_glAddrModeU));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_glAddrModeV));
		}
	}

	void GLES2SamplerState::create()
	{
		if (m_desc.minFilter == FO_NONE)
		{
			m_glMinFilter = 0;
		}
		else if (m_desc.minFilter == FO_LINEAR || m_desc.minFilter == FO_ANISOTROPIC)
		{
			if (m_desc.mipFilter == FO_NONE)
				m_glMinFilter = GL_LINEAR;
			else if (m_desc.mipFilter == FO_LINEAR || m_desc.mipFilter == FO_ANISOTROPIC)
				m_glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			else
				m_glMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		}
		else
		{
			if (m_desc.mipFilter == FO_NONE)
				m_glMinFilter = GL_NEAREST;
			else if (m_desc.mipFilter == FO_LINEAR || m_desc.mipFilter == FO_ANISOTROPIC)
				m_glMinFilter = GL_NEAREST_MIPMAP_LINEAR;
			else
				m_glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
		}

		if (m_desc.magFilter == FO_NONE)
			m_glMagFilter = 0;
		else if (m_desc.magFilter == FO_LINEAR || m_desc.magFilter == FO_ANISOTROPIC)
			m_glMagFilter = GL_LINEAR;
		else
			m_glMagFilter = GL_NEAREST;

		m_glAddrModeU = GLES2Mapping::MapAddressMode(m_desc.addrUMode);
		m_glAddrModeV = GLES2Mapping::MapAddressMode(m_desc.addrVMode);
		m_glAddrModeW = GLES2Mapping::MapAddressMode(m_desc.addrWMode);
	}

	GLint GLES2SamplerState::getGLMinFilter() const
	{
		return m_glMinFilter;
	}

	GLint GLES2SamplerState::getGLMagFilter() const
	{
		return m_glMagFilter;
	}

	GLint GLES2SamplerState::getGLAddrModeU() const
	{
		return m_glAddrModeU;
	}

	GLint GLES2SamplerState::getGLAddrModeV() const
	{
		return m_glAddrModeV;
	}

	GLint GLES2SamplerState::getGLAddrModeW() const
	{
		return m_glAddrModeW;
	}
}
