#include "XColorDialog.h"
#include <QDebug>

XColorDialog::XColorDialog(QWidget *parent) :
    QDialog(parent)
{
    setFixedSize(552,274);
    SetupUI();

    QVector<QColor> rainbow;
    for ( int i = 0; i < 360; i+= 360/6 )
        rainbow.push_back(QColor::fromHsv(i,255,255));
    rainbow.push_back(Qt::red);
    sliderHue->setColors(rainbow);

    this->setWindowTitle("Color Plane");
    setAcceptDrops(true);
}

void XColorDialog::SetupUI()
{
    hLayoutAll = new QHBoxLayout(this);
    hLayoutAll->setContentsMargins(8,10,8,10);
    colorSquare = new XColorSquare(this);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    colorSquare->setSizePolicy(sizePolicy);
    hLayoutAll->addWidget(colorSquare);
    verticalSlider = new XGradientSlider(this);
    verticalSlider->setOrientation(Qt::Vertical);
    hLayoutAll->addWidget(verticalSlider);

    vLayoutPreview = new QVBoxLayout;
    vLayoutPreview->setSpacing(0);
    labelNew = new QLabel("   New");
    labelCurrent = new QLabel("  Current");
    colorPreview = new XColorPreview(this);
    colorPreview->setSizePolicy(sizePolicy);
    vLayoutPreview->addWidget(labelNew);
    vLayoutPreview->addWidget(colorPreview);
    vLayoutPreview->addWidget(labelCurrent);

    vLayoutButtons = new QVBoxLayout;
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");
    vLayoutButtons->addWidget(okButton);
    vLayoutButtons->addWidget(cancelButton);
    vLayoutButtons->addStretch();
    vLayoutButtons->setContentsMargins(0,5,5,0);

    hLayoutPreviewButton = new QHBoxLayout;

    hLayoutPreviewButton->addLayout(vLayoutPreview);
    hLayoutPreviewButton->addStretch();
    hLayoutPreviewButton->addLayout(vLayoutButtons);
    hLayoutPreviewButton->setContentsMargins(25,0,0,0);

    gLayoutSlider = new QGridLayout;

    labelAlpha = new QLabel("   α");
    spinAlpha = new QSpinBox(this);
    spinAlpha->setMaximum(100);
    labelAlphaSuffix = new QLabel("%");
    sliderAlpha = new XGradientSlider(this);
    sliderAlpha->setMaximum(100);
    sliderAlpha->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(labelAlpha,0,0,1,1);
    gLayoutSlider->addWidget(spinAlpha, 0,1,1,1);
    gLayoutSlider->addWidget(labelAlphaSuffix, 0,2,1,1);
    gLayoutSlider->addWidget(sliderAlpha, 0,3,1,1);

    gLayoutSlider->setVerticalSpacing(3);

    radioHue = new QRadioButton("H:");
    radioHue->setChecked(true);
    spinHue = new QSpinBox(this);
    spinHue->setMaximum(359);
    labelHueSuffix = new QLabel("°");
    sliderHue = new XGradientSlider(this);
    sliderHue->setMaximum(359);
    sliderHue->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioHue,1,0,1,1);
    gLayoutSlider->addWidget(spinHue, 1,1,1,1);
    gLayoutSlider->addWidget(labelHueSuffix, 1,2,1,1);
    gLayoutSlider->addWidget(sliderHue, 1,3,1,1);

    radioSat = new QRadioButton("S:");
    radioSat->setChecked(false);
    spinSat = new QSpinBox(this);
    spinSat->setMaximum(100);
    labelSatSuffix = new QLabel("%");
    sliderSat = new XGradientSlider(this);
    sliderSat->setMaximum(100);
    sliderSat->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioSat,2,0,1,1);
    gLayoutSlider->addWidget(spinSat, 2,1,1,1);
    gLayoutSlider->addWidget(labelSatSuffix, 2,2,1,1);
    gLayoutSlider->addWidget(sliderSat, 2,3,1,1);

    radioVal = new QRadioButton("V:");
    radioVal->setChecked(false);
    spinVal = new QSpinBox(this);
    spinVal->setMaximum(100);
    labelValSuffix = new QLabel("%");
    sliderVal = new XGradientSlider(this);
    sliderVal->setMaximum(100);
    sliderVal->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioVal,3,0,1,1);
    gLayoutSlider->addWidget(spinVal, 3,1,1,1);
    gLayoutSlider->addWidget(labelValSuffix, 3,2,1,1);
    gLayoutSlider->addWidget(sliderVal, 3,3,1,1);

    gLayoutSlider->setVerticalSpacing(3);

    radioRed = new QRadioButton("R:");
    radioRed->setChecked(false);
    spinRed = new QSpinBox(this);
    spinRed->setMaximum(255);
    sliderRed = new XGradientSlider(this);
    sliderRed->setMaximum(255);
    sliderRed->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioRed,4,0,1,1);
    gLayoutSlider->addWidget(spinRed, 4,1,1,1);
    gLayoutSlider->addWidget(sliderRed, 4,3,1,1);

    radioGreen = new QRadioButton("G:");
    radioGreen->setChecked(false);
    spinGreen = new QSpinBox(this);
    spinGreen->setMaximum(255);
    sliderGreen = new XGradientSlider(this);
    sliderGreen->setMaximum(255);
    sliderGreen->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioGreen,5,0,1,1);
    gLayoutSlider->addWidget(spinGreen, 5,1,1,1);
    gLayoutSlider->addWidget(sliderGreen, 5,3,1,1);

    radioBlue = new QRadioButton("B:");
    radioBlue->setChecked(false);
    spinBlue = new QSpinBox(this);
    spinBlue->setMaximum(255);
    sliderBlue = new XGradientSlider(this);
    sliderBlue->setMaximum(255);
    sliderBlue->setOrientation(Qt::Horizontal);
    gLayoutSlider->addWidget(radioBlue,6,0,1,1);
    gLayoutSlider->addWidget(spinBlue, 6,1,1,1);
    gLayoutSlider->addWidget(sliderBlue, 6,3,1,1);
    gLayoutSlider->setContentsMargins(5,0,0,0);

    vLayoutPreviewSlider = new QVBoxLayout;
    vLayoutPreviewSlider->addLayout(hLayoutPreviewButton);
    vLayoutPreviewSlider->addLayout(gLayoutSlider);
    hLayoutAll->addLayout(vLayoutPreviewSlider);

    this->setLayout(hLayoutAll);

    SetConnect();
    SetVerticalSlider();
}

