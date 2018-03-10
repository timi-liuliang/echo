#pragma once

#include "PostImageEffect.h"

namespace Echo
{
	class PostProcessRenderStage;
	class Fxaa : public PostImageEffectPass
	{
	public:
		Fxaa(PostImageEffect* parent);
		~Fxaa();

		virtual void process();

		virtual void setCustomParameter(const String& uniform, const Vector4& value);
		virtual bool getCustomParameter(const String& name, Vector4& value) const;

	protected:
		virtual void load(rapidxml::xml_node<char>* node);
	};

}  // namespace Echo
