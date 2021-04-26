#include "OperationManager.h"
#include "Operations/OperationTranslate.h"
#include "Operations/OperationRotate.h"
#include "Operations/OperationScale.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Studio
{
	OperationManager::OperationManager()
	{
		m_operations[Translate] = EchoNew(OperationTranslate);
		m_operations[Rotate] = EchoNew(OperationRotate);
		m_operations[Scale] = EchoNew(OperationScale);
	}

    OperationManager::~OperationManager()
    {
		for (Echo::ObjectOperation* operation : m_operations)
		{
			EchoSafeDelete(operation, ObjectOperation);
		}
    }

	OperationManager* OperationManager::instance()
	{
		static OperationManager* inst = EchoNew(OperationManager);
		return inst;
	}

	void OperationManager::setOperationType(OperationType type)
	{
		m_operationType = type;
		m_operations[m_operationType]->active();
	}

	void OperationManager::tick()
	{
		m_operations[m_operationType]->tick(m_selectedObjects);
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
