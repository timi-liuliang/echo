#include "XColorSquare.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

XColorSquare::XColorSquare(QWidget *parent) :
    QFrame(parent),huem(0),sat(0),val(0),colorX(0),colorY(0),
    colorChar('0'),nSquareWidth(248),mouseStatus(Nothing)
{
    this->setCursor(Qt::CrossCursor);
}

void XColorSquare::setSquareWidth(int width)
{
    nSquareWidth = width;
    update();
}

QColor XColorSquare::color() const
{
    return QColor::fromHsvF(huem,sat,val);
}

void XColorSquare::setColor(QColor c)
{
    huem = c.hueF();
    if ( huem < 0 )
        huem = 0;
    sat = c.saturationF();
    val = c.valueF();

    update();
    emit colorChanged(c);
}

void XColorSquare::setHue(qreal h)
{
    huem = qMax(0.0,qMin(1.0,h));
    update();
}

void XColorSquare::setSaturation(qreal s)
{
    sat = qMax(0.0, qMin(1.0, s));
    update();
}

void XColorSquare::setValue(qreal v)
{
    val = qMax(0.0, qMin(1.0, v));
    update();
}

void XColorSquare::setCheckedColor(char checked)
{
    colorChar = checked;
    update();
}

void XColorSquare::RenderRectangle()
{
    int sz = nSquareWidth;
    colorSquare = QImage(sz,sz, QImage::Format_RGB32);
    for(int i = 0; i < sz; ++i)
    {
        for(int j = 0; j < sz; ++j)
        {
            switch(colorChar)
            {
                case 'H':
            {

                    colorSquare.setPixel(i,j,
                                         QColor::fromHsvF(huem, double(i)/sz, double(j)/sz).rgb());
                    break;
            }
                case 'S':
                    colorSquare.setPixel(i,j,
                                         QColor::fromHsvF(double(i)/sz, sat, double(j)/sz).rgb());
                    break;
                case 'V':
                    colorSquare.setPixel(i,j,
                                         QColor::fromHsvF(double(i)/sz, double(j)/sz, val).rgb());
                    break;
                case 'R':
                      {
                        qreal r = QColor::fromHsvF(huem, sat, val).redF();
                        colorSquare.setPixel(i,j,
                                             QColor::fromRgbF(r, double(i)/sz, double(j)/sz).rgb());
                        break;
                      }
                case 'G':
                    {
                        qreal g = QColor::fromHsvF(huem, sat, val).greenF();
                        colorSquare.setPixel(i,j,
                                             QColor::fromRgbF(double(i)/sz, g, double(j)/sz).rgb());
                        break;
                    }
                case 'B':
                    {
                        qreal b = QColor::fromHsvF(huem, sat, val).blueF();
                        colorSquare.setPixel(i,j,
                                             QColor::fromRgbF(double(i)/sz, double(j)/sz, b).rgb());
                        break;
                    }
            }
        }
    }
}

void XColorSquare::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    RenderRectangle();

    painter.setPen(QPen(Qt::black,1));
    painter.drawImage(QRectF(1,1,nSquareWidth, nSquareWidth+3.5), colorSquare);

    double selectorWidth = 6;
    painter.setPen(QPen(val > 0.5 ? Qt::black : Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    double maxDist = nSquareWidth;
    painter.drawEllipse(QPointF(colorX*maxDist,
                                colorY*maxDist),
                        selectorWidth, selectorWidth);

     this->setStyleSheet("border:1px; border-style:solid;border-color:black;");
}

void XColorSquare::mousePressEvent(QMouseEvent *ev)
{
    if(ev->buttons() & Qt::LeftButton)
    {
        mouseStatus = DragSquare;
    }
}

void XColorSquare::mouseReleaseEvent(QMouseEvent *ev)
{
    mouseMoveEvent(ev);
    mouseStatus = Nothing;

}

void XColorSquare::mouseMoveEvent(QMouseEvent *ev)
{
    QLineF glob_mouse_ln = lineToPoint(ev->pos());
    QLineF center_mouse_ln ( QPointF(0,0),
                             glob_mouse_ln.p2() - glob_mouse_ln.p1() );

    colorX = center_mouse_ln.x2()/nSquareWidth +0.5;

    colorY = center_mouse_ln.y2()/nSquareWidth +0.5;

    if ( colorX > 1 )
        colorX = 1;
    else if ( colorX < 0 )
        colorX = 0;

    if ( colorY > 1 )
        colorY = 1;
    else if ( colorY < 0 )
        colorY = 0;

    switch(colorChar)
    {
    case 'H':
        {
            sat = colorX;
            val = colorY;
            break;
        }
    case 'S':
        {
            huem = colorX;
            val = colorY;
            break;
        }
    case 'V':
        {
            huem = colorX;
            sat = colorY;
            break;
        }
    case 'R':
        {
            qreal r = QColor::fromHsvF(huem,sat,val).redF();
            huem = QColor::fromRgbF(r, colorX, colorY).hueF();
            sat = QColor::fromRgbF(r, colorX, colorY).saturationF();
            val = QColor::fromRgbF(r, colorX, colorY).valueF();
            break;
        }
    case 'G':
        {
            qreal g = QColor::fromHsvF(huem,sat,val).greenF();
            huem = QColor::fromRgbF(colorX, g, colorY).hueF();
            sat = QColor::fromRgbF(colorX, g, colorY).saturationF();
            val = QColor::fromRgbF(colorX, g, colorY).valueF();
            break;
        }
    case 'B':
        {
            qreal b = QColor::fromHsvF(huem,sat,val).blueF();
            huem = QColor::fromRgbF(colorX, colorY, b).hueF();
            sat = QColor::fromRgbF(colorX, colorY, b).saturationF();
            val = QColor::fromRgbF(colorX, colorY, b).valueF();
            break;
        }
    }

    emit colorSelected(color());
    emit colorChanged(color());
    update();
}
