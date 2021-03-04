#include "gles_render_base.h"
#include "gles_render_state.h"
#include "gles_mapping.h"
#include "gles_texture_2d.h"
#include "gles_renderer.h"
#include "base/Renderer.h"
#include "engine/core/util/Exception.h"

namespace Echo
{
	GLESBlendState::GLESBlendState(const BlendDesc& desc)
		: BlendState(desc)
	{
		create();
	}

	GLESBlendState::~GLESBlendState()
	{
	}

	void GLESBlendState::active()
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

		BlendState* pCurState = (ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance()))->getBlendState();
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

	void GLESBlendState::create()
	{
		m_glBlendOP = GLESMapping::MapBlendOperation(m_desc.blendOP);
		m_glAlphaBlendOP = GLESMapping::MapBlendOperation(m_desc.alphaBlendOP);
		m_glSrcBlend = GLESMapping::MapBlendFactor(m_desc.srcBlend);
		m_glDstBlend = GLESMapping::MapBlendFactor(m_desc.dstBlend);
		m_glSrcAlphaBlend = GLESMapping::MapBlendFactor(m_desc.srcAlphaBlend);
		m_glDstAlphaBlend = GLESMapping::MapBlendFactor(m_desc.dstAlphaBlend);
		m_glRedMask = (m_desc.colorWriteMask & CMASK_RED) != 0;
		m_glGreenMask = (m_desc.colorWriteMask & CMASK_GREEN) != 0;
		m_glBlueMask = (m_desc.colorWriteMask & CMASK_BLUE) != 0;
		m_glAlphaMask = (m_desc.colorWriteMask & CMASK_ALPHA) != 0;
	}

	GLESDepthStencilState::GLESDepthStencilState()
		: DepthStencilState()
	{
	}

	GLESDepthStencilState::~GLESDepthStencilState()
	{
	}

	void GLESDepthStencilState::active()
	{
		if (m_dirty)
		{
			m_glDepthMask = bWriteDepth ? GL_TRUE : GL_FALSE;
			m_glDepthFunc = GLESMapping::MapComparisonFunc(depthFunc);
			m_glFrontStencilFunc = GLESMapping::MapComparisonFunc(frontStencilFunc);
			m_glFrontStencilFailOP = GLESMapping::MapStencilOperation(frontStencilFailOP);
			m_glFrontStencilDepthFailOP = GLESMapping::MapStencilOperation(frontStencilDepthFailOP);
			m_glFrontStencilPassOP = GLESMapping::MapStencilOperation(frontStencilPassOP);
			m_glBackStencilFunc = GLESMapping::MapComparisonFunc(backStencilFunc);
			m_glBackStencilFailOP = GLESMapping::MapStencilOperation(backStencilFailOP);
			m_glBackStencilDepthFailOP = GLESMapping::MapStencilOperation(backStencilDepthFailOP);
			m_glBackStencilPassOP = GLESMapping::MapStencilOperation(backStencilPassOP);

			m_dirty = false;
		}

		DepthStencilStateParams depthStencilParams;
		memset(&depthStencilParams, 0, sizeof(depthStencilParams));
		depthStencilParams.isEnableDepthTest = bDepthEnable;
		depthStencilParams.isEnableStencilTest = bFrontStencilEnable || bBackStencilEnable;
		depthStencilParams.isWriteDepth = bWriteDepth;
		depthStencilParams.frontStencilRef = frontStencilRef;
		depthStencilParams.frontStencilReadMask = frontStencilReadMask;
		depthStencilParams.frontStencilWriteMask = frontStencilWriteMask;
		depthStencilParams.backStencilRef = backStencilRef;
		depthStencilParams.backStencilReadMask = backStencilReadMask;
		depthStencilParams.backStencilWriteMask = backStencilWriteMask;
		depthStencilParams.depthFunc = m_glDepthFunc;
		depthStencilParams.frontStencilFunc = m_glFrontStencilFunc;
		depthStencilParams.frontStencilFailOP = m_glFrontStencilFailOP;
		depthStencilParams.frontStencilDepthFailOP = m_glFrontStencilDepthFailOP;
		depthStencilParams.frontStencilPassOP = m_glFrontStencilPassOP;
		depthStencilParams.backStencilFunc = m_glBackStencilFunc;
		depthStencilParams.backStencilFailOP = m_glBackStencilFailOP;
		depthStencilParams.backStencilDepthFailOP = m_glBackStencilDepthFailOP;
		depthStencilParams.backStencilPassOP = m_glBackStencilPassOP;
		
		DepthStencilState* pCurState = (ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance()))->getDepthStencilState();
		if(pCurState)
		{
			if (bDepthEnable != pCurState->bDepthEnable)
			{
				depthStencilParams.isChangeDepthTest = true;
			}

			if (bWriteDepth != pCurState->bWriteDepth)
			{
				depthStencilParams.isChangeDepthWrite = true;
			}

			if (depthFunc != pCurState->depthFunc)
			{
				depthStencilParams.isSetDepthFunc = true;
			}

			if ((frontStencilFunc != pCurState->frontStencilFunc) ||
				(frontStencilRef != pCurState->frontStencilRef) ||
				(frontStencilReadMask != pCurState->frontStencilReadMask))
			{
				depthStencilParams.isSetStencilFuncFront = true;
			}

			if ((frontStencilFailOP != pCurState->frontStencilFailOP) ||
				(frontStencilDepthFailOP != pCurState->frontStencilDepthFailOP) ||
				(frontStencilPassOP != pCurState->frontStencilPassOP))
			{
				depthStencilParams.isSetStencilOpFront = true;
			}

			if (frontStencilWriteMask != pCurState->frontStencilWriteMask)
			{
				depthStencilParams.isSetStencilMaskFront = true;
			}

			if ((backStencilFunc != pCurState->backStencilFunc) ||
				(backStencilRef != pCurState->backStencilRef) ||
				(backStencilReadMask != pCurState->backStencilReadMask))
			{
				depthStencilParams.isSetStencilFuncBack = true;
			}

			if ((backStencilFailOP != pCurState->backStencilFailOP) ||
				(backStencilDepthFailOP != pCurState->backStencilDepthFailOP) ||
				(backStencilPassOP != pCurState->backStencilPassOP))
			{
				depthStencilParams.isSetStencilOpBack = true;
			}

			if (backStencilWriteMask != pCurState->backStencilWriteMask)
			{
				depthStencilParams.isSetStencilMaskBack = true;
			}

			if ((bFrontStencilEnable != pCurState->bFrontStencilEnable) ||
				(bBackStencilEnable != pCurState->bBackStencilEnable))
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

	GLESRasterizerState::GLESRasterizerState()
		: RasterizerState()
	{
		if (polygonMode != PM_FILL)
			EchoLogError("GLES2Renderer only support polygon fill mode [PM_FILL]. Check polygonMode property.");

		if (shadeModel != SM_GOURAND)
			EchoLogError("GLES2Renderer only support smooth shading [SM_SMOOTH]. Check shadeModel property.");

		m_glFrontFace = m_frontFaceCCW ? GL_CCW : GL_CW;
	}

	GLESRasterizerState::~GLESRasterizerState()
	{
	}

	void GLESRasterizerState::active()
	{
		RasterizerState* curState = (ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance()))->getRasterizerState();
		if (curState)
		{
			if (cullMode != curState->getCullMode())
			{
				switch (cullMode)
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

			if (m_frontFaceCCW != curState->isFrontFaceCCW())
			{
				OGLESDebug(glFrontFace(m_glFrontFace));
			}

			if ((m_depthBiasFactor != curState->getDepthBiasFactor()) ||
				(m_depthBias != curState->getDepthBias()))
			{
				if (m_depthBias != 0.0f || m_depthBiasFactor != 0.0f)
				{
					OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
				}
				else
				{
					OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
				}

				OGLESDebug(glPolygonOffset(m_depthBiasFactor, m_depthBias));
			}

			if (m_scissor != curState->isScissor())
			{
				if (m_scissor)
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
			switch (cullMode)
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
			if (m_depthBias != 0.0f && m_depthBiasFactor != 0.0f)
			{
				OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
			}
			else
			{
				OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
			}

			OGLESDebug(glPolygonOffset(m_depthBiasFactor, m_depthBias));

			// scissor test
			if (m_scissor)
			{
				OGLESDebug(glEnable(GL_SCISSOR_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_SCISSOR_TEST));
			}
		}
	}

	GLESSamplerState::GLESSamplerState(const SamplerDesc& desc)
		: SamplerState(desc)
	{
		create();
	}

	GLESSamplerState::~GLESSamplerState()
	{
	}

	void GLESSamplerState::active(const SamplerState* pre) const
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

	void GLESSamplerState::create()
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

		m_glAddrModeU = GLESMapping::MapAddressMode(m_desc.addrUMode);
		m_glAddrModeV = GLESMapping::MapAddressMode(m_desc.addrVMode);
		m_glAddrModeW = GLESMapping::MapAddressMode(m_desc.addrWMode);
	}

	GLint GLESSamplerState::getGLMinFilter() const
	{
		return m_glMinFilter;
	}

	GLint GLESSamplerState::getGLMagFilter() const
	{
		return m_glMagFilter;
	}

	GLint GLESSamplerState::getGLAddrModeU() const
	{
		return m_glAddrModeU;
	}

	GLint GLESSamplerState::getGLAddrModeV() const
	{
		return m_glAddrModeV;
	}

	GLint GLESSamplerState::getGLAddrModeW() const
	{
		return m_glAddrModeW;
	}
}
