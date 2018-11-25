#include "device_features.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	const String DeviceFeature::cs_etc1_format = "GL_OES_compressed_ETC1_RGB8_texture";
	const String DeviceFeature::cs_pvr_format = "GL_IMG_texture_compression_pvrtc";
	const String DeviceFeature::cs_atitc_format = "GL_ATI_texture_compression_atitc";
	const String DeviceFeature::cs_atitc_format2 = "GL_AMD_compressed_ATC_texture";
	const String DeviceFeature::cs_dxt1_format = "GL_EXT_texture_compression_dxt1";
	const String DeviceFeature::cs_s3tc_format = "GL_EXT_texture_compression_s3tc";
	const String DeviceFeature::cs_s3tc_format2 = "GL_OES_texture_compression_S3TC";
	const String DeviceFeature::cs_half_float_texture = "GL_OES_texture_half_float";
	const String DeviceFeature::cs_half_float_texture_linear = "GL_OES_texture_half_float_linear";
	const String DeviceFeature::cs_depth_24 = "GL_OES_depth24";
	const String DeviceFeature::cs_depth_32 = "GL_OES_depth32";
	const String DeviceFeature::cs_program_binary = "GL_OES_get_program_binary";
	const String DeviceFeature::cs_color_buffer_half_float = "GL_EXT_color_buffer_half_float";

#if defined(ECHO_PLATFORM_IOS)
	bool DeviceFeature::s_supportGLES3 = false;
#else
	bool DeviceFeature::s_supportGLES3 = true;
#endif

	bool DeviceFeature::s_supportETC2 = true;

	DeviceFeature::DeviceFeature()
	{
		initialize();
	}

	DeviceFeature::~DeviceFeature()
	{
		m_features.clear();
	}

	void DeviceFeature::checkOESExtensionSupport(const String& features)
	{
		if (features.find(DeviceFeature::cs_etc1_format) != String::npos)
		{
			m_supportETC1 = true;
		}

		if (features.find(DeviceFeature::cs_program_binary) != String::npos)
		{
			m_supportBinaryProgram = true;
			EchoLogInfo("GL_OES_get_program_binary is Support");
		}

		if (features.find(DeviceFeature::cs_atitc_format) != String::npos
			|| features.find(DeviceFeature::cs_atitc_format2) != String::npos)
		{
			m_supportATITC = true;
		}

		if (features.find(DeviceFeature::cs_dxt1_format) != String::npos
			|| features.find(DeviceFeature::cs_s3tc_format) != String::npos
			|| features.find(DeviceFeature::cs_s3tc_format2) != String::npos)
		{
			m_supportDXT1 = true;
		}

		if (features.find(DeviceFeature::cs_pvr_format) != String::npos)
		{
			m_supportPVR = true;
		}

		if (features.find(DeviceFeature::cs_half_float_texture) != String::npos)
		{
			m_supportHalfFloatTexture = true;

			if (features.find(DeviceFeature::cs_half_float_texture_linear) != String::npos)
			{
				m_supportHalfFloatTextureLinear = true;
			}
		}

		if (features.find(DeviceFeature::cs_depth_24) != String::npos)
		{
			m_supportDepth24 = true;
			EchoLogDebug("GL_DEPTH_COMPONENT24_OES is support!");
		}
		else
		{
			m_supportDepth24 = false;
			EchoLogDebug("GL_DEPTH_COMPONENT24_OES is not support!");
		}

		if (features.find(DeviceFeature::cs_depth_32) != String::npos)
		{
			m_supportDepth32 = true;
			EchoLogDebug("GL_DEPTH_COMPONENT32_OES is support!");
		}
		else
		{
			m_supportDepth32 = false;
			EchoLogDebug("GL_DEPTH_COMPONENT32_OES is not support!");
		}

		if (features.find(DeviceFeature::cs_color_buffer_half_float) != String::npos)
		{
			m_supportHalfFloatColorBuffer = true;
			EchoLogDebug("GL_EXT_color_buffer_half_float is support!");
		}
		else
		{
			m_supportHalfFloatColorBuffer = false;
			EchoLogDebug("GL_EXT_color_buffer_half_float is not support!");
		}
	}

	bool DeviceFeature::supportPVR() const
	{
		return m_supportPVR;
	}

	bool DeviceFeature::supportDepth24() const
	{
		return m_supportDepth24;
	}

	bool DeviceFeature::supportDepth32() const
	{
		return m_supportDepth32;
	}

	bool DeviceFeature::supportBinaryProgram() const
	{
		return m_supportBinaryProgram;
	}

	bool DeviceFeature::supportGLES30() const
	{
		return s_supportGLES3;
	}

	bool DeviceFeature::supportATITC() const
	{
		return m_supportATITC;
	}

	bool DeviceFeature::supportDXT1() const
	{
		return m_supportDXT1;
	}

	bool DeviceFeature::supportETC1() const
	{
		return m_supportETC1;
	}

	bool DeviceFeature::supportETC2() const
	{
		return s_supportETC2;
	}

	bool DeviceFeature::supportHFTexture() const
	{
		return m_supportHalfFloatTexture;
	}

	bool DeviceFeature::supportHFTextureLinear() const
	{
		return m_supportHalfFloatTextureLinear;
	}

	void DeviceFeature::initialize()
	{
		m_supportETC1 = false;
		m_supportBinaryProgram = false;
		m_supportPVR = false;
		m_supportDXT1 = false;
		m_supportATITC = false;
		m_supportHalfFloatTexture = false;
		m_supportHalfFloatTextureLinear = false;
		m_supportDepth24 = false;
		m_supportDepth32 = false;
		m_supportHalfFloatColorBuffer = false;
	}

	bool DeviceFeature::supportHFColorBf() const
	{
		return true;
	}

	void DeviceFeature::SetSupportGLES3(bool value)
	{
		s_supportGLES3 = value;
	}

	void DeviceFeature::SetSupportETC2(bool value)
	{
		s_supportETC2 = value;
	}

	bool DeviceFeature::supportHFColorBf1() const
	{
#ifdef ECHO_PLATFORM_WINDOWS
		return true;
#elif defined(ECHO_PLATFORM_ANDROID)
		return m_supportHalfFloatColorBuffer && s_supportGLES3;
#else
		return m_supportHalfFloatColorBuffer;
#endif
	}
}
