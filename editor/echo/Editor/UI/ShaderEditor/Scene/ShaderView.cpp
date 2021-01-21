#include "ShaderView.h"
#include "ShaderEditor.h"
#include "nodeeditor/internal/node/Node.hpp"
#include <QMenu>
#include <QWidgetAction>
#include <QTreeWidget>
#include <QContextMenuEvent>
#include <QLineEdit>
#include <nodeeditor/FlowScene>
#include "engine/core/log/Log.h"

namespace DataFlowProgramming
{
	ShaderView::ShaderView(QWidget* parent)
		: FlowView(parent)
	{

	}

	ShaderView::ShaderView(QtNodes::FlowScene* scene, QWidget* parent)
		: FlowView(scene, parent)
	{

	}

	ShaderView::~ShaderView()
	{

	}

	void ShaderView::contextMenuEvent(QContextMenuEvent* event)
	{
		if (itemAt(event->pos()))
		{
			QGraphicsView::contextMenuEvent(event);
			return;
		}

		QMenu modelMenu;

		auto skipText = QStringLiteral("skip me");

		//Add filter box to the context menu
		auto* txtBox = new QLineEdit(&modelMenu);

		txtBox->setPlaceholderText(QStringLiteral("Filter"));
		txtBox->setClearButtonEnabled(true);

		auto* txtBoxAction = new QWidgetAction(&modelMenu);
		txtBoxAction->setDefaultWidget(txtBox);

		modelMenu.addAction(txtBoxAction);

		//Add result tree view to the context menu
		auto* treeView = new QTreeWidget(&modelMenu);
		treeView->header()->close();

		auto* treeViewAction = new QWidgetAction(&modelMenu);
		treeViewAction->setDefaultWidget(treeView);

		modelMenu.addAction(treeViewAction);

		QMap<QString, QTreeWidgetItem*> topLevelItems;
		for (auto const& cat : m_scene->registry().categories())
		{
			if (cat != skipText)
			{
				auto item = new QTreeWidgetItem(treeView);
				item->setText(0, cat);
				item->setData(0, Qt::UserRole, skipText);
				topLevelItems[cat] = item;
			}
		}

		for (auto const& assoc : m_scene->registry().registeredModelsCategoryAssociation())
		{
			auto parent = topLevelItems[assoc.second];
			if (parent)
			{
				auto item = new QTreeWidgetItem(parent);
				item->setText(0, assoc.first);
				item->setData(0, Qt::UserRole, assoc.first);
			}
		}

		treeView->expandAll();

		connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem* item, int)
		{
			QString modelName = item->data(0, Qt::UserRole).toString();
			if (modelName == skipText)
			{
				return;
			}

			auto type = m_scene->registry().create(modelName);
			if (type)
			{
				auto& node = m_scene->createNode(std::move(type));
				QPoint pos = event->pos();
				QPointF posView = this->mapToScene(pos);
				node.nodeGraphicsObject().setPos(posView);

				m_scene->nodePlaced(node);
			}
			else
			{
				EchoLogError("Model not found");
			}

			modelMenu.close();
		});

		//Setup filtering
		connect(txtBox, &QLineEdit::textChanged, [&](const QString& text)
		{
			for (auto& topLvlItem : topLevelItems)
			{
				if (topLvlItem)
				{
					for (int i = 0; i < topLvlItem->childCount(); ++i)
					{
						auto child = topLvlItem->child(i);
						auto modelName = child->data(0, Qt::UserRole).toString();
						const bool match = (modelName.contains(text, Qt::CaseInsensitive));
						child->setHidden(!match);
					}
				}
			}
		});

		// make sure the text box gets focus so the user doesn't have to click on it
		txtBox->setFocus();

		modelMenu.exec(event->globalPos());
	}
}
