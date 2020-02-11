#include "XColorPreview.hpp"
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

XColorPreview::XColorPreview(QWidget *parent) :
    QWidget(parent), col(Qt::red), back( Qt::darkGray, Qt::DiagCrossPattern ),
    alpha_mode(NoAlpha), colorPrevious(Qt::blue)
{
    back.setTexture(QPixmap(QString(":/images/alphaback.png")));
}

QSize XColorPreview::sizeHint() const
{
    return QSize(24,24);
}

void XColorPreview::setColor(QColor c)
{
    col = c;
    update();
    emit colorChanged(c);
}

void XColorPreview::setPreviousColor(QColor colorPre)
{
    colorPrevious = colorPre;
    update();
}

void XColorPreview::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);

    painter.setPen(QPen(Qt::black, 1));
    QSize rectSize(62,64);
    painter.setBrush(back);
    painter.drawRect(1,1,rectSize.width(),rectSize.height());

    int h = rectSize.height() / 2;
    painter.setPen(Qt::NoPen);
    painter.setBrush(col);
    painter.drawRect(1,1,rectSize.width(),h);
    painter.setBrush(colorPrevious);
    painter.drawRect(1,h + 1,rectSize.width(),rectSize.height() - h);
}


