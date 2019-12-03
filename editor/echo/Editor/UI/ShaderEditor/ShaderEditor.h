#pragma once

#include <QDockWidget>

namespace Studio
{
	class ShaderEditor : public QDockWidget
	{
		Q_OBJECT

	public:
		ShaderEditor(QWidget* parent = 0);
		~ShaderEditor();

	private:
	};
}