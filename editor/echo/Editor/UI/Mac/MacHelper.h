#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>

extern "C" void macChangeTitleBarColor(WId winId, double red, double green, double blue);

// adjust widget app
// https://stackoverflow.com/questions/5298614/change-the-size-of-qt-dialogs-depending-on-the-platform
void adjustWidgetSizeByOS(QWidget *rootWidget);
