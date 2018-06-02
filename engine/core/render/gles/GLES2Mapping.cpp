#include "GLES2RenderStd.h"
#include "GLES2Mapping.h"
#include "Render/GPUBuffer.h"
#include "Render/PixelFormat.h"

namespace Echo
{
	GLenum GLES2Mapping::g_halfFloatDataType = GL_INVALID_ENUM;
	GLenum GLES2Mapping::g_halfFloatInternalFormat = GL_INVALID_ENUM;

	GLenum GLES2Mapping::MapStencilOperation(DepthStencilState::StencilOperation op)
	{
		switch(op)
		{
			case DepthStencilState::SOP_KEEP:		return GL_KEEP;
			case DepthStencilState::SOP_ZERO:		return GL_ZERO;
			case DepthStencilState::SOP_REPLACE:	return GL_REPLACE;
			case DepthStencilState::SOP_INCR:		return GL_INCR;
			case DepthStencilState::SOP_DECR:		return GL_DECR;
			case DepthStencilState::SOP_INVERT:		return GL_INVERT;
			case DepthStencilState::SOP_INCR_WRAP:	return GL_INCR_WRAP;
			case DepthStencilState::SOP_DECR_WRAP:	return GL_DECR_WRAP;
			default:
			{
				EchoAssert(false);
				return GL_KEEP;
			}
		}
	}

	GLenum GLES2Mapping::MapBlendOperation(BlendState::BlendOperation op)
	{
		switch(op)
		{
			case BlendState::BOP_ADD:			return GL_FUNC_ADD;
			case BlendState::BOP_SUB:			return GL_FUNC_SUBTRACT;
			case BlendState::BOP_REV_SUB:		return GL_FUNC_REVERSE_SUBTRACT;
			//case BlendState::BOP_MIN:			return GL_MIN_EXT;
			//case BlendState::BOP_MAX:			return GL_MAX_EXT;
			default:
			{
				EchoAssert(false);
				return GL_FUNC_ADD;
			}
		}
	}

	GLenum GLES2Mapping::MapBlendFactor(BlendState::BlendFactor factor)
	{
		switch(factor)
		{
			case BlendState::BF_ZERO:			return GL_ZERO;
			case BlendState::BF_ONE:			return GL_ONE;
			case BlendState::BF_SRC_ALPHA:		return GL_SRC_ALPHA;
			case BlendState::BF_DST_ALPHA:		return GL_DST_ALPHA;
			case BlendState::BF_INV_SRC_ALPHA:	return GL_ONE_MINUS_SRC_ALPHA;
			case BlendState::BF_INV_DST_ALPHA:	return GL_ONE_MINUS_DST_ALPHA;
			case BlendState::BF_SRC_COLOR:		return GL_SRC_COLOR;
			case BlendState::BF_DST_COLOR:		return GL_DST_COLOR;
			case BlendState::BF_INV_SRC_COLOR:	return GL_ONE_MINUS_SRC_COLOR;
			case BlendState::BF_INV_DST_COLOR:	return GL_ONE_MINUS_DST_COLOR;
			case BlendState::BF_SRC_ALPHA_SAT:	return GL_SRC_ALPHA_SATURATE;
			default:
			{
				EchoAssert(false);
				return GL_FUNC_ADD;
			}
		}
	}

	GLint GLES2Mapping::MapAddressMode(SamplerState::AddressMode mode)
	{
		switch(mode)
		{
			case SamplerState::AM_WRAP:		return GL_REPEAT;
			case SamplerState::AM_MIRROR:	return GL_MIRRORED_REPEAT;
			case SamplerState::AM_CLAMP:	return GL_CLAMP_TO_EDGE;
			case SamplerState::AM_BORDER:	return GL_CLAMP_TO_EDGE;
			default:
			{
				EchoAssert(false);
				return GL_REPEAT;
			}
		}
	}

}
