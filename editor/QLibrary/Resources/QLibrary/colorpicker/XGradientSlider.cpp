#include "XGradientSlider.hpp"
#include <QPainter>
#include <QStyleOptionSlider>

XGradientSlider::XGradientSlider(QWidget *parent) :
    QSlider(parent), back( Qt::darkGray, Qt::DiagCrossPattern )
{
    col_list.push_back(Qt::black);
    col_list.push_back(Qt::white);

    setOrientation(Qt::Horizontal);
}

void XGradientSlider::setBackground(QBrush bg)
{
    back = bg;
    update();
}

void XGradientSlider::setColors(QVector<QColor> bg)
{
    col_list = bg;
    update();
}

void XGradientSlider::setGradient(QLinearGradient bg)
{
    col_list.clear();
    foreach(const QGradientStop& gs, bg.stops() )
    {
        col_list.push_back(gs.second);
    }
    update();
}

QLinearGradient XGradientSlider::gradient() const
{
    int ior = orientation() == Qt::Horizontal ? 1 : 0;
    QLinearGradient lg(0,0,ior,1-ior);
    lg.setCoordinateMode(QGradient::StretchToDeviceMode);
    for(int i = 0; i < col_list.size(); i++)
        lg.setColorAt(double(i)/(col_list.size()-1),col_list[i]);
    return lg;
}

void XGradientSlider::setFirstColor(QColor c)
{
//    if ( col_list.empty() )
//        col_list.push_back(c);
//    else
//        col_list.front() = c;
    col_list.clear();
    col_list.push_back(c);
    update();
}

void XGradientSlider::setLastColor(QColor c)
{

//    if ( col_list.size() < 2 )
//        col_list.push_back(c);
//    else
//        col_list.back() = c;
    col_list.push_back(c);
    update();
}

QColor XGradientSlider::firstColor() const
{
    return col_list.empty() ? QColor() : col_list.front();
}

QColor XGradientSlider::lastColor() const
{
    return col_list.empty() ? QColor() : col_list.back();
}


void XGradientSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if(this->orientation() == Qt::Horizontal)
    {
        const int offset = 5;
        QRect rect = this->rect();
        rect.setHeight(rect.height() - 5);
        QRect sliderRect = rect.adjusted(offset,0,-offset,-rect.height()/2);

        painter.setPen(QPen(Qt::black, 0.5));
        painter.setBrush(back);
        painter.drawRect(sliderRect);
        painter.setBrush(gradient());
        painter.drawRect(sliderRect);

        const double k = (double)(value() - minimum()) / (maximum()-minimum());
        int x = (int)((rect.width()-2*offset) * k) + offset;
        QPoint tickPts[] = {
            QPoint(x,sliderRect.bottom()+1),
            QPoint(x-offset,rect.bottom()-1),
            QPoint(x+offset,rect.bottom()-1),
        };
        painter.setBrush(Qt::white);
        painter.drawPolygon(tickPts,3);
        return;
    }

//    painter.setPen(QPen(Qt::black,0.5));
//    painter.setBrush(back);
//    painter.drawRect(0,0,geometry().width()-2,geometry().height()-2);
//    painter.setBrush(gradient());
//    painter.drawRect(0,0,geometry().width()-2,geometry().height()-2);



//    QStyleOptionSlider opt_slider;
//    initStyleOption(&opt_slider);
//    opt_slider.subControls = QStyle::SC_SliderHandle;
//    if (isSliderDown())
//        opt_slider.state |= QStyle::State_Sunken;
//    style()->drawComplexControl(QStyle::CC_Slider, &opt_slider, &painter, this);

    this->setFixedWidth(25);
    const int offset = 5;
    QRect rect = this->rect();
    QRect sliderRect = QRect(rect.left()+offset, rect.top()+offset,
                             rect.right()-2*offset,rect.bottom()-2*offset);
    const double k = (double)(value() - minimum()) / (maximum()-minimum());
    int y = (int)((rect.height()-2*offset) * (1-k)) + offset;
    QPoint tickTop[] = {
        QPoint(sliderRect.left(), y),
        QPoint(rect.left(), y - offset),
        QPoint(rect.left(), y + offset)
    };
    painter.setPen(QPen(Qt::black, 0.5));
    painter.drawPolygon(tickTop, 3);

    painter.setBrush(back);
    painter.drawRect(sliderRect);
    painter.setBrush(gradient());
    painter.drawRect(sliderRect);

    QPoint tickPts[] = {
        QPoint(sliderRect.right()+1, y),
        QPoint(rect.right(), y - offset),
        QPoint(rect.right(), y + offset)
    };

    painter.setBrush(Qt::white);
    painter.drawPolygon(tickPts,3);

}
