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

	void OperationTranslate::active()
	{
		TransformWidget* transformWidget = getTransformWidget();
		if (transformWidget)
		{
			transformWidget->setPosition(getObjectsCenter());
			transformWidget->setRenderType2d(is2d());
			transformWidget->SetEditType(TransformWidget::EditType::Translate);
			transformWidget->setListener(this);
		}
	}

	void OperationTranslate::tick(const Echo::set<Echo::ui32>::type& objects)
	{
		if (m_selectedObjects != objects)
		{
			m_selectedObjects = objects;

			active();
		}

		TransformWidget* transformWidget = getTransformWidget();
		if (transformWidget)
		{
			transformWidget->tick();
		}
	}

	void OperationTranslate::onTranslate(const Echo::Vector3& trans)
	{
		for (Echo::i32 id : m_selectedObjects)
		{
			Echo::Node* node = dynamic_cast<Echo::Node*>(Echo::Object::getById(id));
			if (node)
			{
				node->setWorldPosition(node->getWorldPosition() + trans);
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

		return count ? position / count : Echo::Vector3::INVALID;
	}

	bool OperationTranslate::is2d()
	{
		for (Echo::i32 id : m_selectedObjects)
		{
			Echo::Render* node = dynamic_cast<Echo::Render*>(Echo::Object::getById(id));
			if (node)
			{
				return node->getRenderType().getValue() == "3d" ? false : true;
			}
		}

		return (Echo::Render::getRenderTypes() & Echo::Render::Type::Type_3D) ? false : true;
	}
}