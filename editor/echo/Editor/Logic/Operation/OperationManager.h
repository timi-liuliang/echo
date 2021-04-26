#pragma once

#include <engine/core/base/object.h>
#include <engine/core/util/Array.hpp>
#include "engine/core/editor/object_operation.h"

namespace Studio
{
	class OperationManager
	{
	public:
		// Operation Type
		enum OperationType
		{
			Translate = 0,
			Rotate,
			Scale,
			All
		};

	public:
		OperationManager();
        ~OperationManager();

		// instance
		static OperationManager* instance();

		// set type
		void setOperationType(OperationType type);
		bool isOperationType(OperationType type) { return m_operationType == type; }

		// tick
		void tick();

		// on select object
		void onSelectedObject(Echo::ui32 objectId, bool isMultiSelect=false);
		void onUnselectedObject(Echo::ui32 objectId);

	private:
		Echo::set<Echo::ui32>::type				m_selectedObjects;
		OperationType							m_operationType = OperationType::Translate;
		Echo::array<Echo::ObjectOperation*, 3>	m_operations;
	};
}
