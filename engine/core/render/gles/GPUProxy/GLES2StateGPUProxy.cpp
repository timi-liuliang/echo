#include "GLES2RenderStd.h"
#include "GLES2RenderState.h"
#include "GLES2StateGPUProxy.h"

namespace Echo
{

	void ActiveBlendState(const BlendStateParams& params)
	{
		if (params.isChangeA2C)
		{
			if (params.isA2CEnable)
			{
				OGLESDebug(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			}
			else
			{
				OGLESDebug(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			}
		}

		if (params.isChangeBlendEnable)
		{
			if (params.isBlendEnable)
			{
				OGLESDebug(glEnable(GL_BLEND));
			}
			else
			{
				OGLESDebug(glDisable(GL_BLEND));
			}
		}

		if (params.isChangeBlendOp)
		{
			OGLESDebug(glBlendEquationSeparate(params.blend_op, params.alpha_blend_op));
		}

		if (params.isChangeBlendFunc)
		{
			OGLESDebug(glBlendFuncSeparate(params.src_blend, params.dst_blend, params.src_alpha_blend, params.dst_alpha_blend));
		}

		if (params.isChangeColorWriteMask)
		{
			OGLESDebug(glColorMask(params.red_mask, params.green_mask, params.blue_mask, params.alpha_mask));
		}

		if (params.isChangeBlendFactor)
		{
			OGLESDebug(glBlendColor((GLclampf)params.blendFactor.r, (GLclampf)params.blendFactor.g, (GLclampf)params.blendFactor.b, (GLclampf)params.blendFactor.a));
		}
	}

	void ActiveDepthStencilState(const DepthStencilStateParams& params)
	{
		if (params.isChangeDepthTest)
		{
			if (params.isEnableDepthTest)
			{
				OGLESDebug(glEnable(GL_DEPTH_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_DEPTH_TEST));
			}
		}

		if (params.isChangeDepthWrite)
		{
			OGLESDebug(glDepthMask(params.isWriteDepth));
		}

		if (params.isSetDepthFunc)
		{
			OGLESDebug(glDepthFunc(params.depthFunc));
		}

		if (params.isSetStencilFuncFront)
		{
			OGLESDebug(glStencilFuncSeparate(GL_FRONT, params.frontStencilFunc, params.frontStencilRef, params.frontStencilReadMask));
		}

		if (params.isSetStencilOpFront)
		{
			OGLESDebug(glStencilOpSeparate(GL_FRONT, params.frontStencilFailOP, params.frontStencilDepthFailOP, params.frontStencilPassOP));
		}

		if (params.isSetStencilMaskFront)
		{
			OGLESDebug(glStencilMaskSeparate(GL_FRONT, params.frontStencilWriteMask));
		}

		if (params.isSetStencilFuncBack)
		{
			OGLESDebug(glStencilFuncSeparate(GL_BACK, params.backStencilFunc, params.backStencilRef, params.backStencilReadMask));
		}

		if (params.isSetStencilOpBack)
		{
			OGLESDebug(glStencilOpSeparate(GL_BACK, params.backStencilFailOP, params.backStencilDepthFailOP, params.backStencilPassOP));
		}

		if (params.isSetStencilMaskBack)
		{
			OGLESDebug(glStencilMaskSeparate(GL_BACK, params.backStencilWriteMask));
		}

		if (params.isChangeStencilTest)
		{
			if (params.isEnableStencilTest)
			{
				OGLESDebug(glEnable(GL_STENCIL_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_STENCIL_TEST));
			}
		}
	}

	void ActiveRasterizerState(const RasterizerState::RasterizerDesc& myDesc, const RasterizerState::RasterizerDesc& currDesc, GLenum frontFace, bool needCompare)
	{
		if (needCompare)
		{
			if (myDesc.cullMode != currDesc.cullMode)
			{
				switch (myDesc.cullMode)
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

			if (myDesc.bFrontFaceCCW != currDesc.bFrontFaceCCW)
			{
				OGLESDebug(glFrontFace(frontFace));
			}


			if ((myDesc.depthBiasFactor != currDesc.depthBiasFactor) ||
				(myDesc.depthBias != currDesc.depthBias))
			{
				if (myDesc.depthBias != 0.0f || myDesc.depthBiasFactor != 0.0f)
				{
					OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
				}
				else
				{
					OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
				}

				OGLESDebug(glPolygonOffset(myDesc.depthBiasFactor, myDesc.depthBias));
			}

			if (myDesc.bScissor != currDesc.bScissor)
			{
				if (myDesc.bScissor)
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
			// set polygon mode
			// mark: only support polygon fill mode
			//glPolygonMode(GL_FRONT_AND_BACK, m_polygonMode);

			// set shade model
			// mark: flat shading is not supported, only support smooth shading
			//glShadeModel(m_shadeModel);

			// set cull mode
			switch (myDesc.cullMode)
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
			OGLESDebug(glFrontFace(frontFace));

			// Bias is in {0, 16}, scale the unit addition appropriately
			if (myDesc.depthBias != 0.0f && myDesc.depthBiasFactor != 0.0f)
			{
				OGLESDebug(glEnable(GL_POLYGON_OFFSET_FILL));
			}
			else
			{
				OGLESDebug(glDisable(GL_POLYGON_OFFSET_FILL));
			}

			OGLESDebug(glPolygonOffset(myDesc.depthBiasFactor, myDesc.depthBias));

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
			if (myDesc.bScissor)
			{
				OGLESDebug(glEnable(GL_SCISSOR_TEST));
			}
			else
			{
				OGLESDebug(glDisable(GL_SCISSOR_TEST));
			}

			// multi-sample
			// mark: ios uses glResolveMultisampleFramebufferAPPLE function to multi sample.
			/*
			if (myDesc.bMultisample)
			{
				glEnable(GL_MULTISAMPLE);
				//glResolveMultisampleFramebufferAPPLE
			}
			else
			{
				glDisable(GL_MULTISAMPLE);
			}
			*/
		}
	}

	void GLES2SamplerStateGPUProxy::active(const SamplerState::SamplerDesc* curr_desc, const SamplerState::SamplerDesc* prev_desc)
	{
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

			EchoSafeDelete(prev_desc, SamplerDesc);
			EchoSafeDelete(curr_desc, SamplerDesc);
		}
		else
		{
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_glMinFilter));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_glMagFilter));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_glAddrModeU));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_glAddrModeV));
		}
	}

}