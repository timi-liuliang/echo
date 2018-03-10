#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "ScenePanel.h"

namespace Studio
{
	ScenePanel::ScenePanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_newNodeDialog(nullptr)
	{
		setupUi( this);

		QObject::connect(m_newNodeButton, SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
	}

	ScenePanel::~ScenePanel()
	{

	}

	// 设置显示配置
	void ScenePanel::setDisplayConfig(const char* config)
	{
		EchoSafeDelete(m_property, QProperty);

		m_property = EchoNew(QT_UI::QProperty(config, m_propertyTreeView));
		m_property->m_model->setObjectName(QStringLiteral("property_model"));
		m_propertyTreeView->setModel(m_property->m_model);
		m_propertyTreeView->setItemDelegate(m_property->m_delegate);
		m_propertyTreeView->setAlternatingRowColors(true);
		m_propertyTreeView->expandAll();
	}

	// 设置属性值
	void ScenePanel::setPropertyValue(const char* propertyName, QVariant& value)
	{
		m_property->m_model->setValue(propertyName, value);
	}

	void ScenePanel::showNewNodeDialog()
	{
		if (!m_newNodeDialog)
			m_newNodeDialog = new NewNodeDialog(this);

		m_newNodeDialog->setVisible(true);
	}
}