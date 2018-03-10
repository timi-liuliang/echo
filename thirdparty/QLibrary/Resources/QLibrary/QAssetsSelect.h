#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <functional>

namespace QT_UI
{
	//-------------------------------------------
	// AssetsSelect 文件选择控件 2013-1-22 帝林
	//-------------------------------------------
	class QAssetsSelect : public QWidget
	{
		Q_OBJECT
	public:
		using OpenFileDialogFunction = std::function < QString(QWidget*, const char*, const char*, const char*) > ;
		QAssetsSelect( QWidget* parent = 0);

		// 设置路径
		void SetPath( QString text) { m_lineEdit->setText( text);  }

		// 设置后缀过滤
		void setExts( const char* exts) { m_exts = exts; }

		// 设置文件过滤
		void setFiles(const char* files) { m_files = files; }

		// 获取路径
		QString GetPath() { return m_lineEdit->text(); }

	//	void setOpenFileDialogFunction(const OpenFileDialogFunction& func) { m_openFileFunction = func; }

	public:
		static OpenFileDialogFunction m_openFileFunction;
	private slots:
		// 选择路径
		void OnSelectPath();

	private:
		QString			m_exts;
		QString			m_files;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
	};
}