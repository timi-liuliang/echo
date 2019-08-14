#pragma once

#include <engine/core/util/Buffer.h>
#include <engine/core/math/Math.h>
#include <engine/core/math/Matrix4.h>
#include "engine/core/thread/Threading.h"

namespace Echo
{
	class DeviceFeature
	{
	public:
		DeviceFeature();
		~DeviceFeature();

		const static String cs_pvr_format;
		const static String cs_etc1_format;
		const static String cs_atitc_format;
		const static String cs_atitc_format2;
		const static String cs_dxt1_format;
		const static String cs_s3tc_format;
		const static String cs_s3tc_format2;
		const static String cs_half_float_texture;
		const static String cs_half_float_texture_linear;
		const static String cs_depth_24;
		const static String cs_depth_32;
		const static String cs_program_binary;
		const static String cs_color_buffer_half_float;
		void initialize();

		void checkOESExtensionSupport(const String& features);

		bool supportPVR() const;
		bool supportDXT1() const;
		bool supportATITC() const;
		bool supportETC1() const;
		bool supportETC2() const;
		bool supportHFTexture() const;
		bool supportHFTextureLinear() const;
		bool supportDepth24() const;
		bool supportDepth32() const;
		bool supportBinaryProgram() const;
		bool supportGLES30() const;
		bool supportHFColorBf() const;
		bool supportHFColorBf1() const;

		String& rendererName() { return m_rendererName; }

		const String& rendererName() const { return m_rendererName; }

		String& glesVersion() { return m_glesVersion; }

		const String& glesVersion() const { return m_glesVersion; }

		String& vendor() { return m_vendor; }

		const String& vendor() const { return m_vendor; }

		String& shadingLangVersion() { return m_shadingLanVersion; }

		const String& shadingLangVersion() const { return m_shadingLanVersion; }

		static void SetSupportGLES3(bool value);
		static void SetSupportETC2(bool value);

	protected:
		typedef std::map<String, bool> FeaturesMap;
		FeaturesMap m_features;
		String		m_rendererName;
		String		m_glesVersion;
		String		m_vendor;
		String		m_shadingLanVersion;
		bool		m_supportDXT1;
		bool		m_supportPVR;
		bool		m_supportATITC;
		bool		m_supportETC1;
		bool		m_supportBinaryProgram;

		bool		m_supportHalfFloatTexture;
		bool		m_supportHalfFloatTextureLinear;

		bool		m_supportDepth24;				// is device support depth 24;
		bool		m_supportDepth32;				// is device support depth 32;

		bool		m_supportHalfFloatColorBuffer;

		static bool s_supportGLES3;
		static bool s_supportETC2;
	};
}
