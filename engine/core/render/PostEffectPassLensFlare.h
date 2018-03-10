#pragma once
#include "PostImageEffect.h"

namespace Echo
{
	const ui8 MAX_STARLINES = 4;

	struct StarLine
	{
		float sinAngle;
		float cosAngle;
		
	};

	struct StarDef
	{
		int nStarLines;
		int nLinePasses;
		float attenuation;
		float chromaticAberration;
		float sampleLength;

		array<Color, 8> arrChromaticAberrationColor;
		array<StarLine, MAX_STARLINES> starLines;

		StarDef();
	};

	class PostEffectPassLensFlare : public PostImageEffectPass
	{
	public:
		PostEffectPassLensFlare(PostImageEffect* parent);
		~PostEffectPassLensFlare();

		virtual void process();

	private:
		virtual void initializeShaderParams();
		virtual void loadCustom(rapidxml::xml_node<char>* node);

		void* getUniformValue(const String& name);
		ui32 createCustomTarget(const String& name, float scale);

		static const ui8 MAX_SAMPLES = 8;
		static const ui8 MAX_PASSES = 3;

		StarDef m_starDef;
		Material* m_materialMerge;
		Renderable* m_renderableMerge;
		ui32 m_rtStarTemp[2];
		ui32 m_rtStarLine[MAX_STARLINES];
		array<Vector2, MAX_SAMPLES> m_sampleOffsets;
		array<Vector4, MAX_SAMPLES> m_sampleWeights;

		Color m_aaColor[MAX_PASSES][8];
	};
}