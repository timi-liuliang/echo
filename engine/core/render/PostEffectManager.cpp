#include "PostEffectManager.h"
#include "engine/core/Memory/MemManager.h"

namespace Echo
{
	__ImplementSingleton(PostEffectManager);

	PostEffectManager::PostEffectManager()
	{
		__ConstructSingleton;
		m_pDofManager = EchoNew(DofManager);
	}

	PostEffectManager::~PostEffectManager()
	{
		__DestructSingleton;
		EchoSafeDelete(m_pDofManager, DofManager);
	}
}