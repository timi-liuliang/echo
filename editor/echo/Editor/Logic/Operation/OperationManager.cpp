#include "OperationManager.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Studio
{
	OperationManager* OperationManager::instance()
	{
		static OperationManager* inst = EchoNew(OperationManager);
		return inst;
	}

	void OperationManager::tick()
	{
		int a = 10;
	}

	void OperationManager::onSelectedObject(Echo::ui32 objectId, bool isMultiSelect)
	{ 
		if (!isMultiSelect)
			m_selectedObjects.clear();

		if(objectId)
			m_selectedObjects.insert(objectId);
	}

	void OperationManager::onUnselectedObject(Echo::ui32 objectId)
	{
		m_selectedObjects.erase(objectId);
	}
}