void XColorDialog::SetConnect()
{
    QObject::connect(radioHue, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(radioSat, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(radioVal, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(radioRed, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(radioGreen, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(radioBlue, SIGNAL(clicked(bool)),this, SLOT(SetVerticalSlider()));
    QObject::connect(sliderAlpha, SIGNAL(valueChanged(int)), spinAlpha, SLOT(setValue(int)));
    QObject::connect(spinAlpha, SIGNAL(valueChanged(int)), sliderAlpha, SLOT(setValue(int)));
    QObject::connect(sliderHue, SIGNAL(valueChanged(int)), spinHue, SLOT(setValue(int)));
    QObject::connect(spinHue, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    QObject::connect(sliderSat, SIGNAL(valueChanged(int)), spinSat, SLOT(setValue(int)));
    QObject::connect(spinSat, SIGNAL(valueChanged(int)), sliderSat, SLOT(setValue(int)));
    QObject::connect(sliderVal, SIGNAL(valueChanged(int)), spinVal, SLOT(setValue(int)));
    QObject::connect(spinVal, SIGNAL(valueChanged(int)), sliderVal, SLOT(setValue(int)));
    QObject::connect(sliderRed, SIGNAL(valueChanged(int)), spinRed, SLOT(setValue(int)));
    QObject::connect(spinRed, SIGNAL(valueChanged(int)), sliderRed, SLOT(setValue(int)));
    QObject::connect(sliderGreen, SIGNAL(valueChanged(int)), spinGreen, SLOT(setValue(int)));
    QObject::connect(spinGreen, SIGNAL(valueChanged(int)), sliderGreen, SLOT(setValue(int)));
    QObject::connect(sliderBlue, SIGNAL(valueChanged(int)), spinBlue, SLOT(setValue(int)));
    QObject::connect(spinBlue, SIGNAL(valueChanged(int)), sliderBlue, SLOT(setValue(int)));
    QObject::connect(sliderHue, SIGNAL(valueChanged(int)), this, SLOT(SetHSV()));
    QObject::connect(sliderSat, SIGNAL(valueChanged(int)), this, SLOT(SetHSV()));
    QObject::connect(sliderVal, SIGNAL(valueChanged(int)), this, SLOT(SetHSV()));
    QObject::connect(sliderRed, SIGNAL(valueChanged(int)), this, SLOT(SetRGB()));
    QObject::connect(sliderGreen, SIGNAL(valueChanged(int)), this, SLOT(SetRGB()));
    QObject::connect(sliderBlue, SIGNAL(valueChanged(int)), this, SLOT(SetRGB()));
    QObject::connect(sliderAlpha, SIGNAL(valueChanged(int)), this, SLOT(UpdateWidgets()));
    QObject::connect(colorSquare, SIGNAL(colorSelected(QColor)), this, SLOT(UpdateWidgets()));
    QObject::connect(this, SIGNAL(checkedChanged(char)), colorSquare, SLOT(setCheckedColor(char)));
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(ClickedOkButton()));

}

QColor XColorDialog::color() const
{
    QColor color = colorSquare->color();
    color.setAlpha(qRound(sliderAlpha->value() * 2.55));
    return color;
}

QColor XColorDialog::getColor(const QColor& initColor, QWidget* parent)
{
    XColorDialog dialog(parent);
    dialog.SetColor(initColor);
    if (dialog.exec() == QDialog::Accepted)
    {
        return dialog.color();
    }

    return initColor;
}

void XColorDialog::SetColor(QColor color)
{
    colorPreview->setPreviousColor(color);
    colorSquare->setColor(color);
    sliderAlpha->setValue(color.alpha() / 2.55);
    UpdateWidgets();
}

void XColorDialog::SetVerticalSlider()
{
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderSat, SLOT(setValue(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderVal, SLOT(setValue(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderRed, SLOT(setValue(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderGreen, SLOT(setValue(int)));
    disconnect(verticalSlider, SIGNAL(valueChanged(int)), sliderBlue, SLOT(setValue(int)));

    if(radioHue->isChecked())
    {
        QVector<QColor> rainbow;
        for ( int i = 0; i < 360; i+= 360/6 )
            rainbow.push_front(QColor::fromHsv(i,255,255));
        rainbow.push_front(Qt::red);
        verticalSlider->setMaximum(sliderHue->maximum());
        verticalSlider->setColors(rainbow);
        verticalSlider->setValue(sliderHue->value());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
        emit checkedChanged('H');
    }
    if(radioSat->isChecked())
    {
        verticalSlider->setMaximum(sliderSat->maximum());
        verticalSlider->setValue(sliderSat->value());
        verticalSlider->setFirstColor(sliderSat->lastColor());
        verticalSlider->setLastColor(sliderSat->firstColor());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderSat, SLOT(setValue(int)));
        emit checkedChanged('S');
    }
    if(radioVal->isChecked())
    {
        verticalSlider->setMaximum(sliderVal->maximum());
        verticalSlider->setValue(sliderVal->value());
        verticalSlider->setFirstColor(sliderVal->lastColor());
        verticalSlider->setLastColor(sliderVal->firstColor());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderVal, SLOT(setValue(int)));
        emit checkedChanged('V');
    }
    if(radioRed->isChecked())
    {
        verticalSlider->setMaximum(sliderRed->maximum());
        verticalSlider->setValue(sliderRed->value());
        verticalSlider->setFirstColor(sliderRed->lastColor());
        verticalSlider->setLastColor(sliderRed->firstColor());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderRed, SLOT(setValue(int)));
        emit checkedChanged('R');
    }
    if(radioGreen->isChecked())
    {
        verticalSlider->setMaximum(sliderGreen->maximum());
        verticalSlider->setValue(sliderGreen->value());
        verticalSlider->setFirstColor(sliderGreen->lastColor());
        verticalSlider->setLastColor(sliderGreen->firstColor());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderGreen, SLOT(setValue(int)));
        emit checkedChanged('G');
    }
    if(radioBlue->isChecked())
    {
        verticalSlider->setMaximum(sliderBlue->maximum());
        verticalSlider->setValue(sliderBlue->value());
        verticalSlider->setFirstColor(sliderBlue->lastColor());
        verticalSlider->setLastColor(sliderBlue->firstColor());
        connect(verticalSlider, SIGNAL(valueChanged(int)), sliderBlue, SLOT(setValue(int)));
        emit checkedChanged('B');
    }

}

void XColorDialog::SetHSV()
{
    if(!signalsBlocked())
    {
        colorSquare->setColor(QColor::fromHsv(sliderHue->value(),
                                              qRound(sliderSat->value()*2.55),
                                              qRound(sliderVal->value()*2.55)));
        UpdateWidgets();
    }
}

void XColorDialog::SetRGB()
{
    if(!signalsBlocked())
    {
        QColor color(sliderRed->value(), sliderGreen->value(), sliderBlue->value());
        if(color.saturation() == 0)
        {
            color = QColor::fromHsv(sliderHue->value(), 0, color.value());
        }
        colorSquare->setColor(color);
        UpdateWidgets();
    }
}

void XColorDialog::UpdateWidgets()
{
    blockSignals(true);
    foreach(QWidget* w, findChildren<QWidget*>())
        w->blockSignals(true);

    QColor col = color();

    sliderRed->setValue(col.red());
    spinRed->setValue(sliderRed->value());
    sliderRed->setFirstColor(QColor(0,col.green(),col.blue()));
    sliderRed->setLastColor(QColor(255,col.green(),col.blue()));

    sliderGreen->setValue(col.green());
    spinGreen->setValue(sliderGreen->value());
    sliderGreen->setFirstColor(QColor(col.red(),0,col.blue()));
    sliderGreen->setLastColor(QColor(col.red(),255,col.blue()));

    sliderBlue->setValue(col.blue());
    spinBlue->setValue(sliderBlue->value());
    sliderBlue->setFirstColor(QColor(col.red(),col.green(),0));
    sliderBlue->setLastColor(QColor(col.red(),col.green(),255));

    sliderHue->setValue(qRound(colorSquare->hue()*360.0));
    spinHue->setValue(sliderHue->value());

    sliderSat->setValue(qRound(colorSquare->saturation()*100.0));
    spinSat->setValue(sliderSat->value());
    sliderSat->setFirstColor(QColor::fromHsvF(colorSquare->hue(),0,colorSquare->value()));
    sliderSat->setLastColor(QColor::fromHsvF(colorSquare->hue(),1,colorSquare->value()));

    sliderVal->setValue(qRound(colorSquare->value()*100.0));
    spinVal->setValue(sliderVal->value());
    sliderVal->setFirstColor(QColor::fromHsvF(colorSquare->hue(),colorSquare->saturation(),0));
    sliderVal->setLastColor(QColor::fromHsvF(colorSquare->hue(),colorSquare->saturation(),1));

    colorPreview->setColor(col);

    SetVerticalSlider();

    blockSignals(false);
    foreach(QWidget* w, findChildren<QWidget*>())
        w->blockSignals(false);

    emit colorChanged(col);
}

void XColorDialog::ClickedOkButton()
{
    QColor color = this->color();
    emit colorSelected(color);

    accept();
}
