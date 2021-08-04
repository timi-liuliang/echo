#include <QPainter>
#include "QRealEditor.h"
#include "QPropertyModel.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/script/lua/lua_binder.h>

namespace QT_UI
{
	QRealEditor::QRealEditor(QPropertyModel* model, QString propertyName, QWidget* parent)
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
	void QRealEditor::setValue(const QString& val)
	{
		Echo::String str = val.toStdString().c_str();
		Echo::Real result = Echo::StringUtil::ParseReal(str);

		m_lineEdit->setText(Echo::StringUtil::ToString(result).c_str());
	}

	// redefine paintEvent
	void QRealEditor::paintEvent(QPaintEvent* event)
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
	void QRealEditor::onEditFinished()
	{
		using namespace Echo;

		LuaBinder* inst = LuaBinder::instance();

		String xText = m_lineEdit->text().toStdString().c_str();	
		if (inst->execString("__editor_calc_result_x = " + xText))
		{
			double x = inst->getGlobalVariableDouble("__editor_calc_result_x");
			inst->execString("__editor_calc_result_x = nil");

			m_propertyModel->setValue(m_propertyName, StringUtil::ToString(x).c_str());
		}
	}

	// editing
	void QRealEditor::onEditing()
	{

	}
}