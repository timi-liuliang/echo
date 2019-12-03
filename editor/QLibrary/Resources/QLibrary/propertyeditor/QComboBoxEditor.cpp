#include <QPainter>
#include "QComboBoxEditor.h"
#include "QPropertyModel.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/script/lua/lua_binder.h>

namespace QT_UI
{
	QComboBoxEditor::QComboBoxEditor(QPropertyModel* model, QString propertyName, QWidget* parent)
		: QWidget(parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_comboBox = new QComboBox(this);
		m_comboBox->setObjectName(QString::fromUtf8("compobox"));
		m_horizonLayout->addWidget(m_comboBox);
	}

	// add item
	void QComboBoxEditor::addItem(const QString& val)
	{
		// when add item, don't emit any signal
		QObject::disconnect(m_comboBox);

		m_comboBox->addItem( val);
	}

	// set value
	void QComboBoxEditor::setValue(const QString& val)
	{
		Echo::String value = val.toStdString().c_str();
		m_comboBox->setCurrentText(value.c_str());
		if (value.empty())
		{
			onEditFinished();
		}

		// after set initial value, begin emit combox index changed signal
		QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onEditFinished()));
	}

	// edit finished
	void QComboBoxEditor::onEditFinished()
	{
		Echo::String xText = m_comboBox->currentText().toStdString().c_str();
		m_propertyModel->setValue(m_propertyName, xText.c_str());
	}
}