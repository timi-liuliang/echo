#pragma once

#include <QStandardItemModel>

namespace QT_UI
{
	class QResListModel : public QStandardItemModel
	{
		Q_OBJECT

	public:
		QResListModel(QObject* parent);
		virtual ~QResListModel();

		Qt::DropActions supportedDropActions() const 
		{
			return Qt::CopyAction | Qt::MoveAction;
		}

		Qt::ItemFlags flags(const QModelIndex& index) const 
		{
			return  Qt::ItemIsSelectable  | Qt::ItemIsDragEnabled |
					Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
		}

		QStringList mimeTypes() const 
		{
			QStringList types;
			types << "text/uri-list";
			return types;
		}

		bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) 
		{
			return true;
		}

	protected:
	};
}