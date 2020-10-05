#pragma once

#include <engine/core/base/object.h>
#include "engine/core/editor/object_operation.h"

namespace Studio
{
	class OperationManager
	{
	public:
		OperationManager();
        ~OperationManager();

		// instance
		static OperationManager* instance();

		// tick
		void tick();

		// on select object
		void onSelectedObject(Echo::ui32 objectId, bool isMultiSelect=false);
		void onUnselectedObject(Echo::ui32 objectId);

	private:
		Echo::set<Echo::ui32>::type	m_selectedObjects;
		Echo::ObjectOperation*		m_currentOperation = nullptr;
	};
}
