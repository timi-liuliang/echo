#pragma once

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QToolButton>
#include <string>

using namespace std;

namespace QT_UI
{
	//------------------------------------
	// 多选编辑 2013-4-12 帝林
	//------------------------------------
	class QCheckBoxList : public QWidget
	{
		Q_OBJECT

	public:
		QCheckBoxList( QWidget* parent=0, const char* items="Opacity");

		// 获取值
		QString GetValue() const { return m_value.c_str(); }

		// 设置值
		void SetValue( QString& value) { m_value = value.toStdString(); }

	private:
		// 初始化Items
		void InitItems();

	private:
		string				m_value;		// 值
		string				m_items;		// 选项
		vector<QCheckBox*>	m_checkBoxs;	// 控件列表

		QHBoxLayout*		m_horizonLayout;
		QLineEdit*			m_lineEdit;
		QToolButton*		m_toolButton;
	};
}