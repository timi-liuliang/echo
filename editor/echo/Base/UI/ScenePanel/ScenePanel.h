#pragma once

#include <QDockWidget>
#include "ui_ScenePanel.h"
#include "QProperty.hpp"
#include "NewNodeDialog.h"
#include <engine/core/scene/node.h>

namespace Studio
{
	class ScenePanel : public QDockWidget, public Ui_ScenePanel
	{
		Q_OBJECT

	public:
		ScenePanel( QWidget* parent=0);
		~ScenePanel();

		static ScenePanel* instance();

		// 设置显示配置
		void setDisplayConfig(const char* config);

		// 设置属性值
		void setPropertyValue(const char* propertyName, QVariant& value);

	public:
		void refreshNodeTreeDisplay();
		void addNode(Echo::Node* node);
		void addNode( Echo::Node* node, QTreeWidgetItem* parent, bool recursive);
		Echo::Node* getCurrentSelectNode();

	private slots:
		void showNewNodeDialog();

	private:
		QT_UI::QProperty*	m_property;				// 属性
		NewNodeDialog*		m_newNodeDialog;		// 新建结点
	};
}