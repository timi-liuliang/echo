#pragma once

#include "QPropertyModel.h"
#include "QPropertyDelegate.h"
#include "QPropertyConfigHelper.h"
#include <QSortFilterProxyModel>

namespace QT_UI
{
	//----------------------------------
	// QProperty 2013-1-21 帝林
	// 记录Model,Delegate对
	//----------------------------------
	struct QProperty
	{
		QPropertyModel*			m_model;
		QPropertyDelegate*		m_delegate;

		// 构造函数
		QProperty(const char* config, QWidget* parent = 0, bool isGB2312 = false)
		{
			// 设置
			m_model			= EchoNew( QPropertyModel(config, isGB2312, parent));
			m_delegate		= EchoNew( QPropertyDelegate( m_model));
		}

		// 析构函数
		~QProperty()
		{
			EchoSafeDelete( m_model, QPropertyModel);
			EchoSafeDelete( m_delegate, QPropertyDelegate);
		}
	};
}