#include "timeline_object_bar.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/main/Engine.h"

namespace Echo
{
	QTimelineObjectBar::QTimelineObjectBar(QTreeWidgetItem* parent)
		: QWidget(nullptr)
	{
		setupUi(this);
		this->setAttribute(Qt::WA_TranslucentBackground);
		m_text->setAttribute(Qt::WA_TranslucentBackground);
		m_addButton->setAttribute(Qt::WA_TranslucentBackground);
		m_addButton->setStyleSheet("background-color: transparent;");

		setToolbuttonIcon(m_addButton, "engine/modules/anim/editor/icon/add.png");

		// Set fixed width of add toolbutton
		m_addButton->setIconSize(QSize(24, 24));
	}

	void QTimelineObjectBar::setToolbuttonIcon(QToolButton* button, const String& iconPath)
	{
		if (button)
		{
			String fullPath = Engine::instance()->getRootPath() + iconPath;
			button->setIcon(QIcon(fullPath.c_str()));
		}
	}

	void QTimelineObjectBar::setText(QString text)
	{
		m_text->setText(text);
	}
}

#endif