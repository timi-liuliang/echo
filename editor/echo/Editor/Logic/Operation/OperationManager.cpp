#include "OperationManager.h"
#include "Operations/OperationTranslate.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Studio
{
	OperationManager::OperationManager()
	{
		m_currentOperation = EchoNew(OperationTranslate);
	}

	OperationManager* OperationManager::instance()
	{
		static OperationManager* inst = EchoNew(OperationManager);
		return inst;
	}

	void OperationManager::tick()
	{
		if (m_currentOperation)
		{
			m_currentOperation->tick(m_selectedObjects);
		}
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