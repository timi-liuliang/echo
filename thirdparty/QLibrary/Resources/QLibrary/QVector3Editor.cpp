#include <QPainter>
#include "QVector3Editor.h"
#include "QPropertyModel.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QVector3Editor::QVector3Editor(QPropertyModel* model, QString propertyName, QWidget* parent)
		: QWidget(parent)
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_labelX = new QLabel(this);
		m_labelX->setObjectName(QString::fromUtf8("labelX"));
		//m_labelX->setContentsMargins(0, 0, 0, 0);
		m_labelX->setText("x");
		m_horizonLayout->addWidget(m_labelX);

		m_lineEditX = new QLineEdit(this);
		m_lineEditX->setObjectName(QString::fromUtf8("lineEditX"));
		//m_lineEditX->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_lineEditX);

		m_labelY = new QLabel(this);
		m_labelY->setObjectName(QString::fromUtf8("labelY"));
		//m_labelY->setContentsMargins(0, 0, 0, 0);
		m_labelY->setText("y");
		m_horizonLayout->addWidget(m_labelY);

		m_lineEditY = new QLineEdit(this);
		m_lineEditY->setObjectName(QString::fromUtf8("lineEditY"));
		//m_lineEditY->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_lineEditY);

		m_labelZ = new QLabel(this);
		m_labelZ->setObjectName(QString::fromUtf8("labelZ"));
		//m_labelZ->setContentsMargins(0, 0, 0, 0);
		m_labelZ->setText("z");
		m_horizonLayout->addWidget(m_labelZ);

		m_lineEditZ = new QLineEdit(this);
		m_lineEditZ->setObjectName(QString::fromUtf8("lineEditZ"));
		//m_lineEditZ->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_lineEditZ);
	}

	// set value
	void QVector3Editor::setValue(const QString& val)
	{
		Echo::String vec3Str = val.toStdString().c_str();
		Echo::Vector3 result = Echo::StringUtil::ParseVec3(vec3Str);

		m_lineEditX->setText(Echo::StringUtil::ToString(result.x).c_str());
		m_lineEditY->setText(Echo::StringUtil::ToString(result.y).c_str());
		m_lineEditZ->setText(Echo::StringUtil::ToString(result.z).c_str());
	}

	// redefine paintEvent
	void QVector3Editor::paintEvent(QPaintEvent* event)
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
}