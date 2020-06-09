#pragma once

#include <engine/core/editor/ObjectOperation.h>
#include <engine/core/base/object.h>
#include "TransformWidget.h"

namespace Studio
{
	class OperationTranslate : public Echo::ObjectOperation
	{
	public:
		OperationTranslate();
		virtual ~OperationTranslate();

		// tick
		virtual void tick(const Echo::set<Echo::ui32>::type& objects);

	private:
		// get transform widget
		TransformWidget* getTransformWidget();

		// get objects center
		Echo::Vector3 getObjectsCenter();

	private:
		Echo::set<Echo::ui32>::type	m_selectedObjects;
	};
}