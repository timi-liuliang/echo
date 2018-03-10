#pragma once

#include <QDockWidget>
#include "ui_ScenePanel.h"
#include "QProperty.hpp"
#include "NewNodeDialog.h"

namespace Studio
{
	class ScenePanel : public QDockWidget, public Ui_ScenePanel
	{
		Q_OBJECT

	public:
		ScenePanel( QWidget* parent=0);
		~ScenePanel();

		// 设置显示配置
		void setDisplayConfig(const char* config);

		// 设置属性值
		void setPropertyValue(const char* propertyName, QVariant& value);

	private slots:
		void showNewNodeDialog();

	private:
		QT_UI::QProperty*	m_property;				// 属性
		NewNodeDialog*		m_newNodeDialog;		// 新建结点
	};
}