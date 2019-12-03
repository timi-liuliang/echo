#include <QPainter>
#include "QStringEditor.h"
#include "QPropertyModel.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/script/lua/lua_binder.h>

namespace QT_UI
{
	QStringEditor::QStringEditor(QPropertyModel* model, QString propertyName, QWidget* parent)
		: QWidget(parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_lineEdit = new QLineEdit(this);
		m_lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
		m_horizonLayout->addWidget(m_lineEdit);

		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
		QObject::connect(m_lineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onEditing()));
	}

	// set value
	void QStringEditor::setValue(const QString& val)
	{
		Echo::String result = val.toStdString().c_str();
		m_lineEdit->setText(result.c_str());
	}

	// redefine paintEvent
	void QStringEditor::paintEvent(QPaintEvent* event)
	{
		// get label background color
		QColor background = m_lineEdit->palette().color(QPalette::Window);

		// add background color for this widget.(Transparent is inappropriate here)
		QPainter painter(this);
		painter.setBrush(background);
		painter.drawRect(rect());
 		painter.setPen(background);
 		painter.drawRect(rect());

		QWidget::paintEvent(event);
	}

	// edit finished
	void QStringEditor::onEditFinished()
	{
		using namespace Echo;

		String xText = m_lineEdit->text().toStdString().c_str();
        m_propertyModel->setValue(m_propertyName, xText.c_str());
	}

	// editing
	void QStringEditor::onEditing()
	{
	}
}
