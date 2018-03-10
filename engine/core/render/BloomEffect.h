#pragma once

#include "PostImageEffect.h"
#include <array>

namespace Echo
{
	class BloomEffectPass0 : public PostImageEffectPass
	{
	public:
		BloomEffectPass0(PostImageEffect* parent);
		~BloomEffectPass0();
	protected:
		virtual void initializeShaderParams();
		virtual void setCustomParameter(const String& uniform, const Vector4& value);
		virtual bool getCustomParameter(const String& name, Vector4& value) const;

		virtual Material* createMaterial(const String& fileName);

	private:
		PostImageEffect* m_parent;
		std::array<float, 128> m_kernel;
		float m_bloomSize;
	};

	class BloomEffectPass1 : public PostImageEffectPass
	{
	public:
		BloomEffectPass1(PostImageEffect* parent);
		~BloomEffectPass1();
		virtual void setCustomParameter(const String& uniform, const Vector4& value);
		virtual bool getCustomParameter(const String& name, Vector4& value) const;

		virtual Material* createMaterial(const String& fileName);

	protected:
		virtual void initializeShaderParams();
	private:
		PostImageEffect* m_parent;
		std::array<float, 128> m_kernel;
		float m_bloomSize;
	};

}  // namespace Echo
