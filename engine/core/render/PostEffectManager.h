#ifndef _POST_EFFECT_MANAGER_H_
#define _POST_EFFECT_MANAGER_H_

#include "engine/core/Memory/MemAllocDef.h"
#include "engine/core/Util/AssertX.h"
#include "DofManager.h"
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	class PostEffectManager
	{
		__DeclareSingleton(PostEffectManager);

	public:
		PostEffectManager();
		~PostEffectManager();
	public:
		DofManager* getDofManager(){ EchoAssert(m_pDofManager);  return m_pDofManager; }
	private:
		DofManager* m_pDofManager;
	};
}
#endif