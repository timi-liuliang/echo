#pragma once

#include <engine/core/editor/object_operation.h>
#include <engine/core/base/object.h>
#include "TransformWidget.h"

namespace Studio
{
	class OperationScale : public Echo::ObjectOperation, TransformWidget::Listener
	{
	public:
		OperationScale();
		virtual ~OperationScale();

		// active
		virtual void active();

		// tick
		virtual void tick(const Echo::set<Echo::ui32>::type& objects);

		// on translate
		virtual void onScale(const Echo::Vector3& rotate) override;

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