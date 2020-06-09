#include "OperationTranslate.h"
#include "Studio.h"
#include "RenderWindow.h"
#include "TransformWidget.h"

namespace Studio
{
	OperationTranslate::OperationTranslate()
	{

	}

	OperationTranslate::~OperationTranslate()
	{

	}

	void OperationTranslate::tick(const Echo::set<Echo::ui32>::type& objects)
	{
		if (m_selectedObjects != objects)
		{
			m_selectedObjects = objects;

			TransformWidget* transformWidget = getTransformWidget();
			if (transformWidget)
			{
				transformWidget->setPosition(getObjectsCenter());
			}
		}
	}

	TransformWidget* OperationTranslate::getTransformWidget()
	{
		return AStudio::instance()->getRenderWindow()->getTransformWidget();
	}

	Echo::Vector3 OperationTranslate::getObjectsCenter()
	{
		float count = 0.f;
		Echo::Vector3 position;
		for (Echo::i32 id : m_selectedObjects)
		{
			Echo::Node* node = dynamic_cast<Echo::Node*>(Echo::Object::getById(id));
			if (node)
			{
				count = count + 1.f;
				position += node->getWorldPosition();
			}
		}

		return count ? position / count : position;
	}
}