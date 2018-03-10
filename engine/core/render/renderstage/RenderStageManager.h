#pragma once

#include "RenderStage.h"
#include "ShadowMapRenderStage.h"
#include "PostProcessRenderStage.h"
#include "LDRSceneColorRenderStage.h"

namespace Echo
{
	/**
	* äÖÈ¾½×¶Î¹ÜÀíÆ÷
	*/
	class RenderStageManager
	{
		__DeclareSingleton(RenderStageManager);

	public:
		RenderStageManager();
		~RenderStageManager();

		// ³õÊ¼»¯
		bool initialize();

		// Í¨¹ıäÖÈ¾½×¶ÎID»ñÈ¡äÖÈ¾½×¶Î
		RenderStage* getRenderStageByID(RenderStageID _rsid);

		// Ö´ĞĞäÖÈ¾½×¶Î
		void processRenderStages();

	protected:
		ShadowMapRenderStage*		m_shadowMapRenderStage;		// ÒõÓ°Í¼äÖÈ¾½×¶Î
		vector<RenderStage*>::type	m_renderStages;				// äÖÈ¾½×¶Î
	};
}