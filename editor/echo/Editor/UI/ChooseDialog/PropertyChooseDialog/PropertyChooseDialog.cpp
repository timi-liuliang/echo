#include "PropertyChooseDialog.h"
#include <engine/core/base/class.h>
#include <QVariant>

namespace Studio
{
	PropertyChooseDialog::PropertyChooseDialog(QWidget* parent, Echo::Object* objectPtr)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// header labels
		//QStringList headerLabels; headerLabels << "Property" << "Type";
		//m_treeWidget->setHeaderLabels(headerLabels);

		// refresh propetys display
		m_treeWidget->clear();
		refreshPropertysDisplayRecursive( m_treeWidget, objectPtr, objectPtr->getClassName());
		m_treeWidget->expandAll();
		currentItemChanged();

		QObject::connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged()));
	}

	PropertyChooseDialog::~PropertyChooseDialog()
	{

	}

	Echo::String PropertyChooseDialog::getSelectingProperty(QWidget* parent, Echo::Object* objectPtr)
	{
		PropertyChooseDialog dialog(parent, objectPtr);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			return dialog.getSelectingProperty();
		}
		else
		{
			return Echo::StringUtil::BLANK;
		}
	}

	const Echo::String PropertyChooseDialog::getSelectingProperty() const
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item && item->parent()!=m_treeWidget->invisibleRootItem())
		{
			return item->text(0).toStdString().c_str();
		}
		else
		{
			return Echo::StringUtil::BLANK;
		}
	}

	void PropertyChooseDialog::refreshPropertysDisplayRecursive(QTreeWidget* treeWidget, Echo::Object* objectPtr, const Echo::String& className)
	{
		// show parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if (parentClassName != "Object")
				refreshPropertysDisplayRecursive( treeWidget, objectPtr, parentClassName);
		}

		// show self property
		Echo::PropertyInfos propertys;
		Echo::Class::getPropertys(className, objectPtr, propertys);
		if (propertys.size())
		{
			QTreeWidgetItem* classNameItem = new QTreeWidgetItem();
			classNameItem->setText(0, className.c_str());

			// set bold true
			QFont font = classNameItem->font(0);
			font.setBold(true);
			classNameItem->setFont(0, font);
			classNameItem->setData(0, Qt::UserRole, "class");

			// append class name item
			treeWidget->invisibleRootItem()->addChild(classNameItem);

			// all propertys
			for (const Echo::PropertyInfo* prop : propertys)
			{
				QTreeWidgetItem* propertyItem = new QTreeWidgetItem();
				propertyItem->setText(0, prop->m_name.c_str());

				QFont font = propertyItem->font(0);
				font.setBold(false);
				propertyItem->setFont(0, font);
				propertyItem->setData(0, Qt::UserRole, "property");

				classNameItem->addChild(propertyItem);
			}
		}
	}

	void PropertyChooseDialog::currentItemChanged()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item && item->data(0, Qt::UserRole).toString()=="property")
		{
			m_confirm->setEnabled(true);
		}
		else
		{
			m_confirm->setEnabled(false);
		}
	}
}
