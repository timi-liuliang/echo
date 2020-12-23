#ifndef XCOLORDIALOG_H
#define XCOLORDIALOG_H

#include <QDialog>
#include "XColorPreview.hpp"
#include "XColorSquare.h"
#include "XGradientSlider.hpp"
#include <engine/core/editor/qt/extension/QMenuBarEx.h>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>

class XColorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit XColorDialog(QWidget *parent = 0);

    QColor  color() const;

    // get color
    static QColor getColor(const QColor& initColor, QWidget* parent = 0);

private:
    enum CheckedColor
    {
        H,S,V,R,G,B
    }checkedColor;

    QVBoxLayout*        vLayoutAll = nullptr;
    QT_UI::QMenuBarEx*  menubar = nullptr;

    QHBoxLayout*        hLayoutAll;
    XColorSquare*       colorSquare;
    XGradientSlider*    verticalSlider;

    QVBoxLayout*        vLayoutPreview;
    QLabel*             labelNew;
    XColorPreview*      colorPreview;
    QLabel*             labelCurrent;

    QVBoxLayout*        vLayoutButtons;
    QPushButton*        okButton;
    QPushButton*        cancelButton;

    QHBoxLayout*        hLayoutPreviewButton;
    QVBoxLayout*        vLayoutPreviewSlider;

    QGridLayout*        gLayoutSlider;
    QLabel*             labelAlpha;
    QSpinBox*           spinAlpha;
    QLabel*             labelAlphaSuffix;
    XGradientSlider*    sliderAlpha;

    QRadioButton*       radioHue;
    QSpinBox*           spinHue;
    QLabel*             labelHueSuffix;
    XGradientSlider*    sliderHue;

    QRadioButton*       radioSat;
    QSpinBox*           spinSat;
    QLabel*             labelSatSuffix;
    XGradientSlider*    sliderSat;

    QRadioButton*       radioVal;
    QSpinBox*           spinVal;
    QLabel*             labelValSuffix;
    XGradientSlider*    sliderVal;

    QRadioButton*       radioRed;
    QSpinBox*           spinRed;
    XGradientSlider*    sliderRed;
    QRadioButton*       radioGreen;
    QSpinBox*           spinGreen;
    XGradientSlider*    sliderGreen;
    QRadioButton*       radioBlue;
    QSpinBox*           spinBlue;
    XGradientSlider*    sliderBlue;

private:
    void    SetupUI();
    void    SetConnect();

signals:
    void    colorChanged(QColor);
    void    checkedChanged(char);
    void    colorSelected(QColor);

public slots:
    void    UpdateWidgets();
    void    SetHSV();
    void    SetRGB();

    void    SetColor(QColor color);
    void    SetVerticalSlider();

    void    ClickedOkButton();
};

#endif // XCOLORDIALOG_H
