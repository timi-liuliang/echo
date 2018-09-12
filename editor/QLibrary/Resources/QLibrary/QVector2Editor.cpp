#include <QPainter>
#include "QVector2Editor.h"
#include "QPropertyModel.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/script/lua/lua_binder.h>

namespace QT_UI
{
	QVector2Editor::QVector2Editor(QPropertyModel* model, QString propertyName, QWidget* parent)
		: QWidget(parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_labelX = new QLabel(this);
		m_labelX->setObjectName(QString::fromUtf8("labelX"));
		m_labelX->setText("x");
		m_horizonLayout->addWidget(m_labelX);

		m_lineEditX = new QLineEdit(this);
		m_lineEditX->setObjectName(QString::fromUtf8("lineEditX"));
		m_horizonLayout->addWidget(m_lineEditX);

		m_labelY = new QLabel(this);
		m_labelY->setObjectName(QString::fromUtf8("labelY"));
		m_labelY->setText("y");
		m_horizonLayout->addWidget(m_labelY);

		m_lineEditY = new QLineEdit(this);
		m_lineEditY->setObjectName(QString::fromUtf8("lineEditY"));
		m_horizonLayout->addWidget(m_lineEditY);

		QObject::connect(m_lineEditX, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
		QObject::connect(m_lineEditY, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));

		QObject::connect(m_lineEditX, SIGNAL(textEdited(const QString&)), this, SLOT(onEditing()));
		QObject::connect(m_lineEditY, SIGNAL(textEdited(const QString&)), this, SLOT(onEditing()));
	}

	// set value
	void QVector2Editor::setValue(const QString& val)
	{
		Echo::String vec3Str = val.toStdString().c_str();
		Echo::Vector2 result = Echo::StringUtil::ParseVec2(vec3Str);

		m_lineEditX->setText(Echo::StringUtil::ToString(result.x).c_str());
		m_lineEditY->setText(Echo::StringUtil::ToString(result.y).c_str());
	}

	// redefine paintEvent
	void QVector2Editor::paintEvent(QPaintEvent* event)
	{
		// get label background color
		QColor background = m_labelX->palette().color(QPalette::Window);

		// add background color for this widget.(Transparent is inappropriate here)
		QPainter painter(this);
		painter.setBrush(background);
		painter.drawRect(rect());
 		painter.setPen(background);
 		painter.drawRect(rect());

		QWidget::paintEvent(event);
	}

	// edit finished
	void QVector2Editor::onEditFinished()
	{
		using namespace Echo;

		LuaBinder* inst = LuaBinder::instance();

		String xText = m_lineEditX->text().toStdString().c_str();
		String yText = m_lineEditY->text().toStdString().c_str();
		
		if (inst->execString("__editor_calc_result_x = " + xText) &&
			inst->execString("__editor_calc_result_y = " + yText))
		{
			float x = inst->getGlobalVariableFloat("__editor_calc_result_x");
			float y = inst->getGlobalVariableFloat("__editor_calc_result_y");
			inst->execString("__editor_calc_result_x = nil");
			inst->execString("__editor_calc_result_y = nil");

			Vector2 vec2(x, y);
			m_propertyModel->setValue(m_propertyName, StringUtil::ToString(vec2).c_str());
		}
	}

	// editing
	void QVector2Editor::onEditing()
	{
		if (m_lineEditX == sender())
		{

		}
		else if (m_lineEditY == sender())
		{

		}
	}
}