#pragma once

#include <engine/core/util/StringUtil.h>
#include <QDockWidget>
#include "ui_Document.h"

namespace Studio
{
	class DocumentPanel : public QDockWidget, public Ui_DocumentPanel
	{
		Q_OBJECT

	public:
		DocumentPanel(QWidget* parent = 0);
		~DocumentPanel();

	private:
		void initApiDisplay();
		void addClassNode(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private slots:
		void onSelectClass();

	private:
	};
}