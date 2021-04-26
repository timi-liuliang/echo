#include "OperationScale.h"
#include "Studio.h"
#include "RenderWindow.h"
#include "TransformWidget.h"

namespace Studio
{
	OperationScale::OperationScale()
	{

	}

	OperationScale::~OperationScale()
	{

	}

	void OperationScale::active()
	{
		TransformWidget* transformWidget = getTransformWidget();
		if (transformWidget)
		{
			transformWidget->setPosition(getObjectsCenter());
			transformWidget->SetEditType(TransformWidget::EditType::Scale);
			transformWidget->setRenderType2d(is2d());
			transformWidget->setListener(this);
		}
	}

	void OperationScale::tick(const Echo::set<Echo::ui32>::type& objects)
	{
		if (m_selectedObjects != objects)
		{
			m_selectedObjects = objects;

			active();
		}
	}

	void OperationScale::onScale(const Echo::Vector3& rotate)
	{
		for (Echo::i32 id : m_selectedObjects)
		{
			Echo::Node* node = dynamic_cast<Echo::Node*>(Echo::Object::getById(id));
			if (node)
			{
				node->setLocalYawPitchRoll(node->getLocalYawPitchRoll() + rotate);
			}
		}
	}

	TransformWidget* OperationScale::getTransformWidget()
	{
		return AStudio::instance()->getRenderWindow()->getTransformWidget();
	}

	Echo::Vector3 OperationScale::getObjectsCenter()
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

	bool OperationScale::is2d()
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