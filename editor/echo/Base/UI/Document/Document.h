#pragma once

#include <engine/core/util/StringUtil.h>
#include <QDialog>
#include "ui_Document.h"

namespace Studio
{
	class DocumentDialog : public QDialog, public Ui_Document
	{
		Q_OBJECT

	public:
		// instance
		static DocumentDialog* instance();

	private:
		DocumentDialog(QWidget* parent = 0);
		~DocumentDialog();

		void initApiDisplay();
		void addClassNode(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private slots:
		void onSelectClass();

	private:
	};
}