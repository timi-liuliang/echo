#include <QPainter>
#include "QStringEditor.h"
#include "QPropertyModel.h"
#include "TextEditorDialog.h"
#include <engine/core/math/Vector3.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/script/lua/lua_binder.h>
#include <engine/core/base/class.h>
#include <engine/core/base/object.h>
#include <engine/core/base/property_info.h>

namespace QT_UI
{
	QStringEditor::QStringEditor(QPropertyModel* model, QString propertyName, const Echo::String& extraData, QWidget* parent)
		: QWidget(parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
		, m_extraData(extraData)
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_lineEdit = new QLineEdit(this);
		m_lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
		m_horizonLayout->addWidget(m_lineEdit);

		m_toolButton = new QToolButton(this);
		m_toolButton->setObjectName(QString::fromUtf8("toolButton"));
		m_toolButton->setText("...");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_toolButton);

		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
		QObject::connect(m_lineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onEditing()));
		QObject::connect(m_toolButton, SIGNAL(clicked()), this, SLOT(onEditText()));

		parseExtraData();
	}

	void QStringEditor::parseExtraData()
	{
		Echo::StringArray extraDataArray = Echo::StringUtil::Split(m_extraData, ":");
		if (extraDataArray.size() == 2)
		{
			Echo::ui32   objectId = Echo::StringUtil::ParseUI32(extraDataArray[0]);
			Echo::String propertyName = extraDataArray[1];
			Echo::PropertyInfo* info = Echo::Class::getProperty(Echo::Object::getById(objectId), propertyName);
			if (info)
			{
				Echo::String readOnly = info->getHint(Echo::PropertyHintType::ReadOnly);
				Echo::String language = info->getHint(Echo::PropertyHintType::Language);

				m_isBase64String = info->m_type == Echo::Variant::Type::Base64String;
				m_readOnly = Echo::StringUtil::Equal(readOnly, "true", false);
				m_language = language;

				m_lineEdit->setReadOnly(m_readOnly);
			}
		}
	}

	void QStringEditor::setValue(const QString& val)
	{
		Echo::String result = val.toStdString().c_str();
		m_lineEdit->setText(result.c_str());
	}

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

	void QStringEditor::onEditFinished()
	{
		using namespace Echo;

		String xText = m_lineEdit->text().toStdString().c_str();
        m_propertyModel->setValue(m_propertyName, xText.c_str());
	}

	void QStringEditor::onEditing()
	{
	}

	void QStringEditor::onEditText()
	{
		if (m_isBase64String)
		{
			Echo::Base64String text64 = m_lineEdit->text().toStdString().c_str();
			Echo::String text = text64.decode();
			if (Studio::TextEditorDialog::getText(this, text, m_readOnly))
			{
				text64.encode(text.c_str());
				m_lineEdit->setText(text64.getData().c_str());

				onEditFinished();
			}
		}
		else
		{
			Echo::String text = m_lineEdit->text().toStdString().c_str();
			if (Studio::TextEditorDialog::getText(this, text, m_readOnly))
			{
				m_lineEdit->setText(text.c_str());

				onEditFinished();
			}
		}
	}
}
