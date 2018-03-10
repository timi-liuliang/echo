#include "RenderStage.h"

namespace Echo
{
	// 构造函数
	RenderStage::RenderStage(RenderStageID stageID)
		: m_rsID(stageID)
		, m_bEnable(false)
		, m_bInitialized(false)
	{
	}

	// 析构函数
	RenderStage::~RenderStage()
	{
	}
}