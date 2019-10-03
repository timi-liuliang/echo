#pragma once

#include <QProcess>
#include "ui_BuildWindow.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class BuildWindow : public QMainWindow, public Ui_BuildWindow
	{
        Q_OBJECT
    
    public:
		BuildWindow(QWidget* parent=0);
		~BuildWindow();

	private:
        // init target platform list
        void initPlatformList();
        
    private slots:
        // on build
        void onBuild();
        

	};
}
