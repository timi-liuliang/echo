#pragma once

#include <engine/core/editor/object_operation.h>
#include <engine/core/base/object.h>
#include "TransformWidget.h"

namespace Studio
{
	class OperationTranslate : public Echo::ObjectOperation, TransformWidget::Listener
	{
	public:
		OperationTranslate();
		virtual ~OperationTranslate();

		// active
		virtual void active();

		// tick
		virtual void tick(const Echo::set<Echo::ui32>::type& objects);

		// on translate
		virtual void onTranslate(const Echo::Vector3& trans);

	private:
		// get transform widget
		TransformWidget* getTransformWidget();

		// get objects center
		Echo::Vector3 getObjectsCenter();

		// get render type
		bool is2d();

	private:
		Echo::i32					m_renderTypes;
		Echo::set<Echo::ui32>::type	m_selectedObjects;
	};
}